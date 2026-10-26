/*
  ==============================================================================
    ScarlettWarmth - Analog Vocal Preamp VST3 Plugin
    Designed for Focusrite Scarlett CM25 MkIII & Studio Vocals
    JUCE AudioProcessor Header
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ScarlettWarmthAudioProcessor : public juce::AudioProcessor
{
public:
    ScarlettWarmthAudioProcessor();
    ~ScarlettWarmthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ScarlettWarmth"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Processors
    juce::dsp::Oversampling<float> oversampling;
    juce::dsp::IIR::Filter<float> hpfLeft, hpfRight;
    juce::dsp::IIR::Filter<float> cm25NotchLeft, cm25NotchRight;
    juce::dsp::IIR::Filter<float> cm25SibilanceLeft, cm25SibilanceRight;
    juce::dsp::IIR::Filter<float> warmthLeft, warmthRight;
    juce::dsp::IIR::Filter<float> silkShelfLeft, silkShelfRight;

    // Analog Waveshaper transfer function (Triode 2nd Harmonic + Transformer Iron)
    float processAnalogColoration (float input, float driveVal, float ironVal, int modelIdx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessor)
};
