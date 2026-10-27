#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// IDs de parametros (deben coincidir con createParameterLayout en PluginProcessor.cpp)
namespace ID
{
    constexpr const char* drive    = "drive";
    constexpr const char* iron     = "iron";
    constexpr const char* hpf      = "hpf";
    constexpr const char* chest    = "chest";
    constexpr const char* anti     = "antiStridence";
    constexpr const char* silk     = "silkAir";
    constexpr const char* topology = "topology";
    constexpr const char* trim     = "outputTrim";
    constexpr const char* bypass   = "bypass";
}

static const juce::Colour kOrange = juce::Colour::fromRGB (255, 140, 0);

// Texto con tildes: siempre se lee como UTF-8
static juce::String U (const char* utf8) { return juce::String::fromUTF8 (utf8); }

//==============================================================================
ScarlettWarmthAudioProcessorEditor::ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), audioProcessor (p)
{
    setupKnob (drive, ID::drive, "DRIVE / GAIN",   " dB", 1);
    setupKnob (iron,  ID::iron,  "IRON CORE",      " %",  0);
    setupKnob (chest, ID::chest, "CHEST (220Hz)",  " dB", 1);
    setupKnob (anti,  ID::anti,  "ANTI-STRIDENCE", " dB", 1);
    setupKnob (silk,  ID::silk,  "SILK AIR",       " dB", 1);
    setupKnob (trim,  ID::trim,  "OUTPUT TRIM",    " dB", 1);

    setupSmallLabel (hpfLabel,      U ("FILTRO DE GRAVES (HPF)"));
    setupSmallLabel (topologyLabel, U ("TOPOLOGÍA DE PREVIO"));

    setupCombo (hpfBox,      hpfAttach,      ID::hpf);
    setupCombo (topologyBox, topologyAttach, ID::topology);

    bypassButton.setLookAndFeel (&switchLnF);
    addAndMakeVisible (bypassButton);
    bypassAttach = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.apvts, ID::bypass, bypassButton);

    addAndMakeVisible (meter);

    setSize (840, 510);
}

ScarlettWarmthAudioProcessorEditor::~ScarlettWarmthAudioProcessorEditor()
{
    for (auto* s : { &drive.slider, &iron.slider, &chest.slider, &anti.slider, &silk.slider, &trim.slider })
        s->setLookAndFeel (nullptr);

    hpfBox.setLookAndFeel (nullptr);
    topologyBox.setLookAndFeel (nullptr);
    bypassButton.setLookAndFeel (nullptr);
}

//==============================================================================
void ScarlettWarmthAudioProcessorEditor::setupKnob (Knob& k, const char* paramID,
                                                    const juce::String& title,
                                                    const juce::String& unit, int decimals)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f, true);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
    k.slider.setLookAndFeel (&knobLnF);
    k.slider.setColour (juce::Slider::textBoxTextColourId,       juce::Colour::fromRGB (255, 150, 20));
    k.slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    k.slider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.attach = std::make_unique<APVTS::SliderAttachment> (audioProcessor.apvts, paramID, k.slider);

    // Texto limpio y fijo: decimales controlados + unidad (se pone DESPUES del attachment)
    k.slider.textFromValueFunction = [unit, decimals] (double v)
    {
        return juce::String (v, decimals) + unit;
    };
    k.slider.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue(); };
    k.slider.updateText();

    k.name.setText (title, juce::dontSendNotification);
    k.name.setFont (juce::Font (10.0f, juce::Font::bold));
    k.name.setColour (juce::Label::textColourId, juce::Colour::fromRGB (150, 150, 150));
    k.name.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (k.name);
}

void ScarlettWarmthAudioProcessorEditor::setupCombo (juce::ComboBox& box,
                                                     std::unique_ptr<APVTS::ComboBoxAttachment>& attach,
                                                     const char* paramID)
{
    // Los items salen directamente del parametro
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (audioProcessor.apvts.getParameter (paramID)))
        box.addItemList (choice->choices, 1);

    box.setLookAndFeel (&comboLnF);
    addAndMakeVisible (box);

    attach = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.apvts, paramID, box);
}

