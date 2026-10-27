#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Clase personalizada para dibujar perillas analógicas reales con indicador de posición
class AnalogKnobLookAndFeel : public juce::LookAndFeel_V4
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

        // 1. Cuerpo de la perilla (Fondo circular oscuro con relieve metálico sutil)
        g.setColour (juce::Colour::fromRGB (24, 24, 24));
        g.fillEllipse (center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colour::fromRGB (55, 55, 55));
        g.drawEllipse (center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        // 2. Anillo interior decorativo
        g.setColour (juce::Colour::fromRGB (35, 35, 35));
        g.drawEllipse (center.x - (radius * 0.75f), center.y - (radius * 0.75f), radius * 1.5f, radius * 1.5f, 1.0f);

        // 3. Puntero / Línea indicadora analógica (Gira con el valor)
        juce::Path pointer;
        auto pointerLength = radius * 0.65f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle (-pointerThickness * 0.5f, -radius + 5.0f, pointerThickness, pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (center.x, center.y));

        g.setColour (juce::Colour::fromRGB (255, 140, 0)); // Naranja brillante estilo hardware
        g.fillPath (pointer);
    }
};

// Instancia global del diseño de perillas para el editor
static AnalogKnobLookAndFeel analogKnobLF;

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (780, 440);

    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 18);
        slider.setNumDecimalPlacesToDisplay (1);
        
        // Asignar el estilo de perilla analógica personalizada
        slider.setLookAndFeel (&analogKnobLF);

        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour::fromRGB (255, 160, 50));
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB (12, 12, 12));
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB (45, 45, 45));
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font(10.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (170, 170, 170));
        addAndMakeVisible (label);
    };

    setupRotarySlider (driveSlider, driveLabel, "DRIVE");
    setupRotarySlider (ironSlider, ironLabel, "IRON / CORE");
    setupRotarySlider (chestSlider, chestLabel, "CHEST (220Hz)");
    setupRotarySlider (stridenceSlider, stridenceLabel, "ANTI-STRIDENCE");
    setupRotarySlider (silkSlider, silkLabel, "SILK AIR");
    setupRotarySlider (outputSlider, outputLabel, "OUTPUT TRIM");

    // Menús y Botones
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

    bypassButton.setButtonText ("BYPASS");
    bypassButton.setColour (juce::ToggleButton::tickColourId, juce::Colour::fromRGB (255, 140, 0));
    bypassButton.setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour::fromRGB (40, 40, 40));
    bypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (bypassButton);
}

ScarlettWarmthAudioProcessorEditor::~ScarlettWarmthAudioProcessorEditor()
{
    // Limpiar LookAndFeel para evitar fugas de memoria al cerrar
    driveSlider.setLookAndFeel (nullptr);
    ironSlider.setLookAndFeel (nullptr);
    chestSlider.setLookAndFeel (nullptr);
    stridenceSlider.setLookAndFeel (nullptr);
    silkSlider.setLookAndFeel (nullptr);
    outputSlider.setLookAndFeel (nullptr);
}

void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 18));

    g.setColour (juce::Colour::fromRGB (30, 30, 30));
    g.drawRect (getLocalBounds(), 2);

    g.setColour (juce::Colour::fromRGB (25, 25, 25));
    g.fillRoundedRectangle (15, 15, 750, 42, 4);
    g.setColour (juce::Colour::fromRGB (45, 45, 45));
    g.drawRoundedRectangle (15, 15, 750, 42, 4, 1.0f);

    g.setColour (juce::Colour::fromRGB (255, 140, 0));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII  |  WARMTH PREAMP ANALOG STAGE", 30, 26, 500, 20, juce::Justification::left);

    auto drawPanel = [&](juce::Rectangle<float> bounds, const juce::String& title)
    {
        g.setColour (juce::Colour::fromRGB (22, 22, 22));
        g.fillRoundedRectangle (bounds, 6);
        g.setColour (juce::Colour::fromRGB (42, 42, 42));
        g.drawRoundedRectangle (bounds, 6, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 140, 0));
        g.fillRect (bounds.getX() + 15, bounds.getY() + 26, bounds.getWidth() - 30, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 155, 30));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, bounds.getX() + 15, bounds.getY() + 8, 250, 16, juce::Justification::left);
    };

    drawPanel ({15, 72, 360, 225}, "ETAPA DE SATURACION");
    drawPanel ({385, 72, 380, 225}, "ECUALIZACION & DE-HARSH");
    drawPanel ({15, 307, 750, 118}, "TOPOLOGIA & CONTROL DE SALIDA");
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    driveSlider.setBounds (30, 105, 85, 75);
    driveLabel.setBounds  (30, 190, 85, 20);

    ironSlider.setBounds  (130, 105, 85, 75);
    ironLabel.setBounds   (130, 190, 85, 20);

    hpfBox.setBounds      (235, 125, 115, 30);

    chestSlider.setBounds (400, 105, 85, 75);
    chestLabel.setBounds  (400, 190, 85, 20);

    stridenceSlider.setBounds (505, 105, 85, 75);
    stridenceLabel.setBounds  (505, 190, 85, 20);

    silkSlider.setBounds  (610, 105, 85, 75);
    silkLabel.setBounds   (610, 190, 85, 20);

    topologyBox.setBounds (30, 345, 180, 35);
    outputSlider.setBounds(240, 320, 85, 75);
    outputLabel.setBounds (240, 400, 85, 20);

    bypassButton.setBounds(640, 345, 100, 35);
}
