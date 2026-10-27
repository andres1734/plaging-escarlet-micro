#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Saturacion triodo asimetrico + hierro de transformador (port del DSP de Faust)
// Con compensacion: ganancia 1 para senales pequenas, asi Drive agrega color y
// no solo volumen.
static inline float saturate (float sig, float driveNorm, float ironNorm, float biasScale)
{
    const float g    = 1.0f + driveNorm * 2.2f;
    const float in   = sig * g;
    const float bias = 0.28f * driveNorm * biasScale;

    // Semiciclo positivo (conduccion suave de rejilla)
    const float t   = std::tanh (in);
    const float pos = t + bias * (t * t) * 0.45f;

    // Semiciclo negativo (corte suave)
    const float a   = std::abs (in);
    const float sn  = -a / (1.0f + a * 0.65f);
    const float neg = sn - bias * (sn * sn) * 0.20f;

    const float tube = in >= 0.0f ? pos : neg;

    // Saturacion del nucleo del transformador
    const float iron = std::tanh (in * (1.0f + ironNorm * 1.8f)) * 0.95f;

    const float w = ironNorm * 0.4f;
    const float y = tube * (1.0f - w) + iron * w;

    const float smallSignalGain = g * ((1.0f - w) + w * (1.0f + ironNorm * 1.8f) * 0.95f);
    return y / smallSignalGain;
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ScarlettWarmthAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    // Los valores por defecto son los mismos que tu DSP de Faust
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 },         "Drive / Gain",
                                                        NormalisableRange<float> (0.0f, 10.0f, 0.1f), 4.8f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "iron", 1 },          "Iron Core",
                                                        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 40.0f));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "hpf", 1 },          "HPF",
                                                         StringArray { "Off", "40 Hz", "80 Hz", "120 Hz" }, 2));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "chest", 1 },         "Chest (220Hz)",
                                                        NormalisableRange<float> (-6.0f, 6.0f, 0.1f), 3.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "antiStridence", 1 }, "Anti-Stridence",
                                                        NormalisableRange<float> (0.0f, 8.0f, 0.1f), 3.6f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "silkAir", 1 },       "Silk Air",
                                                        NormalisableRange<float> (-6.0f, 6.0f, 0.1f), 1.8f));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "topology", 1 },     "Topologia",
                                                         StringArray { "German Valve V72", "Solid State Modern" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "outputTrim", 1 },    "Output Trim",
                                                        NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f));
    p.push_back (std::make_unique<AudioParameterBool>  (ParameterID { "bypass", 1 },        "Bypass", false));

    return { p.begin(), p.end() };
}

//==============================================================================
ScarlettWarmthAudioProcessor::ScarlettWarmthAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    pDrive    = apvts.getRawParameterValue ("drive");
    pIron     = apvts.getRawParameterValue ("iron");
    pHpf      = apvts.getRawParameterValue ("hpf");
    pChest    = apvts.getRawParameterValue ("chest");
    pAnti     = apvts.getRawParameterValue ("antiStridence");
    pSilk     = apvts.getRawParameterValue ("silkAir");
    pTopology = apvts.getRawParameterValue ("topology");
    pTrim     = apvts.getRawParameterValue ("outputTrim");
    pBypass   = apvts.getRawParameterValue ("bypass");
}

const juce::String ScarlettWarmthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

//==============================================================================
void ScarlettWarmthAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;

    for (int ch = 0; ch < 2; ++ch)
    {
        hpf[ch].reset(); cm25[ch].reset(); warm[ch].reset(); silk[ch].reset();
        dcX[ch] = dcY[ch] = 0.0f;
    }

    dcR = 1.0f - (float) (2.0 * juce::MathConstants<double>::pi * 5.0 / sampleRate);   // corte ~5 Hz
    lastHpf = -1;

    driveSm.reset  (sampleRate, 0.03);
    ironSm.reset   (sampleRate, 0.03);
    trimSm.reset   (sampleRate, 0.03);
    bypassSm.reset (sampleRate, 0.02);

    driveSm.setCurrentAndTargetValue  (juce::jmax (0.05f, pDrive->load() / 10.0f));
    ironSm.setCurrentAndTargetValue   (pIron->load() / 100.0f);
    trimSm.setCurrentAndTargetValue   (juce::Decibels::decibelsToGain (pTrim->load()));
    bypassSm.setCurrentAndTargetValue (pBypass->load() > 0.5f ? 1.0f : 0.0f);

    updateFilters();
}

