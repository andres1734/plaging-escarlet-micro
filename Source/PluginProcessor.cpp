#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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
       apvts (*this, nullptr, "Parameters", createParameterLayout()) // <-- INICIALIZACIÓN DEL APVTS
{
}

ScarlettWarmthAudioProcessor::~ScarlettWarmthAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ScarlettWarmthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 1. Etapa de Saturación - Drive / Gain (0 a 24 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "drive", "Drive / Gain", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));

    // 2. Iron Core (0 a 100 %)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "ironCore", "Iron Core", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    // 3. Filtro de Graves (HPF) - Menú desplegable
    juce::StringArray hpfChoices = { "Off", "40 Hz", "80 Hz", "120 Hz", "160 Hz", "200 Hz" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "hpf", "Filtro de Graves (HPF)", hpfChoices, 0));

    // 4. Chest (220Hz) (-12 a 12 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "chest", "Chest (220Hz)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    // 5. Anti-Stridence (-12 a 12 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "antiStridence", "Anti-Stridence", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    // 6. Silk Air (0 a 12 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "silkAir", "Silk Air", juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f));

    // 7. Topología de Previo - Menú desplegable
    juce::StringArray topologyChoices = { "German Valve V72", "American Console", "British Class A", "Solid State Modern" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "topology", "Topología de Previo", topologyChoices, 0));

    // 8. Output Trim (-24 a 12 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "output", "Output Trim", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ScarlettWarmthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ScarlettWarmthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ScarlettWarmthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ScarlettWarmthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ScarlettWarmthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ScarlettWarmthAudioProcessor::getNumPrograms()
{
    return 1;
}

int ScarlettWarmthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ScarlettWarmthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ScarlettWarmthAudioProcessor::getProgramName (int index)
{
    return {};
}

void ScarlettWarmthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ScarlettWarmthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void ScarlettWarmthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ScarlettWarmthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsSynth
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
  #endif
}
#endif

void ScarlettWarmthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
        
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
        
    outputLevel.store (juce::jmap (juce::Decibels::gainToDecibels (peak, -60.0f), -60.0f, 0.0f, 0.0f, 1.0f));
}

//==============================================================================
bool ScarlettWarmthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ScarlettWarmthAudioProcessor::createEditor()
{
    return new ScarlettWarmthAudioProcessorEditor (*this);
}

//==============================================================================
void ScarlettWarmthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Opcional pero recomendado para guardar estados del APVTS:
    // auto xml = apvts.copyState().createXml();
    // copyXmlToBinary (*xml, destData);
}

void ScarlettWarmthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Opcional:
    // std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    // if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
    //     apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScarlettWarmthAudioProcessor();
}
