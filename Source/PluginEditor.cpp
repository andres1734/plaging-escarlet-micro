#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

ScarlettWarmthAudioProcessorEditor::~ScarlettWarmthAudioProcessorEditor()
{
}

//==============================================================================
void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::center, 1);
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    // This is generally where you'll use all your child components to position their sizes.
}
