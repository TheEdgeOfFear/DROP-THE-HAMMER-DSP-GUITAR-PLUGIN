#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <BinaryData.h>
#include "BrutalLookAndFeel.h"

class ExpressionPedalComponent : public juce::Component
{
public:
    ExpressionPedalComponent()
    {
        loadVikingLidImage();
    }

    void setPedalPosition(float pos)
    {
        pedalPosition = std::clamp(pos, 0.0f, 1.0f);
        repaint();
    }

    float getPedalPosition() const { return pedalPosition; }

    void setBypassed(bool bypassed)
    {
        isBypassed = bypassed;
        repaint();
    }

    bool getBypassed() const { return isBypassed; }

    void setDoubleOctave(bool dbl)
    {
        isDoubleOctave = dbl;
        repaint();
    }

    bool getDoubleOctave() const { return isDoubleOctave; }

    void setPitchRange(int bottom, int top)
    {
        bottomPitch = bottom;
        topPitch = top;
        repaint();
    }

    int getBottomPitch() const { return bottomPitch; }
    int getTopPitch() const { return topPitch; }

    double getCurrentEffectiveShift() const
    {
        if (isBypassed)
            return 0.0;

        const double multiplier = isDoubleOctave ? 2.0 : 1.0;
        const double range = static_cast<double>(topPitch - bottomPitch) * multiplier;
        return (static_cast<double>(bottomPitch) * multiplier) + (static_cast<double>(pedalPosition) * range);
    }

    std::function<void(float newPosition)> onPositionChanged;
    std::function<void(bool bypassed)> onBypassToggled;
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

        // Footswitch area click
        if (footswitchBounds.contains(e.position))
        {
            isBypassed = !isBypassed;
            if (onBypassToggled) onBypassToggled(isBypassed);
            repaint();
            return;
        }

        // Rocker pedal click
        if (treadAreaBounds.contains(e.position))
        {
            updatePedalFromMouse(e.position.y);
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (treadAreaBounds.contains(e.position.x, treadAreaBounds.getY()))
        {
            updatePedalFromMouse(e.position.y);
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Left Rocker Tread & Right Faceplate split (100% transparent backplate)
        const float pedalWidth = bounds.getWidth() * 0.55f;
        treadAreaBounds = juce::Rectangle<float>(bounds.getX() + 6.0f,
                                                 bounds.getY() + 8.0f,
                                                 pedalWidth - 10.0f,
                                                 bounds.getHeight() - 88.0f);

        // 3D Rocker Pedal Plate (Tilts dynamically with pedalPosition)
        const float tiltOffset = (1.0f - pedalPosition) * 20.0f;
        auto rockerPlate = treadAreaBounds.reduced(4.0f);
        rockerPlate = rockerPlate.withTrimmedTop(tiltOffset).withTrimmedBottom(20.0f - tiltOffset);

        // Dynamic 3D Cast Drop Shadow underneath the Viking Coffin Lid
        const float shadowAlpha = 0.50f + 0.30f * (1.0f - pedalPosition);
        const float shadowOffsetY = 6.0f + 14.0f * (1.0f - pedalPosition);
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, shadowAlpha));
        g.fillEllipse(rockerPlate.getX() + 8.0f, rockerPlate.getBottom() - 14.0f + shadowOffsetY,
                      rockerPlate.getWidth() - 16.0f, 22.0f);

