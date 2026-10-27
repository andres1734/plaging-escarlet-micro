#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PluginProcessor.h"

class ScarlettWarmthAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor&);
    ~ScarlettWarmthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScarlettWarmthAudioProcessor& processorRef;

    juce::Slider driveSlider;
    juce::Slider ironSlider;
    juce::Slider chestSlider;
    juce::Slider stridenceSlider;
    juce::Slider silkSlider;
    juce::Slider outputSlider;

    juce::ComboBox hpfBox;
    juce::ComboBox topologyBox;
    juce::ToggleButton bypassButton;

    juce::Label driveLabel;
    juce::Label ironLabel;
    juce::Label chestLabel;
    juce::Label stridenceLabel;
    juce::Label silkLabel;
    juce::Label outputLabel;
    juce::Label hpfLabel;
    juce::Label topologyLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
