#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour::fromRGB (35, 35, 35));
        g.strokePath (backgroundArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (sliderPosProportional > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
            g.setColour (juce::Colour::fromRGB (255, 140, 0));
            g.strokePath (valueArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        auto knobRadius = radius - 6.0f;
        g.setColour (juce::Colour::fromRGB (18, 18, 18));
        g.fillEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        g.setColour (juce::Colour::fromRGB (75, 75, 75));
        g.drawEllipse (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

        juce::Path pointer;
        auto pointerLength = knobRadius * 0.65f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle (-pointerThickness * 0.5f, -knobRadius + 4.0f, pointerThickness, pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (center.x, center.y));

        g.setColour (juce::Colour::fromRGB (255, 170, 50));
        g.fillPath (pointer);
    }
};

static ProAnalogKnobLookAndFeel proKnobLF;

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (820, 480);

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

    setupRotarySlider (driveSlider, driveLabel, "DRIVE / GAIN", " dB");
    setupRotarySlider (ironSlider, ironLabel, "IRON CORE", " %");
    setupRotarySlider (chestSlider, chestLabel, "CHEST (220Hz)", " dB");
    setupRotarySlider (stridenceSlider, stridenceLabel, "ANTI-STRIDENCE", " dB");
    setupRotarySlider (silkSlider, silkLabel, "SILK AIR", " dB");
    setupRotarySlider (outputSlider, outputLabel, "OUTPUT TRIM", " dB");

    hpfLabel.setText ("FILTRO DE GRAVES (HPF)", juce::dontSendNotification);
    hpfLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    hpfLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 180, 180));
    addAndMakeVisible (hpfLabel);

    topologyLabel.setText ("TOPOLOGIA DE PREVIO", juce::dontSendNotification);
    topologyLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    topologyLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 180, 180));
    addAndMakeVisible (topologyLabel);

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
    driveSlider.setLookAndFeel (nullptr);
    ironSlider.setLookAndFeel (nullptr);
    chestSlider.setLookAndFeel (nullptr);
    stridenceSlider.setLookAndFeel (nullptr);
    silkSlider.setLookAndFeel (nullptr);
    outputSlider.setLookAndFeel (nullptr);
}

void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (16, 16, 16));

    g.setColour (juce::Colour::fromRGB (28, 28, 28));
    g.drawRect (getLocalBounds(), 2);

    g.setColour (juce::Colour::fromRGB (22, 22, 22));
    g.fillRoundedRectangle (15.0f, 15.0f, 790.0f, 45.0f, 6.0f);
    g.setColour (juce::Colour::fromRGB (45, 45, 45));
    g.drawRoundedRectangle (15.0f, 15.0f, 790.0f, 45.0f, 6.0f, 1.0f);

    g.setColour (juce::Colour::fromRGB (255, 140, 0));
    g.fillRect (15.0f, 15.0f, 790.0f, 3.0f);

    g.setColour (juce::Colour::fromRGB (255, 160, 50));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII  |  WARMTH PREAMP ANALOG STAGE", 30, 26, 500, 20, juce::Justification::left);

    auto drawPanel = [&](juce::Rectangle<float> bounds, const juce::String& title)
    {
        g.setColour (juce::Colour::fromRGB (24, 24, 24));
        g.fillRoundedRectangle (bounds, 8.0f);
        g.setColour (juce::Colour::fromRGB (45, 45, 45));
        g.drawRoundedRectangle (bounds, 8.0f, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 140, 0));
        g.fillRect (bounds.getX() + 15.0f, bounds.getY() + 26.0f, bounds.getWidth() - 30.0f, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 165, 30));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, (int)bounds.getX() + 15, (int)bounds.getY() + 8, 300, 16, juce::Justification::left);
    };

    drawPanel ({15.0f, 75.0f, 380.0f, 235.0f}, "ETAPA DE SATURACION");
    drawPanel ({405.0f, 75.0f, 400.0f, 235.0f}, "ECUALIZACION & DE-HARSH");
    drawPanel ({15.0f, 320.0f, 790.0f, 145.0f}, "TOPOLOGIA & CONTROL DE SALIDA");
}

void ScarlettWarmthAudioProcessorEditor::resized()
{
    driveSlider.setBounds (30, 115, 80, 80);
    driveLabel.setBounds  (30, 205, 80, 20);

    ironSlider.setBounds  (125, 115, 80, 80);
    ironLabel.setBounds   (125, 205, 80, 20);

    hpfLabel.setBounds    (230, 125, 130, 15);
    hpfBox.setBounds      (230, 145, 140, 30);

    chestSlider.setBounds (425, 115, 80, 80);
    chestLabel.setBounds  (425, 205, 80, 20);

    stridenceSlider.setBounds (535, 115, 80, 80);
    stridenceLabel.setBounds  (535, 205, 80, 20);

    silkSlider.setBounds  (645, 115, 80, 80);
    silkLabel.setBounds   (645, 205, 80, 20);

    topologyLabel.setBounds (35, 350, 180, 15);
    topologyBox.setBounds   (35, 370, 190, 35);

    outputSlider.setBounds  (335, 335, 100, 100);
    outputLabel.setBounds   (335, 440, 100, 20);

    bypassButton.setBounds  (670, 370, 100, 35);
}
