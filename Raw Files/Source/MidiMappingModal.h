#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiManager.h"

class MidiMappingModal : public juce::Component
{
public:
    MidiMappingModal(MidiManager& manager, std::function<void()> onCloseCallback)
        : midiManager(manager), onClose(onCloseCallback)
    {
        closeButton.setButtonText("X");
        closeButton.onClick = [this]() {
            if (onClose) onClose();
        };
        addAndMakeVisible(closeButton);

        addButton.setButtonText("+");
        addButton.onClick = [this]() {
            MidiMapping newMap;
            newMap.type = MidiMappingType::CC_Absolute;
            newMap.parameterId = "pedalPos";
            newMap.displayName = "Pedal Position";
            newMap.channel = 1;
            newMap.controlNumber = 11;
            midiManager.addMapping(newMap);
            rebuildRows();
        };
        addAndMakeVisible(addButton);

        rebuildRows();
    }

    void rebuildRows()
    {
        rows.clear();
        const auto& list = midiManager.getMappings();

        for (int i = 0; i < static_cast<int>(list.size()); ++i)
        {
            auto* row = rows.add(new RowComponent(midiManager, i, [this]() {
                rebuildRows();
            }));
            addAndMakeVisible(row);
        }

        resized();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // Dark translucent overlay modal
        g.setColour(juce::Colour(0xe6131417));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
        g.setColour(juce::Colour(0xff3a3f4a));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.5f);

        // Header Title
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("MIDI Mappings", 20, 12, getWidth() - 40, 24, juce::Justification::centred);

        // Table Header Columns
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff8c92a0));
        g.drawText("Type", 40, 42, 140, 20, juce::Justification::centredLeft);
        g.drawText("Parameter/Preset", 190, 42, 160, 20, juce::Justification::centredLeft);
        g.drawText("Channel", 360, 42, 70, 20, juce::Justification::centredLeft);
        g.drawText("Note/CC/PC", 440, 42, 110, 20, juce::Justification::centredLeft);
    }

    void resized() override
    {
        closeButton.setBounds(getWidth() - 32, 10, 22, 22);

        int y = 68;
        const int rowH = 34;

        for (auto* row : rows)
        {
            row->setBounds(12, y, getWidth() - 24, rowH);
            y += rowH + 4;
        }

        addButton.setBounds(16, y + 4, 30, 28);
    }

