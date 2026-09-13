#pragma once

#include "TuningModel.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <algorithm>

class PresetManager
{
public:
    PresetManager()
    {
        loadFactoryPresets();
        loadUserPresets();
    }

    const std::vector<Tunings::TuningPreset>& getAllPresets() const
    {
        return allPresets;
    }

    std::vector<std::string> getCategories() const
    {
        std::vector<std::string> categories;
        for (const auto& p : allPresets)
        {
            if (std::find(categories.begin(), categories.end(), p.category) == categories.end())
                categories.push_back(p.category);
        }
        return categories;
    }

    bool isCurrentPresetFactory(int globalIndex) const
    {
        if (globalIndex >= 0 && globalIndex < static_cast<int>(allPresets.size()))
            return allPresets[static_cast<size_t>(globalIndex)].isFactory;
        return true;
    }

    bool saveUserPreset(const std::string& name,
                        int semitones,
                        double refHz,
                        double fineCents,
                        const std::string& guide,
                        const std::string& description,
                        int* outNewIndex = nullptr)
    {
        // Check if attempting to overwrite a factory preset
        for (const auto& p : allPresets)
        {
            if (p.name == name && p.isFactory)
            {
                return false; // Cannot overwrite factory presets
            }
        }

        // Check if overwriting existing user preset
        for (size_t i = 0; i < allPresets.size(); ++i)
        {
            if (allPresets[i].name == name && !allPresets[i].isFactory)
            {
                allPresets[i].pitchShiftSemitones = semitones;
                allPresets[i].referenceA4Hz = refHz;
                allPresets[i].fineCentsOffset = fineCents;
                allPresets[i].physicalTuningGuide = guide;
                allPresets[i].description = description;
                if (outNewIndex) *outNewIndex = static_cast<int>(i);
                saveUserPresetsToDisk();
                return true;
            }
        }

        // Create new user preset
        Tunings::TuningPreset newPreset;
        newPreset.category = "User Presets";
        newPreset.name = name;
        newPreset.description = description;
        newPreset.physicalTuningGuide = guide;
        newPreset.pitchShiftSemitones = semitones;
        newPreset.referenceA4Hz = refHz;
        newPreset.fineCentsOffset = fineCents;
        newPreset.isFactory = false;

        allPresets.push_back(newPreset);
        if (outNewIndex) *outNewIndex = static_cast<int>(allPresets.size() - 1);
        saveUserPresetsToDisk();
        return true;
    }

    bool deleteUserPreset(int globalIndex)
    {
        if (globalIndex < 0 || globalIndex >= static_cast<int>(allPresets.size()))
            return false;

        if (allPresets[static_cast<size_t>(globalIndex)].isFactory)
            return false; // Factory presets cannot be deleted

        allPresets.erase(allPresets.begin() + globalIndex);
        saveUserPresetsToDisk();
        return true;
    }

private:
    std::vector<Tunings::TuningPreset> allPresets;

    void loadFactoryPresets()
    {
        allPresets = Tunings::getBuiltInPresets();
    }

    juce::File getUserPresetFile() const
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("TheEdgeOfFear")
                   .getChildFile("DontDropTheHammer");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("UserPresets.xml");
    }

    void loadUserPresets()
    {
        auto file = getUserPresetFile();
        if (!file.existsAsFile()) return;

        auto xml = juce::XmlDocument::parse(file);
        if (xml == nullptr || !xml->hasTagName("UserPresets")) return;

        for (auto* child : xml->getChildIterator())
        {
            if (child->hasTagName("Preset"))
            {
                Tunings::TuningPreset p;
                p.category = "User Presets";
                p.name = child->getStringAttribute("name").toStdString();
                p.description = child->getStringAttribute("description").toStdString();
                p.physicalTuningGuide = child->getStringAttribute("tuningGuide").toStdString();
                p.pitchShiftSemitones = child->getIntAttribute("pitchShift", 0);
                p.referenceA4Hz = child->getDoubleAttribute("referenceHz", 440.0);
                p.fineCentsOffset = child->getDoubleAttribute("fineCents", 0.0);
                p.isFactory = false;
                allPresets.push_back(p);
            }
        }
    }

    void saveUserPresetsToDisk()
    {
        juce::XmlElement xml("UserPresets");
        for (const auto& p : allPresets)
        {
            if (!p.isFactory)
            {
                auto* child = xml.createNewChildElement("Preset");
                child->setAttribute("name", juce::String(p.name));
                child->setAttribute("description", juce::String(p.description));
                child->setAttribute("tuningGuide", juce::String(p.physicalTuningGuide));
                child->setAttribute("pitchShift", p.pitchShiftSemitones);
                child->setAttribute("referenceHz", p.referenceA4Hz);
                child->setAttribute("fineCents", p.fineCentsOffset);
            }
        }
        xml.writeTo(getUserPresetFile());
    }
};