        // Draw the Ancient Viking Coffin Lid
        if (vikingLidImage.isValid())
        {
            // Dark iron rim underneath the carved wood
            auto baseRim = rockerPlate.expanded(3.0f);
            juce::ColourGradient baseGrad(juce::Colour(0xff22252c), baseRim.getX(), baseRim.getY(),
                                          juce::Colour(0xff0a0b0e), baseRim.getX(), baseRim.getBottom(), false);
            g.setGradientFill(baseGrad);
            g.fillRoundedRectangle(baseRim, 8.0f);
            g.setColour(juce::Colour(0xff3f4654));
            g.drawRoundedRectangle(baseRim, 8.0f, 1.4f);

            // Carved Viking coffin lid image
            g.drawImage(vikingLidImage, rockerPlate, juce::RectanglePlacement::stretchToFit);

            // Dynamic 3D lighting gradient overlay based on tilt angle
            if (pedalPosition < 0.99f)
            {
                // Toe raised: crisp metallic sheen highlight on the top tread
                juce::ColourGradient topLight(juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.15f * (1.0f - pedalPosition)),
                                              rockerPlate.getCentreX(), rockerPlate.getY(),
                                              juce::Colours::transparentBlack,
                                              rockerPlate.getCentreX(), rockerPlate.getY() + rockerPlate.getHeight() * 0.45f, false);
                g.setGradientFill(topLight);
                g.fillRoundedRectangle(rockerPlate, 6.0f);
            }
            else
            {
                // Toe down: shadow on upper half
                juce::ColourGradient toeDownShadow(juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 0.22f),
                                                   rockerPlate.getCentreX(), rockerPlate.getY(),
                                                   juce::Colours::transparentBlack,
                                                   rockerPlate.getCentreX(), rockerPlate.getY() + rockerPlate.getHeight() * 0.5f, false);
                g.setGradientFill(toeDownShadow);
                g.fillRoundedRectangle(rockerPlate, 6.0f);
            }
        }
        else
        {
            drawVectorDiamondPlate(g, rockerPlate);
        }

        // --- RIGHT FACEPLATE (LCD & Footswitch) ---
        auto rightFaceplate = juce::Rectangle<float>(treadAreaBounds.getRight() + 16.0f,
                                                     treadAreaBounds.getY(),
                                                     bounds.getRight() - treadAreaBounds.getRight() - 16.0f,
                                                     treadAreaBounds.getHeight());

        // Digital LCD Transposition Screen (Translucent Dark Red / Black glass)
        auto lcdBox = rightFaceplate.removeFromTop(68.0f);
        g.setColour(juce::Colour(0xcc0f0305)); // Deep Dark Red / Black translucent glass
        g.fillRoundedRectangle(lcdBox, 6.0f);
        g.setColour(juce::Colour(0xff800a14)); // Dark Red Border
        g.drawRoundedRectangle(lcdBox, 6.0f, 1.5f);

        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff8c94a2)); // Slate Grey
        g.drawText("PITCH TRANSPOSE", lcdBox.removeFromTop(18.0f), juce::Justification::centred);

        const double effectiveShift = getCurrentEffectiveShift();
        juce::String shiftStr = isBypassed ? "BYPASS" : ((effectiveShift >= 0.0 ? "+" : "") + juce::String(effectiveShift, 1) + " ST");
        g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
        g.setColour(isBypassed ? juce::Colour(0xff606673) : juce::Colour(0xffff1e2e)); // Blood Red
        g.drawText(shiftStr, lcdBox, juce::Justification::centred);

        rightFaceplate.removeFromTop(16.0f);

        // Mode Status
        auto statusBox = rightFaceplate.removeFromTop(36.0f);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(isDoubleOctave ? juce::Colour(0xff00f0ff) : juce::Colour(0xff606775)); // Neon Blue
        g.drawText(isDoubleOctave ? "MODE: 2X OCTAVE ON" : "MODE: 1X STANDARD", statusBox.removeFromTop(18.0f), juce::Justification::centred);

        g.setFont(juce::FontOptions(10.0f));
        g.setColour(juce::Colour(0xff8a92a0));
        g.drawText("RANGE: " + juce::String(bottomPitch) + " TO " + juce::String(topPitch) + " ST", statusBox, juce::Justification::centred);

        // Footswitch Section
        auto fsArea = rightFaceplate.removeFromBottom(108.0f);
        footswitchBounds = fsArea;

        const float fsX = fsArea.getCentreX();
        const float fsY = fsArea.getBottom() - 44.0f;

        // Glowing Blood Red Halo LED
        const float ledY = fsY - 34.0f;
        if (!isBypassed)
        {
            g.setColour(juce::Colour(0x66ff1e2e));
            g.fillEllipse(fsX - 11.0f, ledY - 11.0f, 22.0f, 22.0f);
            g.setColour(juce::Colour(0xffff1e2e)); // Blood Red
            g.fillEllipse(fsX - 5.0f, ledY - 5.0f, 10.0f, 10.0f);
            g.setColour(juce::Colours::white);
            g.fillEllipse(fsX - 2.0f, ledY - 2.0f, 4.0f, 4.0f);
        }
        else
        {
            g.setColour(juce::Colour(0xff2d060a)); // Dark Red / Dim
            g.fillEllipse(fsX - 5.0f, ledY - 5.0f, 10.0f, 10.0f);
        }
        g.setColour(juce::Colour(0xff4a5263));
        g.drawEllipse(fsX - 6.0f, ledY - 6.0f, 12.0f, 12.0f, 1.2f);

        // Chrome Hex Nut & Plunger
        drawHexNut(g, fsX, fsY, 20.0f);

        juce::ColourGradient plungerGrad(juce::Colour(0xffa8b1c0), fsX, fsY - 13.0f,
                                         juce::Colour(0xff343942), fsX, fsY + 13.0f, false);
        g.setGradientFill(plungerGrad);
        g.fillEllipse(fsX - 13.0f, fsY - 13.0f, 26.0f, 26.0f);
        g.setColour(juce::Colour(0xffc8d2e0));
        g.drawEllipse(fsX - 13.0f, fsY - 13.0f, 26.0f, 26.0f, 1.4f);

        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("ENGAGE", juce::Rectangle<float>(fsX - 40.0f, fsY + 18.0f, 80.0f, 16.0f), juce::Justification::centred);
    }

