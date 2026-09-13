#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PitchDetector.h"

class StrobeTunerComponent : public juce::Component, public juce::Timer
{
public:
    StrobeTunerComponent()
    {
        startTimerHz(60); // 60 FPS smooth motion
    }

    ~StrobeTunerComponent() override
    {
        stopTimer();
    }

    void updateResult(const TunerDetectionResult& newResult)
    {
        currentResult = newResult;
    }

    void setReferenceA4(double hz)
    {
        referenceHz = hz;
    }

    void setSpeedFast(bool fast)
    {
        isFastSpeed = fast;
    }

    bool getSpeedFast() const { return isFastSpeed; }

    void setUseFlats(bool flats)
    {
        useFlats = flats;
    }

    bool getUseFlats() const { return useFlats; }

    void setPostPitchShift(bool post)
    {
        isPostShift = post;
    }

    bool getPostPitchShift() const { return isPostShift; }

    void setEngaged(bool engaged)
    {
        isEngaged = engaged;
        repaint();
    }

    bool getEngaged() const { return isEngaged; }

    std::function<void(bool engaged)> onEngageToggled;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick) { onRightClick(e); return; }
        }

        isEngaged = !isEngaged;
        if (onEngageToggled) onEngageToggled(isEngaged);
        repaint();
    }

    void timerCallback() override
    {
        if (isEngaged && currentResult.hasSignal)
        {
            smoothedCents += (currentResult.centsOffset - smoothedCents) * 0.18;

            const double speedMultiplier = isFastSpeed ? 0.08 : 0.04;
            strobePhase += smoothedCents * speedMultiplier;
            while (strobePhase >= juce::MathConstants<double>::twoPi) strobePhase -= juce::MathConstants<double>::twoPi;
            while (strobePhase < 0.0)                                 strobePhase += juce::MathConstants<double>::twoPi;
        }
        else
        {
            smoothedCents = 0.0;
        }

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Sleek translucent dark glass recessed display well floating on artwork
        g.setColour(juce::Colour(0x5506070a));
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(juce::Colour(0x55800a14));
        g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

        // Header Section (Generous width to prevent any truncation)
        auto headerArea = bounds.removeFromTop(30.0f).reduced(10.0f, 4.0f);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff00f0ff)); // Neon Blue
        g.drawText("LOCKON RADIAL STROBE", headerArea.removeFromLeft(150.0f), juce::Justification::centredLeft);

        // Pre / Post Source Badge or Standby
        juce::String modeText;
        if (!isEngaged)
            modeText = "STATUS: STANDBY (OFF)";
        else
            modeText = isPostShift ? "MONITOR: POST-SHIFT" : "MONITOR: PRE-RAW";

        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(!isEngaged ? juce::Colour(0xff606878) : (isPostShift ? juce::Colour(0xffff1e2e) : juce::Colour(0xff00f0ff)));
        g.drawText(modeText, headerArea.removeFromLeft(160.0f), juce::Justification::centredLeft);

        // Reference A4 text (Ample remaining space - zero truncation)
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffffffff)); // White
        g.drawText("A = " + juce::String(referenceHz, 1) + " Hz", headerArea, juce::Justification::centredRight);

        // Main Circular Radar Bounds
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getY() + bounds.getHeight() * 0.46f;
        const float maxRadius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.38f;

        // Circular background well
        g.setColour(juce::Colour(0xff060709));
        g.fillEllipse(centreX - maxRadius, centreY - maxRadius, maxRadius * 2.0f, maxRadius * 2.0f);
        g.setColour(juce::Colour(0xff1c202a));
        g.drawEllipse(centreX - maxRadius, centreY - maxRadius, maxRadius * 2.0f, maxRadius * 2.0f, 1.5f);

        // Dual-Ring Strobe Segments (Neon Blue in tune, Blood Red out of tune, dark if disengaged)
        if (isEngaged)
        {
            drawStrobeRing(g, centreX, centreY, maxRadius * 0.90f, maxRadius * 0.74f, 16, strobePhase);
            drawStrobeRing(g, centreX, centreY, maxRadius * 0.70f, maxRadius * 0.54f, 12, -strobePhase * 1.5);
        }
        else
        {
            drawStrobeRing(g, centreX, centreY, maxRadius * 0.90f, maxRadius * 0.74f, 16, 0.0);
            drawStrobeRing(g, centreX, centreY, maxRadius * 0.70f, maxRadius * 0.54f, 12, 0.0);
        }

        // Center Note Display Hub
        const float centerRadius = maxRadius * 0.44f;
        if (!isEngaged)
        {
            g.setColour(juce::Colour(0xff0d0e12));
            g.fillEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f);
            g.setColour(juce::Colour(0xff333a47));
            g.drawEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f, 1.8f);

            g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xff606878));
            g.drawText("OFF", juce::Rectangle<float>(centreX - centerRadius, centreY - centerRadius + 2.0f,
                                                      centerRadius * 2.0f, centerRadius * 1.3f), juce::Justification::centred);

            g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xff8890a0));
            g.drawText("CLICK TO ENGAGE", juce::Rectangle<float>(centreX - 70.0f, centreY + centerRadius + 6.0f, 140.0f, 20.0f), juce::Justification::centred);
            return;
        }

        juce::Colour centerColor = currentResult.hasSignal
                                   ? (currentResult.isInTune ? juce::Colour(0xff00f0ff) : juce::Colour(0xffff1e2e))
                                   : juce::Colour(0xff181a22);

        // In-tune glow aura
        if (currentResult.hasSignal && currentResult.isInTune)
        {
            g.setColour(juce::Colour(0x4400f0ff));
            g.fillEllipse(centreX - centerRadius - 8.0f, centreY - centerRadius - 8.0f,
                          (centerRadius + 8.0f) * 2.0f, (centerRadius + 8.0f) * 2.0f);
        }

        g.setColour(juce::Colour(0xff0d0e12));
        g.fillEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f);
        g.setColour(centerColor);
        g.drawEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f, 1.8f);

        // Note Name Text
        g.setFont(juce::FontOptions(34.0f, juce::Font::bold));
        g.setColour(currentResult.hasSignal ? juce::Colours::white : juce::Colour(0xff444b5c));
        juce::String displayNote = currentResult.hasSignal ? currentResult.noteName : "--";
        g.drawText(displayNote, juce::Rectangle<float>(centreX - centerRadius, centreY - centerRadius + 2.0f,
                                                        centerRadius * 2.0f, centerRadius * 1.3f), juce::Justification::centred);

        // In-tune laser lock bar
        const float barW = 38.0f;
        const float barH = 7.0f;
        g.setColour(currentResult.hasSignal ? centerColor : juce::Colour(0xff222733));
        g.fillRoundedRectangle(centreX - barW * 0.5f, centreY + centerRadius * 0.38f, barW, barH, 2.0f);

        // Cents readout
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.setColour(currentResult.hasSignal ? centerColor : juce::Colour(0xff606878));
        juce::String centsText = currentResult.hasSignal
                                 ? (currentResult.centsOffset > 0.0 ? "+" : "") + juce::String(currentResult.centsOffset, 1) + " ct"
                                 : "0.0 ct";
        g.drawText(centsText, juce::Rectangle<float>(centreX - 50.0f, centreY + centerRadius + 8.0f, 100.0f, 20.0f), juce::Justification::centred);

        // Bottom Fine Cents Needle Bar
        const float needleBarY = bounds.getBottom() - 20.0f;
        const float needleBarW = bounds.getWidth() * 0.76f;
        const float needleBarX = centreX - needleBarW * 0.5f;

        g.setColour(juce::Colour(0xff202530));
        g.drawHorizontalLine(static_cast<int>(needleBarY), needleBarX, needleBarX + needleBarW);
        g.drawVerticalLine(static_cast<int>(centreX), needleBarY - 6.0f, needleBarY + 6.0f);

        // Needle dot (Neon Blue)
        if (currentResult.hasSignal)
        {
            const float clampedCents = static_cast<float>(std::clamp(smoothedCents, -50.0, 50.0));
            const float needleX = centreX + (clampedCents / 50.0f) * (needleBarW * 0.5f);
            g.setColour(centerColor);
            g.fillEllipse(needleX - 4.0f, needleBarY - 4.0f, 8.0f, 8.0f);
        }
    }

