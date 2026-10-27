#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// LookAndFeel avanzado para perillas boutique (Con surco completo, relieve 3D y brillo)
class BoutiqueKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (4.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 1. Surco / Pista de fondo completa (visible incluso en 0.0 para que no se vea apagado)
        juce::Path backgroundTrack;
        backgroundTrack.addCentredArc (center.x, center.y, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour::fromRGB (28, 28, 28));
        g.strokePath (backgroundTrack, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 2. Arco de valor activo iluminado en naranja cálido
        if (sliderPosProportional > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (center.x, center.y, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);
            g.setColour (juce::Colour::fromRGB (255, 140, 0));
            g.strokePath (valueArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 3. Cuerpo circular con volumen 3D y degradado metálico sutil
        auto knobRadius = radius - 8.0f;
        
        juce::ColourGradient knobGrad (juce::Colour::fromRGB (35, 35, 35), center.x, center.y - knobRadius,
                                       juce::Colour::fromRGB (15, 15, 15), center.x, center.y + knobRadius, true);
        g.setGradientFill (knobGrad);
        g.fillEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Borde metálico exterior de la perilla
        g.setColour (juce::Colour::fromRGB (70, 70, 70));
        g.drawEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // 4. Indicador / Puntero físico con muesca superior clara
        juce::Path pointer;
        auto pointerLength = knobRadius * 0.7f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle (-pointerThickness * 0.5f, -knobRadius + 3.0f, pointerThickness, pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (center.x, center.y));

        g.setColour (juce::Colour::fromRGB (255, 175, 50));
        g.fillPath (pointer);
    }
};

// LookAndFeel para el botón Bypass estilo Footswitch LED iluminado
class FootswitchLEDLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colours&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
        auto isToggled = button.getToggleState();

        // Fondo del pedal
        g.setColour (juce::Colour::fromRGB (22, 22, 22));
        g.fillRoundedRectangle (bounds, 6.0f);

        // Borde metálico
        g.setColour (isToggled ? juce::Colour::fromRGB (255, 120, 0) : juce::Colour::fromRGB (60, 60, 60));
        g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

        // LED indicador de estado
        auto ledBounds = juce::Rectangle<float> (bounds.getX() + 12.0f, bounds.getCentreY() - 5.0f, 10.0f, 10.0f);
        g.setColour (isToggled ? juce::Colour::fromRGB (255, 80, 0) : juce::Colour::fromRGB (45, 45, 45));
        g.fillEllipse (ledBounds);

        g.setColour (isToggled ? juce::Colour::fromRGB (255, 200, 100) : juce::Colour::fromRGB (70, 70, 70));
        g.drawEllipse (ledBounds, 1.0f);
    }
};

static BoutiqueKnobLookAndFeel boutiqueKnobLF;
static FootswitchLEDLookAndFeel footswitchLEDLF;

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (840, 510); // Ventana ampliada para dar espacio perfecto al Output Trim

    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& suffix)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
        slider.setNumDecimalPlacesToDisplay (1);
        slider.setTextValueSuffix (suffix);
        
        slider.setLookAndFeel (&boutiqueKnobLF);

        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour::fromRGB (255, 170, 50));
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font(10.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));
        addAndMakeVisible (label);
    };

    // Inicializar controles con nombres y unidades correctas
    setupRotarySlider (driveSlider, driveLabel, "DRIVE / GAIN", " dB");
    setupRotarySlider (ironSlider, ironLabel, "IRON CORE", " %");
    setupRotarySlider (chestSlider, chestLabel, "CHEST (220Hz)", " dB");
    setupRotarySlider (stridenceSlider, stridenceLabel, "ANTI-STRIDENCE", " dB");
    setupRotarySlider (silkSlider, silkLabel, "SILK AIR", " dB");
    setupRotarySlider (outputSlider, outputLabel, "OUTPUT TRIM", " dB");

    hpfLabel.setText ("FILTRO DE GRAVES (HPF)", juce::dontSendNotification);
    hpfLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    hpfLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));
    addAndMakeVisible (hpfLabel);

    topologyLabel.setText ("TOPOLOGÍA DE PREVIO", juce::dontSendNotification);
    topologyLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    topologyLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 200));
    addAndMakeVisible (topologyLabel);

    hpfBox.addItem ("Flat", 1);
    hpfBox.addItem ("40 Hz", 2);
    hpfBox.addItem ("80 Hz", 3);
    hpfBox.addItem ("120 Hz", 4);
    hpfBox.setSelectedItemIndex (2);
    hpfBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (14, 14, 14));
    hpfBox.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (50, 50, 50));
    hpfBox.setColour (juce::ComboBox::arrowColourId, juce::Colour::fromRGB (255, 140, 0));
    addAndMakeVisible (hpfBox);

    topologyBox.addItem ("German Valve V72", 1);
    topologyBox.addItem ("Tube 610", 2);
    topologyBox.addItem ("British 1073", 3);
    topologyBox.addItem ("Tape Saturation", 4);
    topologyBox.setSelectedItemIndex (0);
    topologyBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (14, 14, 14));
    topologyBox.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (50, 50, 50));
    topologyBox.setColour (juce::ComboBox::arrowColourId, juce::Colour::fromRGB (255, 140, 0));
    addAndMakeVisible (topologyBox);

    // Botón Bypass con estilo LED Footswitch
    bypassButton.setButtonText ("    BYPASS");
    bypassButton.setLookAndFeel (&footswitchLEDLF);
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
    bypassButton.setLookAndFeel (nullptr);
}

