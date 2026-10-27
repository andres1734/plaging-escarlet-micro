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

    // Añade aquí los parámetros que tu PluginEditor esté intentando enlazar (ej. Drive, Output, etc.)
    // Ejemplos comunes para un preamp:
    params.push_back (std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("output", "Output", -24.0f, 12.0f, 0.0f));
    
    // Si usas un ComboBox o selector de modo/tipo (como 'choice' o 'param' que mencionaba el error):
    // params.push_back (std::make_unique<juce::AudioParameterChoice>("mode", "Mode", juce::StringArray{"Clean", "Warm", "Hot"}, 0));

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
