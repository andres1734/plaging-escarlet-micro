#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

ScarlettWarmthAudioProcessor::ScarlettWarmthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

ScarlettWarmthAudioProcessor::~ScarlettWarmthAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ScarlettWarmthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "drive", "Drive / Gain", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 4.8f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "ironCore", "Iron Core", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 40.0f, " %"));

    juce::StringArray hpfChoices = { "Off", "40 Hz", "80 Hz", "120 Hz", "160 Hz", "200 Hz" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "hpf", "Filtro de Graves (HPF)", hpfChoices, 2)); // Default 80Hz

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "chest", "Chest (220Hz)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 5.5f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "antiStridence", "CM25 De-Harsh", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -3.5f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "silkAir", "Silk Air", juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 4.5f, " dB"));

    // Nota: Mantenemos topology por si tu UI la usa, aunque no esté en el DSP de Faust básico
    juce::StringArray topologyChoices = { "German Valve V72", "American Console", "British Class A", "Solid State Modern" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "topology", "Topología de Previo", topologyChoices, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "output", "Output Trim", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "bypass", "Bypass", false));

    return { params.begin(), params.end() };
}

const juce::String ScarlettWarmthAudioProcessor::getName() const { return JucePlugin_Name; }
bool ScarlettWarmthAudioProcessor::acceptsMidi() const { return false; }
bool ScarlettWarmthAudioProcessor::producesMidi() const { return false; }
bool ScarlettWarmthAudioProcessor::isMidiEffect() const { return false; }
double ScarlettWarmthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ScarlettWarmthAudioProcessor::getNumPrograms() { return 1; }
int ScarlettWarmthAudioProcessor::getCurrentProgram() { return 0; }
void ScarlettWarmthAudioProcessor::setCurrentProgram (int) {}
const juce::String ScarlettWarmthAudioProcessor::getProgramName (int) { return {}; }
void ScarlettWarmthAudioProcessor::changeProgramName (int, const juce::String&) {}

void ScarlettWarmthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (uint32) samplesPerBlock;
    spec.numChannels = (uint32) getTotalNumOutputChannels();

    for (int i = 0; i < 2; ++i)
    {
        hpfFilter[i].prepare(spec);
        cm25Notch[i].prepare(spec);
        warmthShelf[i].prepare(spec);
        silkShelf[i].prepare(spec);
    }
}

void ScarlettWarmthAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ScarlettWarmthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}
#endif

float ScarlettWarmthAudioProcessor::applyTriodeSaturation (float inSample, float driveVal, float ironVal)
{
    float drive_norm = std::max (0.05f, driveVal / 10.0f);
    float iron_norm  = std::max (0.0f, ironVal / 100.0f);
    float in_sig     = inSample * (1.0f + drive_norm * 2.2f);
    float bias       = 0.28f * drive_norm;

    // Triode positive swing
    float tanh_pos = std::tanh(in_sig);
    float pos_out  = tanh_pos + bias * std::pow(tanh_pos, 2.0f) * 0.45f;

    // Triode negative swing
    float abs_s    = std::abs(in_sig);
    float soft_neg = (-abs_s) / (1.0f + abs_s * 0.65f);
    float neg_out  = soft_neg - bias * std::pow(soft_neg, 2.0f) * 0.20f;

    float tube_out = (in_sig >= 0.0f) ? pos_out : neg_out;

    // Transformer core saturation
    float iron_out = std::tanh(in_sig * (1.0f + iron_norm * 1.8f)) * 0.95f;

    return tube_out * (1.0f - iron_norm * 0.4f) + iron_out * (iron_norm * 0.4f);
}

void ScarlettWarmthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Leer parámetros del APVTS
    bool bypass      = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    float drive      = apvts.getRawParameterValue("drive")->load();
    float iron       = apvts.getRawParameterValue("ironCore")->load();
    int hpfChoice    = (int) apvts.getRawParameterValue("hpf")->load();
    float chest      = apvts.getRawParameterValue("chest")->load();
    float deHarsh    = apvts.getRawParameterValue("antiStridence")->load();
    float silk       = apvts.getRawParameterValue("silkAir")->load();
    float outTrim    = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load());

    // Calcular frecuencias del HPF según el menú desplegable
    float hpfFreqs[] = { 10.0f, 40.0f, 80.0f, 120.0f, 160.0f, 200.0f };
    float currentHpfFreq = hpfFreqs[std::clamp(hpfChoice, 0, 5)];

    // Actualizar coeficientes de filtros para cada canal
    if (currentHpfFreq > 15.0f) {
        for (int i = 0; i < getTotalNumOutputChannels(); ++i)
            *hpfFilter[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, currentHpfFreq);
    }

    for (int i = 0; i < getTotalNumOutputChannels(); ++i)
    {
        // CM25 De-Harsh Notch (5.8 kHz)
        *cm25Notch[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 5800.0f, 4.2f, juce::Decibels::decibelsToGain(deHarsh));
        // Chest Warmth Peak (220 Hz)
        *warmthShelf[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 220.0f, 2.5f, juce::Decibels::decibelsToGain(chest));
        // Silk Air High Shelf (12.5 kHz)
        *silkShelf[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 12500.0f, 0.707f, juce::Decibels::decibelsToGain(silk));
    }

    float peak = 0.0f;

    // Procesamiento de audio muestra por muestra
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float x = channelData[sample];
            float processed = x;

            if (!bypass)
            {
                // 1. HPF
                if (currentHpfFreq > 15.0f)
                    processed = hpfFilter[channel].processSample(processed);

                // 2. CM25 De-Harsh Notch
                processed = cm25Notch[channel].processSample(processed);

                // 3. Chest Warmth
                processed = warmthShelf[channel].processSample(processed);

                // 4. Silk Air
                processed = silkShelf[channel].processSample(processed);

                // 5. Triode & Transformer Saturation
                processed = applyTriodeSaturation(processed, drive, iron);

                // 6. Output Trim
                processed *= outTrim;
            }

            channelData[sample] = processed;
            peak = juce::jmax(peak, std::abs(processed));
        }
    }

    // Actualizar medidor de salida
    outputLevel.store(juce::jmap(juce::Decibels::gainToDecibels(peak, -60.0f), -60.0f, 0.0f, 0.0f, 1.0f));
}

bool ScarlettWarmthAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ScarlettWarmthAudioProcessor::createEditor() { return new ScarlettWarmthAudioProcessorEditor (*this); }

void ScarlettWarmthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    copyXmlToBinary (*xml, destData);
}

void ScarlettWarmthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScarlettWarmthAudioProcessor();
}
