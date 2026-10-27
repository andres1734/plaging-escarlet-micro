#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h> // <--- ESTO ES LO QUE FALTABA O FALLABA

class ScarlettWarmthAudioProcessor : public juce::AudioProcessor
{
public:
    std::atomic<float> outputLevel { 0.0f };
    ScarlettWarmthAudioProcessor();
    ~ScarlettWarmthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // APVTS para conectar con la interfaz
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Filtros DSP por canal (Estéreo: canal 0 y canal 1)
    juce::dsp::IIR::Filter<float> hpfFilter[2];
    juce::dsp::IIR::Filter<float> cm25Notch[2];
    juce::dsp::IIR::Filter<float> warmthShelf[2];
    juce::dsp::IIR::Filter<float> silkShelf[2];

    double currentSampleRate = 44100.0;

    // Función de saturación basada en tu script de Faust
    float applyTriodeSaturation (float inSample, float driveVal, float ironVal);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessor)
};
