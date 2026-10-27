#pragma once

#include "PluginProcessor.h"

//==============================================================================
// Perillas boutique: glow, cuerpo metalico, arco bipolar con muesca en el cero
class BoutiqueKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getLabelFont (juce::Label&) override { return juce::Font (13.0f, juce::Font::bold); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle,
                           juce::Slider& slider) override
    {
        const auto orange = juce::Colour::fromRGB (255, 140, 0);
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto c      = bounds.getCentre();
        auto angle  = startAngle + pos * (endAngle - startAngle);
        auto arcR   = radius - 2.0f;

        const bool  bipolar   = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
        const float zeroPos   = bipolar ? (float) slider.valueToProportionOfLength (0.0) : 0.0f;
        const float zeroAngle = startAngle + zeroPos * (endAngle - startAngle);

        // Pista de fondo
        juce::Path track;
        track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
        g.setColour (juce::Colour::fromRGB (30, 30, 30));
        g.strokePath (track, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Arco de valor con halo (parte del cero si es bipolar)
        if (std::abs (angle - zeroAngle) > 0.02f)
        {
            juce::Path arc;
            arc.addCentredArc (c.x, c.y, arcR, arcR, 0.0f,
                               juce::jmin (zeroAngle, angle), juce::jmax (zeroAngle, angle), true);
            for (int i = 3; i >= 1; --i)
            {
                g.setColour (orange.withAlpha (0.06f * (float) (4 - i)));
                g.strokePath (arc, juce::PathStrokeType (4.0f + (float) i * 3.0f,
                                                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            g.setColour (orange);
            g.strokePath (arc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Muesca en el punto cero
        if (bipolar)
        {
            juce::Point<float> p1 (c.x + (arcR + 5.0f) * std::sin (zeroAngle), c.y - (arcR + 5.0f) * std::cos (zeroAngle));
            juce::Point<float> p2 (c.x + (arcR + 9.0f) * std::sin (zeroAngle), c.y - (arcR + 9.0f) * std::cos (zeroAngle));
            g.setColour (juce::Colour::fromRGB (110, 110, 110));
            g.drawLine (p1.x, p1.y, p2.x, p2.y, 1.5f);
        }

        // Sombra + cuerpo metalico
        auto kr = radius - 9.0f;
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillEllipse (c.x - kr, c.y - kr + 3.0f, kr * 2.0f, kr * 2.0f);

        juce::ColourGradient body (juce::Colour::fromRGB (78, 78, 78), c.x - kr * 0.4f, c.y - kr * 0.5f,
                                   juce::Colour::fromRGB (14, 14, 14), c.x + kr, c.y + kr, true);
        g.setGradientFill (body);
        g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);

        // Borde: luz arriba, sombra abajo
        juce::ColourGradient rim (juce::Colour::fromRGB (120, 120, 120), c.x, c.y - kr,
                                  juce::Colour::fromRGB (25, 25, 25),   c.x, c.y + kr, false);
        g.setGradientFill (rim);
        g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.5f);

        // Puntero luminoso
        juce::Path pointer;
        pointer.addRoundedRectangle (-1.5f, -kr + 4.0f, 3.0f, kr * 0.45f, 1.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (c.x, c.y));
        g.setColour (orange.withAlpha (0.25f));
        g.strokePath (pointer, juce::PathStrokeType (5.0f));
        g.setColour (juce::Colour::fromRGB (255, 180, 60));
        g.fillPath (pointer);
    }
};

//==============================================================================
// Bypass estilo footswitch con LED (LED encendido = efecto activo)
class FootswitchLEDLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool highlighted, bool /*down*/) override
    {
        auto b = button.getLocalBounds().toFloat().reduced (2.0f);
        const bool bypassed = button.getToggleState();

        juce::ColourGradient face (juce::Colour::fromRGB (40, 40, 40), b.getX(), b.getY(),
                                   juce::Colour::fromRGB (18, 18, 18), b.getX(), b.getBottom(), false);
        g.setGradientFill (face);
        g.fillRoundedRectangle (b, 7.0f);
        g.setColour (highlighted ? juce::Colour::fromRGB (255, 140, 0).withAlpha (0.6f)
                                 : juce::Colour::fromRGB (65, 65, 65));
        g.drawRoundedRectangle (b, 7.0f, 1.5f);

        auto led = juce::Rectangle<float> (10.0f, 10.0f)
                       .withCentre (juce::Point<float> (b.getX() + 20.0f, b.getCentreY()));
        auto ledCol = bypassed ? juce::Colour::fromRGB (90, 30, 20) : juce::Colour::fromRGB (255, 120, 0);

        if (! bypassed)
            for (int i = 3; i >= 1; --i)
            {
                g.setColour (ledCol.withAlpha (0.10f * (float) (4 - i)));
                g.fillEllipse (led.expanded ((float) i * 2.5f));
            }

        g.setColour (ledCol);
        g.fillEllipse (led);
        g.setColour (juce::Colours::white.withAlpha (bypassed ? 0.08f : 0.45f));
        g.fillEllipse (led.reduced (3.0f).translated (-0.8f, -0.8f));

        g.setColour (bypassed ? juce::Colour::fromRGB (150, 150, 150) : juce::Colour::fromRGB (255, 175, 50));
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.drawText (button.getButtonText(), b.withTrimmedLeft (38.0f), juce::Justification::centredLeft);
    }
};

//==============================================================================
// Desplegables y menus oscuros con acento naranja
class DarkComboLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DarkComboLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (20, 20, 20));
        setColour (juce::ComboBox::outlineColourId,    juce::Colour::fromRGB (62, 62, 62));
        setColour (juce::ComboBox::textColourId,       juce::Colour::fromRGB (235, 235, 235));
        setColour (juce::ComboBox::arrowColourId,      juce::Colour::fromRGB (255, 140, 0));
        setColour (juce::PopupMenu::backgroundColourId,            juce::Colour::fromRGB (22, 22, 22));
        setColour (juce::PopupMenu::textColourId,                  juce::Colour::fromRGB (225, 225, 225));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour::fromRGB (255, 140, 0));
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::black);
    }
};

