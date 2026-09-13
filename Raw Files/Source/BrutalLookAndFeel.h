#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

class BrutalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BrutalLookAndFeel()
    {
        // Theme: Black, Greys, White, Red, Dark Red, Neon Blue
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff08090b));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121419));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2d3340));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00f0ff)); // Neon Blue
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff101216));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff800a14)); // Dark Red
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1c2028));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xff00f0ff)); // Neon Blue
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);

        const float radius = static_cast<float>(std::min(width, height)) * 0.5f - 4.0f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Outer chassis shadow and dark bezel (Black / Dark Grey)
        g.setColour(juce::Colour(0xff050608));
        g.fillEllipse(centreX - radius - 2.0f, centreY - radius - 1.0f, (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f);

        // LED Arc Track (Background Grey)
        juce::Path trackPath;
        trackPath.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff181b22));
        g.strokePath(trackPath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active LED Arc (Glowing Blood Red with Dark Red base)
        if (sliderPosProportional > 0.001f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);
            g.setColour(juce::Colour(0x44ff1e2e));
            g.strokePath(activeArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(juce::Colour(0xffff1e2e)); // Blood Red
            g.strokePath(activeArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Inner Knurled Metallic Knob Body (Grey / Slate / Black gradient)
        const float knobRadius = radius - 8.0f;
        juce::ColourGradient knobGrad(juce::Colour(0xff363b47), centreX, centreY - knobRadius,
                                      juce::Colour(0xff121418), centreX, centreY + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Machined edge ring (Slate Grey)
        g.setColour(juce::Colour(0xff4e5666));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Grip notches
        const int numGrips = 16;
        g.setColour(juce::Colour(0xff090a0d));
        for (int i = 0; i < numGrips; ++i)
        {
            const float a = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / static_cast<float>(numGrips));
            const float gx = centreX + (knobRadius - 2.5f) * std::sin(a);
            const float gy = centreY - (knobRadius - 2.5f) * std::cos(a);
            g.fillEllipse(gx - 1.0f, gy - 1.0f, 2.0f, 2.0f);
        }

        // Pointer Line / White notch with red glow
        juce::Path pointer;
        const float pointerLength = knobRadius - 3.5f;
        pointer.startNewSubPath(centreX, centreY);
        pointer.lineTo(centreX + pointerLength * std::sin(angle),
                       centreY - pointerLength * std::cos(angle));

        g.setColour(juce::Colour(0xffff1e2e));
        g.strokePath(pointer, juce::PathStrokeType(3.2f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

        g.setColour(juce::Colours::white);
        g.strokePath(pointer, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        juce::Colour base = backgroundColour;
        if (shouldDrawButtonAsDown)
            base = base.darker(0.35f);
        else if (shouldDrawButtonAsHighlighted)
            base = base.brighter(0.2f);

        // Metallic beveled button (Black / Grey)
        juce::ColourGradient grad(base.brighter(0.12f), 0, bounds.getY(),
                                  base.darker(0.25f), 0, bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Precision border: Neon Blue on hover, Grey normally
        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(0xff00f0ff) : juce::Colour(0xff343a47));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);

        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

        juce::ColourGradient grad(juce::Colour(0xff1a1d24), 0, 0,
                                  juce::Colour(0xff0e1014), 0, static_cast<float>(height), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(juce::Colour(0xff343b4a));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

        // Neon Blue Arrow
        const float arrowX = static_cast<float>(width) - 16.0f;
        const float arrowY = static_cast<float>(height) * 0.5f;

        juce::Path arrow;
        arrow.startNewSubPath(arrowX - 5.0f, arrowY - 3.0f);
        arrow.lineTo(arrowX, arrowY + 3.0f);
        arrow.lineTo(arrowX + 5.0f, arrowY - 3.0f);

        g.setColour(juce::Colour(0xff00f0ff));
        g.strokePath(arrow, juce::PathStrokeType(2.0f));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        auto bounds = button.getLocalBounds().toFloat();
        const float toggleW = 34.0f;
        const float toggleH = 18.0f;
        const float toggleY = bounds.getCentreY() - (toggleH * 0.5f);
        const float toggleX = bounds.getX() + 2.0f;

        const bool isOn = button.getToggleState();

        // Switch Slot / Well (Black with Grey border)
        auto slotRect = juce::Rectangle<float>(toggleX, toggleY, toggleW, toggleH);
        g.setColour(juce::Colour(0xff08090c));
        g.fillRoundedRectangle(slotRect, toggleH * 0.5f);
        g.setColour(juce::Colour(0xff252933));
        g.drawRoundedRectangle(slotRect, toggleH * 0.5f, 1.2f);

        // Switch Bat / Thumb
        const float thumbDiameter = toggleH - 4.0f;
        const float thumbX = isOn ? (toggleX + toggleW - thumbDiameter - 2.0f) : (toggleX + 2.0f);
        const float thumbY = toggleY + 2.0f;

        // Active Neon Blue glow aura if ON
        if (isOn)
        {
            g.setColour(juce::Colour(0x5500f0ff));
            g.fillEllipse(thumbX - 3.0f, thumbY - 3.0f, thumbDiameter + 6.0f, thumbDiameter + 6.0f);
        }

        // Thumb: Neon Blue when ON, Slate Grey when OFF
        juce::ColourGradient thumbGrad(isOn ? juce::Colour(0xff00f0ff) : juce::Colour(0xff687080),
                                       thumbX, thumbY,
                                       isOn ? juce::Colour(0xff007799) : juce::Colour(0xff242730),
                                       thumbX, thumbY + thumbDiameter, false);
        g.setGradientFill(thumbGrad);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

        g.setColour(isOn ? juce::Colour(0xffb3f5ff) : juce::Colour(0xff9ea8ba));
        g.drawEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter, 1.0f);

        // Text label: White when ON, Grey when OFF
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(isOn ? juce::Colours::white : juce::Colour(0xff8c94a4));
        g.drawText(button.getButtonText(),
                   juce::Rectangle<float>(toggleX + toggleW + 8.0f, bounds.getY(),
                                          bounds.getWidth() - toggleW - 10.0f, bounds.getHeight()),
                   juce::Justification::centredLeft);
    }
};

// Custom Slider that automatically resets to 440.0 on double-click
class BrutalSlider : public juce::Slider
{
public:
    explicit BrutalSlider(double defaultVal = 440.0)
        : defaultValue(defaultVal)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 18);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0c0e12));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2a303d));
    }

    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        setValue(defaultValue, juce::sendNotification);
    }

    void setDefaultResetValue(double val)
    {
        defaultValue = val;
    }

    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        juce::Slider::mouseDown(e);
    }

private:
    double defaultValue{440.0};
};
