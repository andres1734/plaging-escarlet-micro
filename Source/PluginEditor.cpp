#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// LookAndFeel avanzado para perillas estilo boutique (Con arco de valor iluminado y relieve)
class ProAnalogKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (6.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 1. Arco de valor iluminado (Background track & Active value arc)
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour::fromRGB (35, 35, 35));
        g.strokePath (backgroundArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (sliderPosProportional > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
            g.setColour (juce::Colour::fromRGB (255, 140, 0)); // Naranja cálido brillante
            g.strokePath (valueArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 2. Cuerpo circular de la perilla con volumen y sombra simulada
        auto knobRadius = radius - 6.0f;
        g.setColour (juce::Colour::fromRGB (18, 18, 18));
        g.fillEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Borde metálico exterior de la perilla
        g.setColour (juce::Colour::fromRGB (75, 75, 75));
        g.drawEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

        // 3. Puntero / Indicador de posición físico
        juce::Path pointer;
        auto pointerLength = knobRadius * 0.65f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle (-pointerThickness * 0.5f, -knobRadius + 4.0f, pointerThickness, pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (center.x, center.y));

        g.setColour (juce::Colour::fromRGB (255, 170, 50)); // Tono ámbar claro para el indicador
        g.fillPath (pointer);
    }
};

static ProAnalogKnobLookAndFeel proKnobLF;

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (820, 480);

    // Función auxiliar para inicializar perillas profesionales con sus unidades
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& suffix)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
        slider.setNumDecimalPlacesToDisplay (1);
        slider.setTextValueSuffix (suffix);
        
        slider.setLookAndFeel (&proKnobLF);

        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour::fromRGB (255, 160, 50));
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font(10.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 180, 180));
        addAndMakeVisible (label);
    };

    // Inicializar controles con sus respectivas unidades profesionales
    setupRotarySlider (driveSlider, driveLabel, "DRIVE / GAIN", " dB");
    setupRotarySlider (ironSlider, ironLabel, "IRON CORE", " %");
    setupRotarySlider (chestSlider, chestLabel, "CHEST (220Hz)", " dB");
    setupRotarySlider (stridenceSlider, stridenceLabel, "ANTI-STRIDENCE", " dB");
    setupRotarySlider (silkSlider, silkLabel, "SILK AIR", " dB");
    setupRotarySlider (outputSlider, outputLabel, "OUTPUT TRIM", " dB");

    // Etiquetas descriptivas para los selectores (Item 5)
    hpfLabel.setText ("FILTRO DE GRAVES (HPF)", juce::dontSendNotification);
    hpfLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    hpfLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 180, 180));
    addAndMakeVisible (hpfLabel);

    topologyLabel.setText ("TOPOLOGIA DE PREVIO", juce::dontSendNotification);
    topologyLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    topologyLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 180, 180));
    addAndMakeVisible (topologyLabel);

    // Menús desplegables estilizados
    hpfBox.addItem ("Flat", 1);
    hpfBox.addItem ("40 Hz", 2);
    hpfBox.addItem ("80 Hz", 3);
    hpfBox.addItem ("120 Hz", 4);
    hpfBox.setSelectedItemIndex (2);
    hpfBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (15, 15, 15));
    hpfBox.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (50, 50, 50));
    hpfBox.setColour (juce::ComboBox::arrowColourId, juce::Colour::fromRGB (255, 140, 0));
    addAndMakeVisible (hpfBox);

    topologyBox.addItem ("German Valve V72", 1);
    topologyBox.addItem ("Tube 610", 2);
    topologyBox.addItem ("British 1073", 3);
    topologyBox.addItem ("Tape Saturation", 4);
    topologyBox.setSelectedItemIndex (0);
    topologyBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (15, 15, 15));
    topologyBox.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (50, 50, 50));
    topologyBox.setColour (juce::ComboBox::arrowColourId, juce::Colour::fromRGB (255, 140, 0));
    addAndMakeVisible (topologyBox);

    // Botón Bypass estilo hardware LED / Footswitch
    bypassButton.setButtonText ("BYPASS");
    bypassButton.setColour (juce::ToggleButton::tickColourId, juce::Colour::fromRGB (255, 140, 0));
    bypassButton.setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour::fromRGB (40, 40, 40));
    bypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (bypassButton);
}