//==============================================================================
// Medidor LED de salida (reacciona al audio)
class LedMeter : public juce::Component, private juce::Timer
{
public:
    explicit LedMeter (std::function<float()> source) : getLevel (std::move (source)) { startTimerHz (30); }

    void timerCallback() override
    {
        auto target = juce::jlimit (0.0f, 1.0f, getLevel());
        level = target > level ? target : level * 0.85f + target * 0.15f;   // ataque rapido, caida suave
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto full = getLocalBounds().toFloat();
        auto b = full;

        g.setColour (juce::Colour::fromRGB (12, 12, 12));
        g.fillRoundedRectangle (full, 6.0f);
        g.setColour (juce::Colour::fromRGB (40, 40, 40));
        g.drawRoundedRectangle (full.reduced (0.5f), 6.0f, 1.0f);

        g.setColour (juce::Colour::fromRGB (120, 120, 120));
        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.drawText ("OUTPUT METER", b.removeFromTop (24.0f), juce::Justification::centred);

        const int n = 12;
        auto area = b.reduced (12.0f, 12.0f);
        const float gap = 3.0f;
        const float w = (area.getWidth() - gap * (float) (n - 1)) / (float) n;

        for (int i = 0; i < n; ++i)
        {
            auto col = i < 8  ? juce::Colour::fromRGB (255, 140, 0)
                     : i < 10 ? juce::Colour::fromRGB (255, 205, 40)
                              : juce::Colour::fromRGB (230, 55, 45);
            const bool on = level > ((float) i + 0.5f) / (float) n;
            juce::Rectangle<float> seg (area.getX() + (float) i * (w + gap), area.getY(), w, area.getHeight());

            if (on)
            {
                g.setColour (col.withAlpha (0.18f));
                g.fillRoundedRectangle (seg.expanded (2.0f), 3.0f);
            }
            g.setColour (on ? col : col.withAlpha (0.10f));
            g.fillRoundedRectangle (seg, 1.5f);
        }
    }

private:
    std::function<float()> getLevel;
    float level = 0.0f;
};

//==============================================================================
class ScarlettWarmthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ScarlettWarmthAudioProcessorEditor (ScarlettWarmthAudioProcessor&);
    ~ScarlettWarmthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  name;
        std::unique_ptr<APVTS::SliderAttachment> attach;   // se destruye antes que el slider
    };

    void setupKnob (Knob&, const char* paramID, const juce::String& title, const juce::String& unit);
    void setupCombo (juce::ComboBox&, std::unique_ptr<APVTS::ComboBoxAttachment>&, const char* paramID);
    void setupSmallLabel (juce::Label&, const juce::String& text);

    ScarlettWarmthAudioProcessor& audioProcessor;

    BoutiqueKnobLookAndFeel   knobLnF;
    FootswitchLEDLookAndFeel  switchLnF;
    DarkComboLookAndFeel      comboLnF;

    Knob drive, iron, chest, anti, silk, trim;

    juce::Label hpfLabel, topologyLabel;
    juce::ComboBox hpfBox, topologyBox;
    std::unique_ptr<APVTS::ComboBoxAttachment> hpfAttach, topologyAttach;

    juce::ToggleButton bypassButton { "BYPASS" };
    std::unique_ptr<APVTS::ButtonAttachment> bypassAttach;

    LedMeter meter { [this] { return audioProcessor.outputLevel.load(); } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessorEditor)
};
