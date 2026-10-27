#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class ScarlettWarmthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor&);
    ~ScarlettWarmthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScarlettWarmthAudioProcessor& audioProcessor;

    // --- Controles de la interfaz ---
    juce::Slider driveSlider;
    juce::Slider iron;
    juce::ComboBox hpfBox;
    juce::Label hpfLabel;
    juce::Slider chest;
    juce::Slider anti;
    juce::Slider silk;
    juce::ComboBox topologyBox;
    juce::Slider outputSlider;
    juce::ToggleButton bypassButton;

    // --- Attachments para el APVTS ---
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> ironAttachment;
    std::unique_ptr<ComboBoxAttachment> hpfAttachment;
    std::unique_ptr<SliderAttachment> chestAttachment;
    std::unique_ptr<SliderAttachment> antiAttachment;
    std::unique_ptr<SliderAttachment> silkAttachment;
    std::unique_ptr<ComboBoxAttachment> topologyAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