ScarlettWarmthAudioProcessorEditor::~ScarlettWarmthAudioProcessorEditor()
{
    driveSlider.setLookAndFeel (nullptr);
    ironSlider.setLookAndFeel (nullptr);
    chestSlider.setLookAndFeel (nullptr);
    stridenceSlider.setLookAndFeel (nullptr);
    silkSlider.setLookAndFeel (nullptr);
    outputSlider.setLookAndFeel (nullptr);
}

void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fondo general profundo #101010
    g.fillAll (juce::Colour::fromRGB (16, 16, 16));

    // Marco exterior de rack
    g.setColour (juce::Colour::fromRGB (28, 28, 28));
    g.drawRect (getLocalBounds(), 2);

    // Cabecera superior con acento degradado naranja
    g.setColour (juce::Colour::fromRGB (22, 22, 22));
    g.fillRoundedRectangle (15, 15, 790, 45, 6);
    g.setColour (juce::Colour::fromRGB (45, 45, 45));
    g.drawRoundedRectangle (15, 15, 790, 45, 6, 1.0f);

    // Línea de acento degradado superior
    g.setColour (juce::Colour::fromRGB (255, 140, 0));
    g.fillRect (15, 15, 790, 3.0f);

    g.setColour (juce::Colour::fromRGB (255, 160, 50));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII  |  WARMTH PREAMP ANALOG STAGE", 30, 26, 500, 20, juce::Justification::left);

    // Función para dibujar paneles profesionales (#1a1a1a) con tildes correctas en español
    auto drawPanel = [&](juce::Rectangle<float> bounds, const juce::String& title)
    {
        g.setColour (juce::Colour::fromRGB (24, 24, 24)); // Panel #1a1a1a aprox
        g.fillRoundedRectangle (bounds, 8);
        g.setColour (juce::Colour::fromRGB (45, 45, 45));
        g.drawRoundedRectangle (bounds, 8, 1.0f);

        // Línea divisoria de título
        g.setColour (juce::Colour::fromRGB (255, 140, 0));
        g.fillRect (bounds.getX() + 15, bounds.getY() + 26, bounds.getWidth() - 30, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 165, 30));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, bounds.getX() + 15, bounds.getY() + 8, 300, 16, juce::Justification::left);
    };

    drawPanel ({15, 75, 380, 235}, "ETAPA DE SATURACION");
    drawPanel ({405, 75, 400, 235}, "ECUALIZACION & DE-HARSH");
    drawPanel ({15, 320, 790, 145}, "TOPOLOGIA & CONTROL DE SALIDA");
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    // Etapa de Saturación (Perillas uniformes y HPF con su etiqueta encima)
    driveSlider.setBounds (30, 115, 80, 80);
    driveLabel.setBounds  (30, 205, 80, 20);

    ironSlider.setBounds  (125, 115, 80, 80);
    ironLabel.setBounds   (125, 205, 80, 20);

    hpfLabel.setBounds    (230, 125, 130, 15);
    hpfBox.setBounds      (230, 145, 140, 30);

    // Ecualización (Cuadrícula uniforme)
    chestSlider.setBounds (425, 115, 80, 80);
    chestLabel.setBounds  (425, 205, 80, 20);

    stridenceSlider.setBounds (535, 115, 80, 80);
    stridenceLabel.setBounds  (535, 205, 80, 20);

    silkSlider.setBounds  (645, 115, 80, 80);
    silkLabel.setBounds   (645, 205, 80, 20);

    // Topología y Control de Salida (Output Trim destacado con mayor tamaño y separado)
    topologyLabel.setBounds (35, 350, 180, 15);
    topologyBox.setBounds   (35, 370, 190, 35);

    outputSlider.setBounds  (335, 335, 100, 100); // Perilla principal más grande
    outputLabel.setBounds   (335, 440, 100, 20);

    bypassButton.setBounds  (670, 370, 100, 35);
}