void ScarlettWarmthAudioProcessorEditor::setupSmallLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setFont (juce::Font (10.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, juce::Colour::fromRGB (150, 150, 150));
    l.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (l);
}

//==============================================================================
void ScarlettWarmthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (16, 16, 16));

    // Encabezado
    juce::Rectangle<float> header (15.0f, 15.0f, 810.0f, 45.0f);
    juce::ColourGradient hg (juce::Colour::fromRGB (30, 30, 30), header.getX(), header.getY(),
                             juce::Colour::fromRGB (18, 18, 18), header.getX(), header.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRoundedRectangle (header, 6.0f);
    g.setColour (kOrange);
    g.fillRoundedRectangle (header.getX(), header.getY(), header.getWidth(), 3.0f, 1.5f);

    g.setColour (juce::Colour::fromRGB (255, 165, 30));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("FOCUSRITE CM25 MKIII  |  WARMTH PREAMP ANALOG STAGE",
                header.withTrimmedLeft (16.0f).withTrimmedTop (3.0f), juce::Justification::centredLeft);

    // Paneles con degradado, brillo superior y linea que se desvanece
    auto drawPanel = [&] (juce::Rectangle<float> b, const juce::String& title)
    {
        juce::ColourGradient bg (juce::Colour::fromRGB (31, 31, 31), b.getX(), b.getY(),
                                 juce::Colour::fromRGB (17, 17, 17), b.getX(), b.getBottom(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (b, 8.0f);

        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.drawLine (b.getX() + 8.0f, b.getY() + 1.0f, b.getRight() - 8.0f, b.getY() + 1.0f);
        g.setColour (juce::Colour::fromRGB (48, 48, 48));
        g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

        const float ly = b.getY() + 26.0f;
        juce::ColourGradient line (kOrange, b.getX() + 15.0f, ly,
                                   kOrange.withAlpha (0.0f), b.getRight() - 15.0f, ly, false);
        g.setGradientFill (line);
        g.fillRect (b.getX() + 15.0f, ly, b.getWidth() - 30.0f, 1.0f);

        g.setColour (juce::Colour::fromRGB (255, 165, 30));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, juce::Rectangle<float> (b.getX() + 15.0f, b.getY() + 7.0f, 320.0f, 16.0f),
                    juce::Justification::left);
    };

    drawPanel ({  15.0f,  75.0f, 390.0f, 240.0f }, U ("ETAPA DE SATURACIÓN"));
    drawPanel ({ 415.0f,  75.0f, 410.0f, 240.0f }, U ("ECUALIZACIÓN & DE-HARSH"));
    drawPanel ({  15.0f, 325.0f, 810.0f, 170.0f }, U ("TOPOLOGÍA & CONTROL DE SALIDA"));
}

//==============================================================================
void ScarlettWarmthAudioProcessorEditor::resized()
{
    // Perilla + valor + nombre, centrados en cx
    auto place = [] (Knob& k, int cx, int top, int size)
    {
        const int w = juce::jmax (100, size);
        k.slider.setBounds (cx - w / 2, top, w, size + 18);
        k.name.setBounds (cx - 60, top + size + 22, 120, 14);
    };

    // Panel de saturacion
    place (drive, 90,  112, 104);
    place (iron,  215, 112, 104);
    hpfLabel.setBounds (285, 140, 110, 14);
    hpfBox.setBounds   (285, 158, 105, 30);

    // Panel de ecualizacion (todas a la misma altura)
    place (chest, 483, 112, 104);
    place (anti,  620, 112, 104);
    place (silk,  757, 112, 104);

    // Panel inferior
    topologyLabel.setBounds (35, 362, 190, 14);
    topologyBox.setBounds   (35, 382, 190, 32);
    place (trim, 420, 354, 96);
    bypassButton.setBounds (530, 395, 120, 38);
    meter.setBounds (670, 350, 130, 105);
}
