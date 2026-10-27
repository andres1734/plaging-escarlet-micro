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

    // --- CONTROLES VISUALES (Los que daban error) ---
    juce::Slider driveSlider;
    juce::Slider iron;              // Control de Iron Core
    
    juce::ComboBox hpfBox;          // Menú desplegable HPF
    juce::Label hpfLabel;           // Etiqueta del HPF
    
    juce::Slider chest;             // Control de Chest Warmth
    juce::Slider anti;              // Control de CM25 De-Harsh / Anti-Stridence
    juce::Slider silk;              // Control de Silk Air
    
    juce::ComboBox topologyBox;     // Menú de Topología
    juce::Slider outputSlider;      // Output Trim
    juce::ToggleButton bypassButton;// Botón de Bypass

    // --- ATTACHMENTS (Para enlazar la UI con el APVTS del procesador) ---
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
