#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Estructura auxiliar para agrupar perilla, nombre y attachment
struct Knob
{
    juce::Slider slider;
    juce::Label name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

class ScarlettWarmthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor&);
    ~ScarlettWarmthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScarlettWarmthAudioProcessor& audioProcessor;
    // LookAndFeel personalizados para evitar el error C2065
    juce::LookAndFeel_V4 knobLnF;
    juce::LookAndFeel_V4 comboLnF;
    juce::LookAndFeel_V4 switchLnF;

    // --- Controles de la interfaz ---
    Knob drive;
    Knob iron;
    Knob chest;
    Knob anti;
    Knob silk;
    Knob trim;

    juce::Label hpfLabel;
    juce::ComboBox hpfBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hpfAttach;

    juce::Label topologyLabel;
    juce::ComboBox topologyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> topologyAttach;

    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    // Componente de Medidor de Salida (Output Meter)
    // Nota: Si tu medidor tiene otra clase o nombre personalizado, asegúrate de que coincida aquí
    juce::Component meter; 

    // LookAndFeel personalizados (si los usas en tu proyecto)
    // juce::LookAndFeel_V4 knobLnF;
    // juce::LookAndFeel_V4 comboLnF;
    // juce::LookAndFeel_V4 switchLnF;

    void setupKnob (Knob& k, const char* paramID, const juce::String& title, const juce::String& unit);
    void setupCombo (juce::ComboBox& box, std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attach, const char* paramID);
    void setupSmallLabel (juce::Label& l, const juce::String& text);

    using APVTS = juce::AudioProcessorValueTreeState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
