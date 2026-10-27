#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PluginProcessor.h"

//==============================================================================
class ScarlettWarmthAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor&);
    ~ScarlettWarmthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScarlettWarmthAudioProcessor& processorRef;

    // Perillas analógicas (Knobs)
    juce::Slider driveSlider;
    juce::Slider ironSlider;
    juce::Slider chestSlider;
    juce::Slider stridenceSlider;
    juce::Slider silkSlider;
    juce::Slider outputSlider;

    // Selectores y Botones
    juce::ComboBox hpfBox;
    juce::ComboBox topologyBox;
    juce::ToggleButton bypassButton;

    // Etiquetas de texto para cada control
    juce::Label driveLabel;
    juce::Label ironLabel;
    juce::Label chestLabel;
    juce::Label stridenceLabel;
    juce::Label silkLabel;
    juce::Label outputLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