void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fondo general profundo #101010
    g.fillAll (juce::Colour::fromRGB (16, 16, 16));

    // Marco exterior de rack
    g.setColour (juce::Colour::fromRGB (25, 25, 25));
    g.drawRect (getLocalBounds(), 2);

    // Cabecera superior elegante con acento degradado naranja
    g.setColour (juce::Colour::fromRGB (20, 20, 20));
    g.fillRoundedRectangle (15.0f, 15.0f, 810.0f, 45.0f, 6.0f);
    g.setColour (juce::Colour::fromRGB (45, 45, 45));
    g.drawRoundedRectangle (15.0f, 15.0f, 810.0f, 45.0f, 6.0f, 1.0f);

    g.setColour (juce::Colour::fromRGB (255, 140, 0));
    g.fillRect (15.0f, 15.0f, 810.0f, 3.0f);

    g.setColour (juce::Colour::fromRGB (255, 170, 50));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII  |  WARMTH PREAMP ANALOG STAGE", 30, 26, 500, 20, juce::Justification::left);

    // Función para dibujar paneles profesionales #1a1a1a con tildes correctas y profundidad
    auto drawPanel = [&](juce::Rectangle<float> bounds, const juce::String& title)
    {
        g.setColour (juce::Colour::fromRGB (22, 22, 22)); // #1a1a1a refinado
        g.fillRoundedRectangle (bounds, 8.0f);
        g.setColour (juce::Colour::fromRGB (45, 45, 45));
        g.drawRoundedRectangle (bounds, 8.0f, 1.0f);

        // Línea divisoria de sección
        g.setColour (juce::Colour::fromRGB (255, 140, 0));
        g.fillRect (bounds.getX() + 15.0f, bounds.getY() + 26.0f, bounds.getWidth() - 30.0f, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 165, 30));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, (int)bounds.getX() + 15, (int)bounds.getY() + 8, 300, 16, juce::Justification::left);
    };

    drawPanel ({15.0f, 75.0f, 390.0f, 240.0f}, "ETAPA DE SATURACIÓN");
    drawPanel ({415.0f, 75.0f, 410.0f, 240.0f}, "ECUALIZACIÓN & DE-HARSH");
    drawPanel ({15.0f, 325.0f, 810.0f, 170.0f}, "TOPOLOGÍA & CONTROL DE SALIDA");

    // Dibujar Medidor de Nivel (VU / LED Bar decorativo para llenar el espacio libre)
    g.setColour (juce::Colour::fromRGB (12, 12, 12));
    g.fillRoundedRectangle (670.0f, 350.0f, 130.0f, 105.0f, 6.0f);
    g.setColour (juce::Colour::fromRGB (40, 40, 40));
    g.drawRoundedRectangle (670.0f, 350.0f, 130.0f, 105.0f, 6.0f, 1.0f);

    g.setColour (juce::Colour::fromRGB (150, 150, 150));
    g.setFont (9.5f);
    g.drawText ("OUTPUT METER", 670, 355, 130, 15, juce::Justification::centred);

    // Barras LED simuladas del medidor
    for (int i = 0; i < 10; ++i)
    {
        auto barColor = (i < 7) ? juce::Colour::fromRGB (255, 140, 0) : juce::Colour::fromRGB (220, 50, 50);
        g.setColour (barColor);
        g.fillRect (685.0f + (i * 11.0f), 380.0f, 8.0f, 18.0f);
    }
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    // Alineación uniforme perfecta en la Etapa de Saturación
    driveSlider.setBounds (30, 120, 80, 80);
    driveLabel.setBounds  (30, 210, 80, 20);

    ironSlider.setBounds  (130, 120, 80, 80);
    ironLabel.setBounds   (130, 210, 80, 20);

    hpfLabel.setBounds    (245, 130, 130, 15);
    hpfBox.setBounds      (245, 150, 140, 30);

    // Alineación uniforme perfecta en Ecualización
    chestSlider.setBounds (440, 120, 80, 80);
    chestLabel.setBounds  (440, 210, 80, 20);

    stridenceSlider.setBounds (555, 120, 80, 80);
    stridenceLabel.setBounds  (555, 210, 80, 20);

    silkSlider.setBounds  (670, 120, 80, 80);
    silkLabel.setBounds   (670, 210, 80, 20);

    // Panel Inferior (Output Trim centrado con espacio holgado abajo, Topología y Bypass con LED)
    topologyLabel.setBounds (35, 360, 180, 15);
    topologyBox.setBounds   (35, 380, 190, 35);

    outputSlider.setBounds  (345, 340, 110, 110); // Perilla principal grande y perfectamente centrada
    outputLabel.setBounds   (345, 458, 110, 20);  // Espacio inferior holgado

    bypassButton.setBounds  (520, 380, 110, 35);  // Posicionado con espacio para el medidor LED a la derecha
}
