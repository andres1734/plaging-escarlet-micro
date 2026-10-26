/*
  ==============================================================================
    ScarlettWarmth - Analog Vocal Preamp VST3 Plugin
    Implementation
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ScarlettWarmthAudioProcessor::ScarlettWarmthAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout()),
      oversampling (2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR) // 4x oversampling
{
}

ScarlettWarmthAudioProcessor::~ScarlettWarmthAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ScarlettWarmthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "DRIVE", "Drive / Input", juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), 4.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "WARMTH", "Chest Warmth (220Hz)", juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), 5.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "CM25_TAMER", "CM25 Anti-Stridence", juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), 6.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "IRON", "Transformer Iron", juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), 4.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "SILK_AIR", "Silk Air (12kHz)", juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), 4.5f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "HPF", "High Pass Filter", juce::StringArray { "Flat", "40 Hz", "80 Hz", "120 Hz" }, 2));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "MODEL", "Preamp Topology", juce::StringArray { "Vintage Tube 610", "British Iron 1073", "German Valve V72", "Tape Saturation" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "TRIM", "Output Trim (dB)", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.2f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "BYPASS", "Bypass", false));

    return { params.begin(), params.end() };
}

void ScarlettWarmthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampling.initProcessing (static_cast<size_t> (samplesPerBlock));
    oversampling.reset();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;
}

void ScarlettWarmthAudioProcessor::releaseResources()
{
    oversampling.reset();
}

bool ScarlettWarmthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

float ScarlettWarmthAudioProcessor::processAnalogColoration (float input, float driveVal, float ironVal, int modelIdx)
{
    const float driveNorm = juce::jmax (0.05f, driveVal / 10.0f);
    const float ironNorm  = juce::jmax (0.0f,  ironVal  / 10.0f);
    const float x = input * (1.0f + driveNorm * 2.2f);

    float y = x;

    switch (modelIdx)
    {
        case 0: // Vintage Tube 610: Asymmetric Triode 2nd Harmonic Warmth
        {
            const float bias = 0.28f * driveNorm;
            if (x >= 0.0f)
            {
                const float tx = std::tanh (x);
                y = tx + bias * (tx * tx) * 0.45f;
            }
            else
            {
                const float absX = std::abs (x);
                const float softNeg = -(absX / (1.0f + absX * 0.65f));
                y = softNeg - bias * (softNeg * softNeg) * 0.20f;
            }
            // Add subtle transformer iron
            if (ironNorm > 0.05f)
            {
                float tIron = std::tanh (x * (1.0f + ironNorm * 1.8f)) * 0.95f;
                y = (y * (1.0f - ironNorm * 0.4f)) + (tIron * (ironNorm * 0.4f));
            }
            break;
        }
        case 1: // British Iron 1073: Transformer Class-A Core Saturation (3rd harmonics)
        {
            const float scaled = x * (1.0f + (ironNorm > 0.05f ? ironNorm : driveNorm) * 1.8f);
            y = std::tanh (scaled) * 0.95f - 0.08f * ironNorm * std::pow (std::sin (scaled), 3.0f);
            break;
        }
        case 2: // German Valve V72: Velvety round highs
        {
            const float scaled = x * (1.0f + driveNorm * 1.5f);
            y = (scaled / std::sqrt (1.0f + scaled * scaled)) * 0.98f;
            break;
        }
        case 3: // Tape Saturation
        {
            const float scaled = x * (1.0f + driveNorm * 2.0f);
            const float sign = scaled >= 0.0f ? 1.0f : -1.0f;
            y = sign * (1.0f - std::exp (-std::abs (scaled))) * 0.96f;
            break;
        }
    }

    // Safety ceiling
    return juce::jlimit (-1.0f, 1.0f, y);
}

void ScarlettWarmthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const bool isBypassed = *apvts.getRawParameterValue ("BYPASS") > 0.5f;
    if (isBypassed) return;

    const float driveVal     = *apvts.getRawParameterValue ("DRIVE");
    const float warmthVal    = *apvts.getRawParameterValue ("WARMTH");
    const float cm25TamerVal = *apvts.getRawParameterValue ("CM25_TAMER");
    const float ironVal      = *apvts.getRawParameterValue ("IRON");
    const float silkAirVal   = *apvts.getRawParameterValue ("SILK_AIR");
    const int   hpfChoice    = static_cast<int> (*apvts.getRawParameterValue ("HPF"));
    const int   modelIdx     = static_cast<int> (*apvts.getRawParameterValue ("MODEL"));
    const float trimDb       = *apvts.getRawParameterValue ("TRIM");
    const float trimLinear   = juce::Decibels::decibelsToGain (trimDb);

    // 1. High Pass Filter calculation
    float hpfCutoff = 20.0f;
    if (hpfChoice == 1) hpfCutoff = 40.0f;
    else if (hpfChoice == 2) hpfCutoff = 80.0f;
    else if (hpfChoice == 3) hpfCutoff = 120.0f;

    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), hpfCutoff, 0.707f);
    *hpfLeft.coefficients = *hpfCoeffs;
    *hpfRight.coefficients = *hpfCoeffs;

    // 2. CM25 MkIII Voice Tamer Notch at 5.8kHz (-5.5dB at max)
    const float deharshGain = -((cm25TamerVal / 10.0f) * 5.5f);
    const float deharshLinGain = juce::Decibels::decibelsToGain (deharshGain);
    auto tamerCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 5800.0f, 1.35f, deharshLinGain);
    *cm25NotchLeft.coefficients = *tamerCoeffs;
    *cm25NotchRight.coefficients = *tamerCoeffs;

    // 3. Warmth Chest contour at 220Hz (+5.5dB at max)
    const float warmthGain = (warmthVal / 10.0f) * 5.5f;
    auto warmthCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 220.0f, 0.85f, juce::Decibels::decibelsToGain (warmthGain));
    *warmthLeft.coefficients = *warmthCoeffs;
    *warmthRight.coefficients = *warmthCoeffs;

    // 4. Silk Air High Shelf at 12.5kHz (+4.0dB at max)
    const float silkGain = (silkAirVal / 10.0f) * 4.0f;
    auto silkCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (getSampleRate(), 12500.0f, 0.707f, juce::Decibels::decibelsToGain (silkGain));
    *silkShelfLeft.coefficients = *silkCoeffs;
    *silkShelfRight.coefficients = *silkCoeffs;

    // Filter audio before saturation
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto& hpf = (channel == 0) ? hpfLeft : hpfRight;
        auto& tamer = (channel == 0) ? cm25NotchLeft : cm25NotchRight;
        auto& wFilter = (channel == 0) ? warmthLeft : warmthRight;
        auto& sFilter = (channel == 0) ? silkShelfLeft : silkShelfRight;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float s = channelData[sample];
            s = hpf.processSample (s);
            s = tamer.processSample (s);
            s = wFilter.processSample (s);
            s = sFilter.processSample (s);
            channelData[sample] = s;
        }
    }

    // 5. 4x Oversampled Analog Saturation
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp (block);

    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* data = oversampledBlock.getChannelPointer (channel);
        for (size_t i = 0; i < oversampledBlock.getNumSamples(); ++i)
        {
            data[i] = processAnalogColoration (data[i], driveVal, ironVal, modelIdx);
        }
    }

    oversampling.processSamplesDown (block);

    // 6. Output Trim
    buffer.applyGain (trimLinear);
}

void ScarlettWarmthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ScarlettWarmthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* ScarlettWarmthAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScarlettWarmthAudioProcessor();
}
