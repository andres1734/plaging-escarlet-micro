#include "PluginProcessor.h"
#include "PluginEditor.h"

ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Definimos el tamaño de la ventana del plugin (formato rack hardware)
    setSize (780, 440);

    // Configuración general para perillas (Rotary Knobs) con estilo analógico
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 20);
        slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB (255, 140, 0)); // Naranja cálido
        slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (255, 200, 100));
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font(11.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));
        addAndMakeVisible (label);
    };

    // Inicializar controles de saturación y EQ
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
    hpfBox.setSelectedItemIndex (2); // Por defecto 80Hz
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
    // Fondo general gris oscuro / negro mate (estilo equipo hardware)
    g.fillAll (juce::Colour::fromRGB (20, 20, 20));

    // Cabecera superior (Banner del nombre del plugin)
    g.setColour (juce::Colour::fromRGB (35, 35, 35));
    g.fillRoundedRectangle (15, 15, 750, 45, 6);

    g.setColour (juce::Colour::fromRGB (255, 140, 0)); // Texto naranja brillante
    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM2M MKIII • WARMTH PREAMP", 25, 27, 450, 20, juce::Justification::left);

    // Función auxiliar para dibujar paneles/cajas contenedoras con estilo
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

    // Dibujar paneles principales
    drawPanel ({15, 75, 360, 225}, "ETAPA DE SATURACION");
    drawPanel ({385, 75, 380, 225}, "ECUALIZACION & DE-HARSH");
    drawPanel ({15, 310, 750, 115}, "TOPOLOGIA & CONTROL DE SALIDA");
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    // Ubicación de controles en la sección de Saturación
    driveSlider.setBounds (30, 110, 90, 100);
    driveLabel.setBounds  (30, 215, 90, 20);

    ironSlider.setBounds  (135, 110, 90, 100);
    ironLabel.setBounds   (135, 215, 90, 20);

    hpfBox.setBounds      (245, 130, 105, 30);

    // Ubicación de controles en la sección de Ecualización
    chestSlider.setBounds (405, 110, 90, 100);
    chestLabel.setBounds  (405, 215, 90, 20);

    stridenceSlider.setBounds (510, 110, 90, 100);
    stridenceLabel.setBounds  (510, 215, 90, 20);

    silkSlider.setBounds  (615, 110, 90, 100);
    silkLabel.setBounds   (615, 215, 90, 20);

    // Ubicación en la sección inferior (Topología y Salida)
    topologyBox.setBounds (30, 345, 180, 35);
    outputSlider.setBounds(240, 325, 90, 90);
    outputLabel.setBounds (240, 395, 90, 20);

    bypassButton.setBounds(640, 345, 100, 35);
}
