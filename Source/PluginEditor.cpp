#include "PluginProcessor.h"
#include "PluginEditor.h"

ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Tamaño de la ventana del plugin
    setSize (780, 440);

    // Configuración de perillas giratorias (Rotary Knobs) con estilo analógico y decimales limpios
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setNumDecimalPlacesToDisplay (1); // Limita los decimales a uno (ej: 0.0 en vez de 0.0000000)
        slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB (255, 140, 0)); // Naranja cálido
        slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (255, 200, 100));
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font(10.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));
        addAndMakeVisible (label);
    };

    // Inicializar controles
    setupRotarySlider (driveSlider, driveLabel, "DRIVE");
    setupRotarySlider (ironSlider, ironLabel, "IRON / CORE");
    setupRotarySlider (chestSlider, chestLabel, "CHEST (220Hz)");
    setupRotarySlider (stridenceSlider, stridenceLabel, "ANTI-STRIDENCE");
    setupRotarySlider (silkSlider, silkLabel, "SILK AIR");
    setupRotarySlider (outputSlider, outputLabel, "OUTPUT TRIM");

    // Menú de Filtro de Graves (HPF)
    hpfBox.addItem ("Flat", 1);
    hpfBox.addItem ("40 Hz", 2);
    hpfBox.addItem ("80 Hz", 3);
    hpfBox.addItem ("120 Hz", 4);
    hpfBox.setSelectedItemIndex (2);
    addAndMakeVisible (hpfBox);

    // Menú de Topología de Previo
    topologyBox.addItem ("German Valve V72", 1);
    topologyBox.addItem ("Tube 610", 2);
    topologyBox.addItem ("British 1073", 3);
    topologyBox.addItem ("Tape Saturation", 4);
    topologyBox.setSelectedItemIndex (0);
    addAndMakeVisible (topologyBox);

    // Botón Bypass
    bypassButton.setButtonText ("BYPASS");
    bypassButton.setColour (juce::ToggleButton::tickColourId, juce::Colour::fromRGB (255, 140, 0));
    bypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (bypassButton);
}

ScarlettWarmthAudioProcessorEditor::~ScarlettWarmthAudioProcessorEditor()
{
}

void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fondo general gris oscuro / negro mate estilo consola
    g.fillAll (juce::Colour::fromRGB (20, 20, 20));

    // Cabecera superior corregida (sin caracteres extraños)
    g.setColour (juce::Colour::fromRGB (35, 35, 35));
    g.fillRoundedRectangle (15, 15, 750, 45, 6);

    g.setColour (juce::Colour::fromRGB (255, 140, 0));
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII - WARMTH PREAMP", 25, 27, 450, 20, juce::Justification::left);

    // Paneles de secciones
    auto drawPanel = [&](juce::Rectangle<float> bounds, const juce::String& title)
    {
        g.setColour (juce::Colour::fromRGB (28, 28, 28));
        g.fillRoundedRectangle (bounds, 8);
        g.setColour (juce::Colour::fromRGB (50, 50, 50));
        g.drawRoundedRectangle (bounds, 8, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 140, 0));
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.drawText (title, bounds.getX() + 15, bounds.getY() + 8, 250, 15, juce::Justification::left);
    };

    drawPanel ({15, 75, 360, 225}, "ETAPA DE SATURACION");
    drawPanel ({385, 75, 380, 225}, "ECUALIZACION & DE-HARSH");
    drawPanel ({15, 310, 750, 115}, "TOPOLOGIA & CONTROL DE SALIDA");
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    // Sección de Saturación (Ajustada para que la etiqueta y caja de texto no choquen)
    driveSlider.setBounds (30, 105, 85, 75);
    driveLabel.setBounds  (30, 195, 85, 20);

    ironSlider.setBounds  (130, 105, 85, 75);
    ironLabel.setBounds   (130, 195, 85, 20);

    hpfBox.setBounds      (235, 125, 115, 30);

    // Sección de Ecualización
    chestSlider.setBounds (400, 105, 85, 75);
    chestLabel.setBounds  (400, 195, 85, 20);

    stridenceSlider.setBounds (505, 105, 85, 75);
    stridenceLabel.setBounds  (505, 195, 85, 20);

    silkSlider.setBounds  (610, 105, 85, 75);
    silkLabel.setBounds   (610, 195, 85, 20);

    // Sección Inferior (Topología y Salida ordenadas sin solapamiento)
    topologyBox.setBounds (30, 345, 180, 35);
    outputSlider.setBounds(240, 320, 85, 75);
    outputLabel.setBounds (240, 400, 85, 20);

    bypassButton.setBounds(640, 345, 100, 35);
}