private:
    float pedalPosition{0.0f};
    bool isBypassed{false};
    bool isDoubleOctave{false};
    int bottomPitch{-12};
    int topPitch{12};

    juce::Image vikingLidImage;
    juce::Rectangle<float> treadAreaBounds;
    juce::Rectangle<float> footswitchBounds;

    void loadVikingLidImage()
    {
        // 1. Embedded binary asset
        if constexpr (BinaryData::Viking_Coffin_Lid_pngSize > 0)
        {
            vikingLidImage = juce::ImageFileFormat::loadFrom(BinaryData::Viking_Coffin_Lid_png,
                                                             static_cast<size_t>(BinaryData::Viking_Coffin_Lid_pngSize));
            if (vikingLidImage.isValid())
                return;
        }

        // 2. Direct path
        juce::File file(R"(C:\Coding\Tunings VST3\Images\Viking_Coffin_Lid.png)");
        if (file.existsAsFile())
        {
            vikingLidImage = juce::ImageFileFormat::loadFrom(file);
        }
    }

    void updatePedalFromMouse(float mouseY)
    {
        if (treadAreaBounds.getHeight() <= 0.0f) return;

        const float relativeY = mouseY - treadAreaBounds.getY();
        float newPos = 1.0f - (relativeY / treadAreaBounds.getHeight());
        newPos = std::clamp(newPos, 0.0f, 1.0f);

        if (std::abs(newPos - pedalPosition) > 0.001f)
        {
            pedalPosition = newPos;
            if (onPositionChanged)
                onPositionChanged(pedalPosition);
            repaint();
        }
    }

    void drawHexNut(juce::Graphics& g, float cx, float cy, float radius)
    {
        juce::Path hex;
        for (int i = 0; i < 6; ++i)
        {
            const float a = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / 6.0f);
            const float x = cx + radius * std::cos(a);
            const float y = cy + radius * std::sin(a);
            if (i == 0) hex.startNewSubPath(x, y);
            else        hex.lineTo(x, y);
        }
        hex.closeSubPath();

        juce::ColourGradient hexGrad(juce::Colour(0xff6a7382), cx, cy - radius,
                                     juce::Colour(0xff252930), cx, cy + radius, false);
        g.setGradientFill(hexGrad);
        g.fillPath(hex);
        g.setColour(juce::Colour(0xff8a95a8));
        g.strokePath(hex, juce::PathStrokeType(1.2f));
    }

    void drawVectorDiamondPlate(juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.saveState();
        g.reduceClipRegion(area.toNearestInt());

        const float stepY = 22.0f;
        const float stepX = 22.0f;

        for (float y = area.getY() + 10.0f; y < area.getBottom() + 10.0f; y += stepY)
        {
            int col = 0;
            for (float x = area.getX() + 10.0f; x < area.getRight() + 10.0f; x += stepX)
            {
                const bool alt = ((col++) % 2 == 0);

                juce::Path lug;
                if (alt)
                {
                    lug.startNewSubPath(x - 6.0f, y - 2.5f);
                    lug.lineTo(x + 6.0f, y + 2.5f);
                }
                else
                {
                    lug.startNewSubPath(x - 2.5f, y + 6.0f);
                    lug.lineTo(x + 2.5f, y - 6.0f);
                }

                g.setColour(juce::Colour(0xff060708));
                g.strokePath(lug, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                g.setColour(juce::Colour(0xff333a47));
                g.strokePath(lug, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
        g.restoreState();
    }
};
