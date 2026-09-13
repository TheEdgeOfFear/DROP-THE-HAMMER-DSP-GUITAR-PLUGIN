#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>

enum class MidiMappingType
{
    CC_Absolute,
    CC_Relative,
    CC_Toggle,
    CC_Gate,
    CC_Preset,
    Note_Toggle,
    Note_Preset,
    Program_Change
};

inline std::string getMidiMappingTypeName(MidiMappingType t)
{
    switch (t)
    {
        case MidiMappingType::CC_Absolute:    return "CC Absolute";
        case MidiMappingType::CC_Relative:    return "CC Relative";
        case MidiMappingType::CC_Toggle:      return "CC Toggle";
        case MidiMappingType::CC_Gate:        return "CC Gate";
        case MidiMappingType::CC_Preset:      return "CC Preset";
        case MidiMappingType::Note_Toggle:    return "Note Toggle";
        case MidiMappingType::Note_Preset:    return "Note Preset";
        case MidiMappingType::Program_Change: return "Program Change";
        default: return "CC Absolute";
    }
}

struct MidiMapping
{
    MidiMappingType type{MidiMappingType::CC_Absolute};
    std::string parameterId;   // e.g. "pedalPos", "pitchShift", "pedalBypass"
    std::string displayName;   // e.g. "Pedal Position"
    int channel{1};            // 0 = Omni, 1-16
    int controlNumber{11};     // CC number, Note number, or Program number
    float minValue{0.0f};
    float maxValue{1.0f};
};

class MidiManager
{
public:
    MidiManager()
    {
        // Default factory MIDI mappings
        mappings.push_back({MidiMappingType::CC_Absolute, "pedalPos", "Pedal Position", 0, 11, 0.0f, 1.0f}); // Expression CC #11
        mappings.push_back({MidiMappingType::CC_Toggle,   "pedalBypass", "Pedal Bypass", 0, 64, 0.0f, 1.0f});  // Sustain Pedal CC #64
        mappings.push_back({MidiMappingType::CC_Toggle,   "pedalOctave2", "2x Octave Toggle", 0, 65, 0.0f, 1.0f});
    }

    void setLearningParameter(const std::string& paramId, const std::string& dispName)
    {
        learningParamId = paramId;
        learningDisplayName = dispName;
        isLearning = true;
    }

    bool getIsLearning() const { return isLearning; }
    std::string getLearningParamId() const { return learningParamId; }

    void cancelLearning()
    {
        isLearning = false;
        learningParamId.clear();
        learningDisplayName.clear();
    }

    const std::vector<MidiMapping>& getMappings() const { return mappings; }
    std::vector<MidiMapping>& getMappings() { return mappings; }

    void addMapping(const MidiMapping& m)
    {
        mappings.push_back(m);
    }

    void removeMapping(int index)
    {
        if (index >= 0 && index < static_cast<int>(mappings.size()))
            mappings.erase(mappings.begin() + index);
    }

    // Process incoming MIDI messages in audio thread
    void processMidi(const juce::MidiBuffer& midiMessages,
                     juce::AudioProcessorValueTreeState& apvts,
                     std::function<void(int presetIndex)> onPresetChange)
    {
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();

            if (msg.isController())
            {
                const int ch = msg.getChannel();
                const int cc = msg.getControllerNumber();
                const int val = msg.getControllerValue();

                if (isLearning && !learningParamId.empty())
                {
                    // Found MIDI event during learn: bind mapping!
                    MidiMapping newMap;
                    newMap.type = MidiMappingType::CC_Absolute;
                    newMap.parameterId = learningParamId;
                    newMap.displayName = learningDisplayName;
                    newMap.channel = ch;
                    newMap.controlNumber = cc;

                    // If toggle parameter, set CC Toggle
                    if (learningParamId == "pedalBypass" || learningParamId == "pedalOctave2" || learningParamId == "tunerSource")
                        newMap.type = MidiMappingType::CC_Toggle;

                    mappings.push_back(newMap);
                    isLearning = false;
                    learningParamId.clear();
                    learningDisplayName.clear();
                    continue;
                }

                // Apply active mappings
                for (auto& m : mappings)
                {
                    if ((m.channel == 0 || m.channel == ch) && m.controlNumber == cc)
                    {
                        const float normalized = static_cast<float>(val) / 127.0f;

                        if (m.type == MidiMappingType::CC_Absolute)
                        {
                            if (auto* param = apvts.getParameter(m.parameterId))
                                param->setValueNotifyingHost(normalized);
                        }
                        else if (m.type == MidiMappingType::CC_Toggle)
                        {
                            if (val >= 64)
                            {
                                if (auto* param = apvts.getParameter(m.parameterId))
                                {
                                    const float current = param->getValue();
                                    param->setValueNotifyingHost(current > 0.5f ? 0.0f : 1.0f);
                                }
                            }
                        }
                        else if (m.type == MidiMappingType::CC_Preset)
                        {
                            if (onPresetChange)
                                onPresetChange(val);
                        }
                    }
                }
            }
            else if (msg.isProgramChange())
            {
                const int prog = msg.getProgramChangeNumber();
                if (onPresetChange)
                    onPresetChange(prog);
            }
        }
    }

    juce::ValueTree exportToValueTree() const
    {
        juce::ValueTree tree("MidiMappings");
        for (const auto& m : mappings)
        {
            juce::ValueTree item("Mapping");
            item.setProperty("type", static_cast<int>(m.type), nullptr);
            item.setProperty("paramId", juce::String(m.parameterId), nullptr);
            item.setProperty("dispName", juce::String(m.displayName), nullptr);
            item.setProperty("channel", m.channel, nullptr);
            item.setProperty("cc", m.controlNumber, nullptr);
            tree.addChild(item, -1, nullptr);
        }
        return tree;
    }

    void importFromValueTree(const juce::ValueTree& tree)
    {
        if (!tree.hasType("MidiMappings")) return;
        mappings.clear();
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto item = tree.getChild(i);
            MidiMapping m;
            m.type = static_cast<MidiMappingType>(static_cast<int>(item.getProperty("type", 0)));
            m.parameterId = item.getProperty("paramId", "").toString().toStdString();
            m.displayName = item.getProperty("dispName", "").toString().toStdString();
            m.channel = item.getProperty("channel", 1);
            m.controlNumber = item.getProperty("cc", 11);
            mappings.push_back(m);
        }
    }

private:
    std::vector<MidiMapping> mappings;
    std::atomic<bool> isLearning{false};
    std::string learningParamId;
    std::string learningDisplayName;
};