private:
    class RowComponent : public juce::Component
    {
    public:
        RowComponent(MidiManager& mgr, int index, std::function<void()> onUpdate)
            : manager(mgr), rowIndex(index), notifyUpdate(onUpdate)
        {
            const auto& m = manager.getMappings()[static_cast<size_t>(index)];

            // Type ComboBox
            typeBox.addItem("CC Absolute", 1);
            typeBox.addItem("CC Relative", 2);
            typeBox.addItem("CC Toggle", 3);
            typeBox.addItem("CC Gate", 4);
            typeBox.addItem("CC Preset", 5);
            typeBox.addItem("Note Toggle", 6);
            typeBox.addItem("Note Preset", 7);
            typeBox.setSelectedId(static_cast<int>(m.type) + 1, juce::dontSendNotification);
            typeBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                item.type = static_cast<MidiMappingType>(typeBox.getSelectedId() - 1);
            };
            addAndMakeVisible(typeBox);

            // Parameter ComboBox
            paramBox.addItem("Pedal Position", 1);
            paramBox.addItem("Pitch Shift Semitones", 2);
            paramBox.addItem("Reference Frequency (440)", 3);
            paramBox.addItem("Pedal Bypass", 4);
            paramBox.addItem("2x Octave Toggle", 5);
            paramBox.addItem("Tuner Pre/Post Source", 6);
            paramBox.addItem("Input Gain", 7);
            paramBox.addItem("Output Gain", 8);

            if (m.parameterId == "pedalPos")         paramBox.setSelectedId(1, juce::dontSendNotification);
            else if (m.parameterId == "pitchShift")  paramBox.setSelectedId(2, juce::dontSendNotification);
            else if (m.parameterId == "referenceHz") paramBox.setSelectedId(3, juce::dontSendNotification);
            else if (m.parameterId == "pedalBypass") paramBox.setSelectedId(4, juce::dontSendNotification);
            else if (m.parameterId == "pedalOctave2")paramBox.setSelectedId(5, juce::dontSendNotification);
            else if (m.parameterId == "tunerSource") paramBox.setSelectedId(6, juce::dontSendNotification);
            else if (m.parameterId == "inputGain")   paramBox.setSelectedId(7, juce::dontSendNotification);
            else if (m.parameterId == "outputGain")  paramBox.setSelectedId(8, juce::dontSendNotification);
            else paramBox.setSelectedId(1, juce::dontSendNotification);

            paramBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                switch (paramBox.getSelectedId())
                {
                    case 1: item.parameterId = "pedalPos"; item.displayName = "Pedal Position"; break;
                    case 2: item.parameterId = "pitchShift"; item.displayName = "Pitch Shift"; break;
                    case 3: item.parameterId = "referenceHz"; item.displayName = "Reference Freq"; break;
                    case 4: item.parameterId = "pedalBypass"; item.displayName = "Pedal Bypass"; break;
                    case 5: item.parameterId = "pedalOctave2"; item.displayName = "2x Octave"; break;
                    case 6: item.parameterId = "tunerSource"; item.displayName = "Tuner Source"; break;
                    case 7: item.parameterId = "inputGain"; item.displayName = "Input Gain"; break;
                    case 8: item.parameterId = "outputGain"; item.displayName = "Output Gain"; break;
                }
            };
            addAndMakeVisible(paramBox);

            // Channel ComboBox
            channelBox.addItem("Omni", 1);
            for (int ch = 1; ch <= 16; ++ch)
                channelBox.addItem(juce::String(ch), ch + 1);
            channelBox.setSelectedId(m.channel == 0 ? 1 : m.channel + 1, juce::dontSendNotification);
            channelBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                item.channel = channelBox.getSelectedId() - 1;
            };
            addAndMakeVisible(channelBox);

            // CC / Control ComboBox
            for (int cc = 0; cc <= 127; ++cc)
                ccBox.addItem("CC #" + juce::String(cc), cc + 1);
            ccBox.setSelectedId(m.controlNumber + 1, juce::dontSendNotification);
            ccBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                item.controlNumber = ccBox.getSelectedId() - 1;
            };
            addAndMakeVisible(ccBox);

            // Delete button
            deleteButton.setButtonText("X");
            deleteButton.onClick = [this]() {
                manager.removeMapping(rowIndex);
                if (notifyUpdate) notifyUpdate();
            };
            addAndMakeVisible(deleteButton);
        }

        void paint(juce::Graphics& g) override
        {
            // Indicator dot
            g.setColour(juce::Colours::white);
            g.fillEllipse(10.0f, static_cast<float>(getHeight()) * 0.5f - 3.5f, 7.0f, 7.0f);
        }

        void resized() override
        {
            const int h = getHeight();
            typeBox.setBounds(28, 3, 140, h - 6);
            paramBox.setBounds(176, 3, 160, h - 6);
            channelBox.setBounds(344, 3, 65, h - 6);
            ccBox.setBounds(418, 3, 110, h - 6);
            deleteButton.setBounds(getWidth() - 32, 4, 24, h - 8);
        }

    private:
        MidiManager& manager;
        int rowIndex;
        std::function<void()> notifyUpdate;
        juce::ComboBox typeBox;
        juce::ComboBox paramBox;
        juce::ComboBox channelBox;
        juce::ComboBox ccBox;
        juce::TextButton deleteButton;
    };

    MidiManager& midiManager;
    std::function<void()> onClose;
    juce::TextButton closeButton;
    juce::TextButton addButton;
    juce::OwnedArray<RowComponent> rows;
};
