#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "BrutalLookAndFeel.h"
#include "StrobeTunerComponent.h"
#include "ExpressionPedalComponent.h"
#include "MidiMappingModal.h"

class TuningsAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer,
                                    public juce::ComboBox::Listener,
                                    public juce::Button::Listener
{
public:
    explicit TuningsAudioProcessorEditor(TuningsAudioProcessor&);
    ~TuningsAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;

private:
    TuningsAudioProcessor& audioProcessor;
    BrutalLookAndFeel brutalLookAndFeel;

    // Header & Brand
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    // Top Preset Browser Bar (Neural DSP style)
    juce::TextButton prevPresetButton{"<"};
    juce::ComboBox categoryBox;
    juce::ComboBox presetBox;
    juce::TextButton nextPresetButton{">"};
    juce::TextButton savePresetButton{"SAVE"};
    juce::TextButton deletePresetButton{"DEL"};

    // Top Right Master Knobs
    BrutalSlider inputGainSlider{1.0};
    juce::Label inputGainLabel;
    BrutalSlider mixSlider{1.0};
    juce::Label mixLabel;
    BrutalSlider outputGainSlider{1.0};
    juce::Label outputGainLabel;
    BrutalSlider refFreqSlider{440.0};
    juce::Label refFreqLabel;
    juce::TextButton midiMenuButton{"MIDI"};

    // Physical Guitar Setup LED Ribbon
    juce::Label tuningGuideBanner;

    // --- LEFT HARDWARE UNIT: HAMMER WHAMMY EXPRESSION PEDAL ---
    ExpressionPedalComponent expressionPedal;
    BrutalSlider pedalBottomSlider{-12.0};
    juce::Label pedalBottomLabel;
    BrutalSlider pedalTopSlider{12.0};
    juce::Label pedalTopLabel;
    juce::ToggleButton doubleOctaveToggle{"2X OCTAVE"};

    // --- RIGHT HARDWARE UNIT: LOCKON PITCH & TUNER PEDAL ---
    // Section 1: Transposition
    juce::Label transposerHeaderLabel;
    juce::Label transposerDisplayBox;
    juce::ComboBox semitoneShiftBox;

    // Section 2: Strobe Tuner
    StrobeTunerComponent strobeTuner;
    juce::ToggleButton tunerEngageToggle{"ENGAGE TUNER"};
    juce::ToggleButton tunerSourceToggle{"POST PITCH SHIFT"};
    juce::ToggleButton tunerSpeedToggle{"FAST STROBE"};
    juce::ToggleButton tunerDisplayToggle{"FLATS (b)"};

    // Modal Overlays
    std::unique_ptr<MidiMappingModal> midiModal;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inGainAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outGainAttachment;
    std::unique_ptr<SliderAttachment> refFreqAttachment;
    std::unique_ptr<SliderAttachment> pedalBottomAttachment;
    std::unique_ptr<SliderAttachment> pedalTopAttachment;
    std::unique_ptr<ButtonAttachment> doubleOctaveAttachment;
    std::unique_ptr<ButtonAttachment> tunerEngageAttachment;
    std::unique_ptr<ButtonAttachment> tunerSourceAttachment;
    std::unique_ptr<ButtonAttachment> tunerSpeedAttachment;
    std::unique_ptr<ButtonAttachment> tunerDisplayAttachment;

    std::vector<std::string> categories;
    std::vector<int> filteredIndices;

    void populateCategories();
    void syncPresetUIFromProcessorState();
    void updatePresetDropdown();
    void updatePhysicalGuideDisplay();
    void updateTransposerDisplay(int semitones);
    void openMidiContextMenu(const std::string& paramId, const std::string& dispName);
    void promptSaveUserPreset();
    void selectPreviousPreset();
    void selectNextPreset();

    void loadBackgroundImage();

    juce::Image backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TuningsAudioProcessorEditor)
};
