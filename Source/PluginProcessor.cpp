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
                       )
#endif
{
}

ScarlettWarmthAudioProcessor::~ScarlettWarmthAudioProcessor()
{
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
}

void ScarlettWarmthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScarlettWarmthAudioProcessor();
}