bool ScarlettWarmthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

//==============================================================================
void ScarlettWarmthAudioProcessor::updateFilters()
{
    static const double hpfFreq[] = { 0.0, 40.0, 80.0, 120.0 };

    const int hpfIdx = juce::jlimit (0, 3, (int) std::lround (pHpf->load()));
    hpfOn = hpfIdx > 0;

    if (hpfIdx != lastHpf)
    {
        for (auto& f : hpf) f.reset();
        lastHpf = hpfIdx;
    }

    if (hpfOn)
        for (auto& f : hpf) f.makeHighpass (currentSampleRate, hpfFreq[hpfIdx], 0.70710678);

    // De-harsh del CM25: recorte a 5.8 kHz (Q = 5800 / 4200 Hz de ancho)
    for (auto& f : cm25) f.makePeak (currentSampleRate, 5800.0, 5800.0 / 4200.0, -(double) pAnti->load());

    // Chest a 220 Hz (Q = 220 / 250 Hz de ancho)
    for (auto& f : warm) f.makePeak (currentSampleRate, 220.0, 220.0 / 250.0, (double) pChest->load());

    // Silk Air: shelf alto a 12.5 kHz
    for (auto& f : silk) f.makeHighShelf (currentSampleRate, 12500.0, (double) pSilk->load());
}

//==============================================================================
void ScarlettWarmthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    const int n        = buffer.getNumSamples();

    for (int i = totalIn; i < totalOut; ++i)
        buffer.clear (i, 0, n);

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);

    if (numCh > 0 && n > 0)
    {
        const bool  valve     = pTopology->load() < 0.5f;       // German Valve V72 = triodo completo
        const float biasScale = valve ? 1.0f : 0.25f;           // Solid State: menos asimetria
        const float ironScale = valve ? 1.0f : 0.5f;            // y menos hierro

        driveSm.setTargetValue  (juce::jmax (0.05f, pDrive->load() / 10.0f));
        ironSm.setTargetValue   ((pIron->load() / 100.0f) * ironScale);
        trimSm.setTargetValue   (juce::Decibels::decibelsToGain (pTrim->load()));
        bypassSm.setTargetValue (pBypass->load() > 0.5f ? 1.0f : 0.0f);

        updateFilters();

        float* data[2] = { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

        for (int i = 0; i < n; ++i)
        {
            const float dn = driveSm.getNextValue();
            const float ir = ironSm.getNextValue();
            const float tg = trimSm.getNextValue();
            const float bp = bypassSm.getNextValue();

            for (int ch = 0; ch < numCh; ++ch)
            {
                const float dry = data[ch][i];
                float x = dry;

                if (hpfOn) x = hpf[ch].process (x);
                x = cm25[ch].process (x);
                x = warm[ch].process (x);
                x = silk[ch].process (x);

                x = saturate (x, dn, ir, biasScale);

                // Bloqueo de DC (la saturacion asimetrica genera un poco)
                const float y = x - dcX[ch] + dcR * dcY[ch];
                dcX[ch] = x;
                dcY[ch] = y;

                data[ch][i] = dry * bp + (y * tg) * (1.0f - bp);
            }
        }
    }

    // Medidor de salida
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, n));

    outputLevel.store (juce::jmap (juce::Decibels::gainToDecibels (peak, -60.0f), -60.0f, 0.0f, 0.0f, 1.0f));
}

//==============================================================================
juce::AudioProcessorEditor* ScarlettWarmthAudioProcessor::createEditor()
{
    return new ScarlettWarmthAudioProcessorEditor (*this);
}

void ScarlettWarmthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ScarlettWarmthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScarlettWarmthAudioProcessor();
}
