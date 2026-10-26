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

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ScarlettWarmthAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