private:
    TunerDetectionResult currentResult;
    double referenceHz{440.0};
    bool isEngaged{true};
    bool isFastSpeed{false};
    bool useFlats{false};
    bool isPostShift{false};
    double strobePhase{0.0};
    double smoothedCents{0.0};

    void drawStrobeRing(juce::Graphics& g, float cx, float cy, float outerR, float innerR, int numBars, double phase)
    {
        const float angleStep = juce::MathConstants<float>::twoPi / static_cast<float>(numBars);
        const float barAngleWidth = angleStep * 0.55f;

        for (int i = 0; i < numBars; ++i)
        {
            const float startA = static_cast<float>(phase) + static_cast<float>(i) * angleStep;
            const float endA = startA + barAngleWidth;

            juce::Path p;
            p.addCentredArc(cx, cy, outerR, outerR, 0.0f, startA, endA, true);
            p.lineTo(cx + innerR * std::sin(endA), cy - innerR * std::cos(endA));
            p.addCentredArc(cx, cy, innerR, innerR, 0.0f, endA, startA, false);
            p.closeSubPath();

            if (currentResult.hasSignal)
            {
                if (currentResult.isInTune)
                    g.setColour(juce::Colour(0xff00f0ff)); // Neon Blue
                else
                    g.setColour(juce::Colour(0xffff1e2e)); // Blood Red
            }
            else
            {
                g.setColour(juce::Colour(0xff1a1d26));
            }

            g.fillPath(p);
            g.setColour(juce::Colour(0xff08090c));
            g.strokePath(p, juce::PathStrokeType(1.0f));
        }
    }
};
