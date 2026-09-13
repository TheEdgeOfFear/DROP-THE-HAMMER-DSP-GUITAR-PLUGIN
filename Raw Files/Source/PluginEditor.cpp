#include "PluginEditor.h"
#include <BinaryData.h>

TuningsAudioProcessorEditor::TuningsAudioProcessorEditor(TuningsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&brutalLookAndFeel);
    setResizable(false, false);
    loadBackgroundImage();
    setSize(1280, 714);

    // --- Top Preset Browser ---
    prevPresetButton.addListener(this);
    addAndMakeVisible(prevPresetButton);

    categoryBox.addListener(this);
    addAndMakeVisible(categoryBox);

    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    nextPresetButton.addListener(this);
    addAndMakeVisible(nextPresetButton);

    savePresetButton.addListener(this);
    addAndMakeVisible(savePresetButton);

    deletePresetButton.addListener(this);
    addAndMakeVisible(deletePresetButton);

    // --- Top Right Master Knobs ---
    inputGainSlider.setRange(0.0, 2.0, 0.01);
    inputGainSlider.setValue(1.0);
    inputGainSlider.setDefaultResetValue(1.0);
    inputGainSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("inputGain", "Input Gain");
    };
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("INPUT", juce::dontSendNotification);
    inputGainLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc8d0e0)); // High contrast light grey
    addAndMakeVisible(inputGainLabel);

    // Mix (Dry / Wet 0 - 100%)
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(1.0);
    mixSlider.setDefaultResetValue(1.0);
    mixSlider.setTextValueSuffix(" %");
    mixSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("mix", "Mix (0-100%)");
    };
    addAndMakeVisible(mixSlider);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1e2e)); // Blood Red
    addAndMakeVisible(mixLabel);

    // Reference Frequency Dial (420 to 460 Hz, Double click -> 440 Hz)
    refFreqSlider.setRange(420.0, 460.0, 0.1);
    refFreqSlider.setValue(440.0);
    refFreqSlider.setDefaultResetValue(440.0);
    refFreqSlider.setTextValueSuffix(" Hz");
    refFreqSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("referenceHz", "Reference Frequency (A4)");
    };
    addAndMakeVisible(refFreqSlider);

    refFreqLabel.setText("REF FREQ", juce::dontSendNotification);
    refFreqLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    refFreqLabel.setJustificationType(juce::Justification::centred);
    refFreqLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1e2e)); // Blood Red
    addAndMakeVisible(refFreqLabel);

    outputGainSlider.setRange(0.0, 2.0, 0.01);
    outputGainSlider.setValue(1.0);
    outputGainSlider.setDefaultResetValue(1.0);
    outputGainSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("outputGain", "Output Gain");
    };
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc8d0e0)); // High contrast light grey
    addAndMakeVisible(outputGainLabel);

    midiMenuButton.setButtonText("MIDI MAP");
    midiMenuButton.addListener(this);
    addAndMakeVisible(midiMenuButton);

    // --- Physical Guitar Setup Backlit LED Strip ---
    tuningGuideBanner.setText(audioProcessor.getCurrentTuningGuide(), juce::dontSendNotification);
    tuningGuideBanner.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    tuningGuideBanner.setJustificationType(juce::Justification::centred);
    tuningGuideBanner.setColour(juce::Label::backgroundColourId, juce::Colour(0x66140306)); // Deep Dark Red translucent
    tuningGuideBanner.setColour(juce::Label::outlineColourId, juce::Colour(0x88800a14));   // Dark Red Border
    tuningGuideBanner.setColour(juce::Label::textColourId, juce::Colour(0xff00f0ff));      // Neon Blue LED glow
    addAndMakeVisible(tuningGuideBanner);

    // --- LEFT UNIT: HAMMER WHAMMY EXPRESSION PEDAL ---
    expressionPedal.onPositionChanged = [this](float pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("pedalPos"))
            param->setValueNotifyingHost(pos);
    };
    expressionPedal.onBypassToggled = [this](bool bypassed) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("pedalBypass"))
            param->setValueNotifyingHost(bypassed ? 1.0f : 0.0f);
    };
    expressionPedal.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("pedalPos", "Expression Pedal Position");
    };
    addAndMakeVisible(expressionPedal);

    // Pedal Range Controls (Mounted on bottom of left unit)
    pedalBottomSlider.setRange(-24, 24, 1);
    pedalBottomSlider.setValue(-12);
    pedalBottomSlider.setDefaultResetValue(-12);
    pedalBottomSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("pedalBottom", "Pedal Bottom Pitch");
    };
    addAndMakeVisible(pedalBottomSlider);

    pedalBottomLabel.setText("BOTTOM", juce::dontSendNotification);
    pedalBottomLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    pedalBottomLabel.setJustificationType(juce::Justification::centred);
    pedalBottomLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8e95a2));
    addAndMakeVisible(pedalBottomLabel);

    pedalTopSlider.setRange(-24, 24, 1);
    pedalTopSlider.setValue(12);
    pedalTopSlider.setDefaultResetValue(12);
    pedalTopSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("pedalTop", "Pedal Top Pitch");
    };
    addAndMakeVisible(pedalTopSlider);

    pedalTopLabel.setText("TOP", juce::dontSendNotification);
    pedalTopLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    pedalTopLabel.setJustificationType(juce::Justification::centred);
    pedalTopLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8e95a2));
    addAndMakeVisible(pedalTopLabel);

    doubleOctaveToggle.addListener(this);
    addAndMakeVisible(doubleOctaveToggle);

    // --- RIGHT UNIT: LOCKON PITCH & TUNER STOMPBOX ---
    // Section 1: Pitch Transposition
    transposerHeaderLabel.setText("PITCH TRANSPOSITION ENGINE", juce::dontSendNotification);
    transposerHeaderLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    transposerHeaderLabel.setJustificationType(juce::Justification::centredLeft);
    transposerHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00f0ff)); // Neon Blue
    addAndMakeVisible(transposerHeaderLabel);

    transposerDisplayBox.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    transposerDisplayBox.setJustificationType(juce::Justification::centred);
    transposerDisplayBox.setColour(juce::Label::backgroundColourId, juce::Colour(0xff0b0d10)); // Black
    transposerDisplayBox.setColour(juce::Label::outlineColourId, juce::Colour(0xff800a14));    // Dark Red
    transposerDisplayBox.setColour(juce::Label::textColourId, juce::Colour(0xffff1e2e));       // Blood Red
    addAndMakeVisible(transposerDisplayBox);

    for (int st = -12; st <= 12; ++st)
    {
        semitoneShiftBox.addItem(Tunings::getSemitoneDescription(st), st + 13);
    }
    semitoneShiftBox.addListener(this);
    addAndMakeVisible(semitoneShiftBox);

    // Section 2: Strobe Tuner
    strobeTuner.onEngageToggled = [this](bool engaged) {
        if (auto* p = audioProcessor.getAPVTS().getParameter("tunerEngaged"))
            p->setValueNotifyingHost(engaged ? 1.0f : 0.0f);
        tunerEngageToggle.setToggleState(engaged, juce::dontSendNotification);
    };
    strobeTuner.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("tunerEngaged", "Engage Tuner");
    };
    addAndMakeVisible(strobeTuner);

    tunerEngageToggle.addListener(this);
    tunerEngageToggle.onStateChange = [this]() {
        strobeTuner.setEngaged(tunerEngageToggle.getToggleState());
    };
    addAndMakeVisible(tunerEngageToggle);

    tunerSourceToggle.addListener(this);
    addAndMakeVisible(tunerSourceToggle);

    tunerSpeedToggle.addListener(this);
    addAndMakeVisible(tunerSpeedToggle);

    tunerDisplayToggle.addListener(this);
    addAndMakeVisible(tunerDisplayToggle);

    // APVTS Attachments
    auto& apvts = audioProcessor.getAPVTS();
    inGainAttachment       = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainSlider);
    mixAttachment          = std::make_unique<SliderAttachment>(apvts, "mix", mixSlider);
    outGainAttachment      = std::make_unique<SliderAttachment>(apvts, "outputGain", outputGainSlider);
    refFreqAttachment      = std::make_unique<SliderAttachment>(apvts, "referenceHz", refFreqSlider);
    pedalBottomAttachment  = std::make_unique<SliderAttachment>(apvts, "pedalBottom", pedalBottomSlider);
    pedalTopAttachment     = std::make_unique<SliderAttachment>(apvts, "pedalTop", pedalTopSlider);
    doubleOctaveAttachment = std::make_unique<ButtonAttachment>(apvts, "pedalOctave2", doubleOctaveToggle);
    tunerEngageAttachment  = std::make_unique<ButtonAttachment>(apvts, "tunerEngaged", tunerEngageToggle);
    tunerSourceAttachment  = std::make_unique<ButtonAttachment>(apvts, "tunerSource", tunerSourceToggle);
    tunerSpeedAttachment   = std::make_unique<ButtonAttachment>(apvts, "tunerSpeed", tunerSpeedToggle);
    tunerDisplayAttachment = std::make_unique<ButtonAttachment>(apvts, "tunerDisplay", tunerDisplayToggle);

    syncPresetUIFromProcessorState();

    startTimerHz(40); // 40 FPS UI sync
}

TuningsAudioProcessorEditor::~TuningsAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void TuningsAudioProcessorEditor::populateCategories()
{
    syncPresetUIFromProcessorState();
}

void TuningsAudioProcessorEditor::syncPresetUIFromProcessorState()
{
    categories = audioProcessor.getPresetManager().getCategories();

    categoryBox.clear(juce::dontSendNotification);
    for (size_t i = 0; i < categories.size(); ++i)
        categoryBox.addItem(categories[i], static_cast<int>(i + 1));

    const auto& allPresets = audioProcessor.getPresetManager().getAllPresets();
    int activeGlobalIdx = audioProcessor.getSelectedPresetIndex();
    if (activeGlobalIdx < 0 || activeGlobalIdx >= static_cast<int>(allPresets.size()))
        activeGlobalIdx = 0;

    const auto& currentPreset = allPresets[static_cast<size_t>(activeGlobalIdx)];

    int targetCatIdx = 0;
    for (size_t i = 0; i < categories.size(); ++i)
    {
        if (categories[i] == currentPreset.category)
        {
            targetCatIdx = static_cast<int>(i);
            break;
        }
    }

    categoryBox.setSelectedId(targetCatIdx + 1, juce::dontSendNotification);

    // Populate presetBox for this category
    filteredIndices.clear();
    presetBox.clear(juce::dontSendNotification);
    int selectedPresetBoxId = 1;
    int id = 1;
    for (int i = 0; i < static_cast<int>(allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t>(i)].category == currentPreset.category)
        {
            if (i == activeGlobalIdx)
                selectedPresetBoxId = id;

            filteredIndices.push_back(i);
            presetBox.addItem(allPresets[static_cast<size_t>(i)].name, id++);
        }
    }

    presetBox.setSelectedId(selectedPresetBoxId, juce::dontSendNotification);

    // Sync physical tuning guide display
    updatePhysicalGuideDisplay();

    // Sync semitone transposer display
    const int currentSt = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("pitchShift")->load());
    semitoneShiftBox.setSelectedId(currentSt + 13, juce::dontSendNotification);
    updateTransposerDisplay(currentSt);
}

void TuningsAudioProcessorEditor::updatePresetDropdown()
{
    const int catIdx = categoryBox.getSelectedItemIndex();
    if (catIdx < 0 || catIdx >= static_cast<int>(categories.size()))
        return;

    const std::string& catName = categories[static_cast<size_t>(catIdx)];
    const auto& allPresets = audioProcessor.getPresetManager().getAllPresets();

    filteredIndices.clear();
    presetBox.clear(juce::dontSendNotification);

    int id = 1;
    for (int i = 0; i < static_cast<int>(allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t>(i)].category == catName)
        {
            filteredIndices.push_back(i);
            presetBox.addItem(allPresets[static_cast<size_t>(i)].name, id++);
        }
    }

    if (!filteredIndices.empty())
        presetBox.setSelectedId(1, juce::sendNotification);
}

void TuningsAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &categoryBox)
    {
        updatePresetDropdown();
    }
    else if (box == &presetBox)
    {
        const int itemIdx = presetBox.getSelectedItemIndex();
        if (itemIdx >= 0 && itemIdx < static_cast<int>(filteredIndices.size()))
        {
            const int globalIdx = filteredIndices[static_cast<size_t>(itemIdx)];
            audioProcessor.applyPreset(globalIdx);

            const auto& p = audioProcessor.getPresetManager().getAllPresets()[static_cast<size_t>(globalIdx)];

            // Directly set Reference Frequency Dial to preset frequency (e.g. 450.3 Hz for Machine Head, 425/435 for Pantera)
            refFreqSlider.setValue(p.referenceA4Hz, juce::sendNotification);

            // Sync semitoneShiftBox
            semitoneShiftBox.setSelectedId(p.pitchShiftSemitones + 13, juce::dontSendNotification);
            updateTransposerDisplay(p.pitchShiftSemitones);

            updatePhysicalGuideDisplay();
        }
    }
    else if (box == &semitoneShiftBox)
    {
        const int st = semitoneShiftBox.getSelectedId() - 13;
        if (auto* param = audioProcessor.getAPVTS().getParameter("pitchShift"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(st)));

        updateTransposerDisplay(st);
    }
}

void TuningsAudioProcessorEditor::updateTransposerDisplay(int semitones)
{
    juce::String text = juce::String(semitones > 0 ? "+" : "") + juce::String(semitones) + " ST  |  " + Tunings::getSemitoneDescription(semitones);
    transposerDisplayBox.setText(text, juce::dontSendNotification);
}

void TuningsAudioProcessorEditor::updatePhysicalGuideDisplay()
{
    tuningGuideBanner.setText(audioProcessor.getCurrentTuningGuide(), juce::dontSendNotification);
}

void TuningsAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &prevPresetButton)
    {
        selectPreviousPreset();
    }
    else if (b == &nextPresetButton)
    {
        selectNextPreset();
    }
    else if (b == &savePresetButton)
    {
        promptSaveUserPreset();
    }
    else if (b == &deletePresetButton)
    {
        const int currentIdx = audioProcessor.getSelectedPresetIndex();
        if (audioProcessor.getPresetManager().isCurrentPresetFactory(currentIdx))
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Cannot Delete Factory Preset",
                "Built-in Band Tunings and Factory presets are write-protected and cannot be deleted.",
                "OK");
        }
        else
        {
            audioProcessor.getPresetManager().deleteUserPreset(currentIdx);
            populateCategories();
        }
    }
    else if (b == &midiMenuButton)
    {
        if (midiModal == nullptr)
        {
            midiModal = std::make_unique<MidiMappingModal>(audioProcessor.getMidiManager(), [this]() {
                midiModal.reset();
                repaint();
            });
            midiModal->setBounds(getLocalBounds().reduced(80, 50));
            addAndMakeVisible(midiModal.get());
        }
        else
        {
            midiModal.reset();
            repaint();
        }
    }
}

void TuningsAudioProcessorEditor::selectPreviousPreset()
{
    int currentId = presetBox.getSelectedId();
    if (currentId > 1)
        presetBox.setSelectedId(currentId - 1, juce::sendNotification);
    else if (categoryBox.getSelectedId() > 1)
    {
        categoryBox.setSelectedId(categoryBox.getSelectedId() - 1, juce::sendNotification);
        presetBox.setSelectedId(presetBox.getNumItems(), juce::sendNotification);
    }
}

void TuningsAudioProcessorEditor::selectNextPreset()
{
    int currentId = presetBox.getSelectedId();
    if (currentId < presetBox.getNumItems())
        presetBox.setSelectedId(currentId + 1, juce::sendNotification);
    else if (categoryBox.getSelectedId() < categoryBox.getNumItems())
    {
        categoryBox.setSelectedId(categoryBox.getSelectedId() + 1, juce::sendNotification);
        presetBox.setSelectedId(1, juce::sendNotification);
    }
}

void TuningsAudioProcessorEditor::promptSaveUserPreset()
{
    auto* aw = new juce::AlertWindow("Save Custom Tuning Preset", "Enter a name for your custom preset:", juce::AlertWindow::NoIcon);
    aw->addTextEditor("presetName", "My Custom Metal Tuning");
    aw->addButton("Save", 1);
    aw->addButton("Cancel", 0);

    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result) {
        if (result == 1)
        {
            const auto name = aw->getTextEditorContents("presetName").toStdString();
            if (!name.empty())
            {
                const int st = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("pitchShift")->load());
                const double hz = audioProcessor.getAPVTS().getRawParameterValue("referenceHz")->load();
                const double cents = audioProcessor.getFineCentsOffset();
                const std::string guide = audioProcessor.getCurrentTuningGuide();

                int newIdx = 0;
                const bool success = audioProcessor.getPresetManager().saveUserPreset(
                    name, st, hz, cents, guide, "Custom user tuning preset", &newIdx);

                if (success)
                {
                    populateCategories();
                    for (int i = 0; i < categoryBox.getNumItems(); ++i)
                    {
                        if (categoryBox.getItemText(i) == "User Presets")
                        {
                            categoryBox.setSelectedId(i + 1, juce::sendNotification);
                            break;
                        }
                    }
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Name Conflict",
                        "Cannot overwrite a factory preset. Choose a custom name.",
                        "OK");
                }
            }
        }
        delete aw;
    }));
}

void TuningsAudioProcessorEditor::openMidiContextMenu(const std::string& paramId, const std::string& dispName)
{
    juce::PopupMenu menu;
    menu.addItem(1, "Parameter: " + dispName, false);
    menu.addSeparator();
    menu.addItem(2, "Reset parameter");
    menu.addItem(3, "Enable MIDI Learn");
    menu.addItem(4, "Open MIDI Mappings...");

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, paramId, dispName](int result) {
        if (result == 2)
        {
            if (auto* p = audioProcessor.getAPVTS().getParameter(paramId))
                p->setValueNotifyingHost(p->getDefaultValue());
        }
        else if (result == 3)
        {
            audioProcessor.getMidiManager().setLearningParameter(paramId, dispName);
        }
        else if (result == 4)
        {
            buttonClicked(&midiMenuButton);
        }
    });
}

void TuningsAudioProcessorEditor::timerCallback()
{
    const bool tunerEngaged = audioProcessor.getAPVTS().getRawParameterValue("tunerEngaged")->load() > 0.5f;
    strobeTuner.setEngaged(tunerEngaged);

    // Pass latest pitch tracker result to strobe component
    auto tunerRes = audioProcessor.getLatestTunerResult();
    strobeTuner.updateResult(tunerRes);

    const double refHz = audioProcessor.getAPVTS().getRawParameterValue("referenceHz")->load();
    strobeTuner.setReferenceA4(refHz);
    strobeTuner.setPostPitchShift(tunerSourceToggle.getToggleState());
    strobeTuner.setSpeedFast(tunerSpeedToggle.getToggleState());
    strobeTuner.setUseFlats(tunerDisplayToggle.getToggleState());

    // Expression pedal state
    const float pPos = audioProcessor.getAPVTS().getRawParameterValue("pedalPos")->load();
    const int bPitch = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("pedalBottom")->load());
    const int tPitch = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("pedalTop")->load());
    const bool oct2  = audioProcessor.getAPVTS().getRawParameterValue("pedalOctave2")->load() > 0.5f;
    const bool bypass = audioProcessor.getAPVTS().getRawParameterValue("pedalBypass")->load() > 0.5f;

    expressionPedal.setPedalPosition(pPos);
    expressionPedal.setPitchRange(bPitch, tPitch);
    expressionPedal.setDoubleOctave(oct2);
    expressionPedal.setBypassed(bypass);

    if (audioProcessor.getMidiManager().getIsLearning())
    {
        repaint();
    }
}

void TuningsAudioProcessorEditor::loadBackgroundImage()
{
    // 1. Embedded binary asset (always available anywhere)
    if constexpr (BinaryData::DROP_THE_HAMMER_BG_jpgSize > 0)
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(BinaryData::DROP_THE_HAMMER_BG_jpg,
                                                          static_cast<size_t>(BinaryData::DROP_THE_HAMMER_BG_jpgSize));
        if (backgroundImage.isValid())
            return;
    }

    // 2. Direct path fallback
    juce::File primaryFile(R"(C:\Coding\Tunings VST3\DROP THE HAMMER BACKGROUND\DROP THE HAMMER BG.jpg)");
    if (primaryFile.existsAsFile())
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(primaryFile);
        if (backgroundImage.isValid())
            return;
    }

    juce::File secondaryFile(R"(C:\Coding\Tunings VST3\Images\DROP_THE_HAMMER_BG.jpg)");
    if (secondaryFile.existsAsFile())
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(secondaryFile);
    }
}

void TuningsAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 1. Full-bleed DDTH Background Image across the whole plugin
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit | juce::RectanglePlacement::fillDestination);
    }
    else
    {
        juce::ColourGradient studioFloor(juce::Colour(0xff14161a), 0, 0,
                                         juce::Colour(0xff090a0d), 0, static_cast<float>(getHeight()), false);
        g.setGradientFill(studioFloor);
        g.fillAll();
    }

    // 2. Sleek translucent dark acrylic glass strip at the very top for presets & master knobs
    const int barH = 48;
    g.setColour(juce::Colour(0xd0080a0e));
    g.fillRect(0, 0, getWidth(), barH);
    g.setColour(juce::Colour(0x88800a14)); // Blood Red subtle divider
    g.drawHorizontalLine(barH, 0.0f, static_cast<float>(getWidth()));

    // Plugin Title on Top Left
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffff1e2e)); // Blood Red
    g.drawText("DONT DROP THE HAMMER", 16, 0, 200, barH, juce::Justification::centredLeft);

    // Outer Window Subtle Border
    g.setColour(juce::Colour(0x55252933));
    g.drawRect(getLocalBounds(), 1);

    // MIDI Learn Indicator Banner
    if (audioProcessor.getMidiManager().getIsLearning())
    {
        g.setColour(juce::Colour(0xee800a14)); // Dark Red
        g.fillRect(0, 0, getWidth(), 26);
        g.setColour(juce::Colour(0xff00f0ff)); // Neon Blue outline
        g.drawHorizontalLine(26, 0.0f, static_cast<float>(getWidth()));
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("MIDI LEARN ACTIVE: Move any MIDI Controller / CC to assign to: "
                   + audioProcessor.getMidiManager().getLearningParamId(),
                   0, 0, getWidth(), 26, juce::Justification::centred);
    }
}

void TuningsAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    // Top Navigation & Preset Bar (y = 0..48)
    prevPresetButton.setBounds(216, 10, 26, 28);
    categoryBox.setBounds(246, 10, 130, 28);
    presetBox.setBounds(382, 10, 175, 28);
    nextPresetButton.setBounds(562, 10, 26, 28);
    savePresetButton.setBounds(594, 10, 42, 28);
    deletePresetButton.setBounds(640, 10, 36, 28);

    // Top Right Master Dials (Labels above knobs)
    refFreqLabel.setBounds(696, 2, 58, 14);
    refFreqSlider.setBounds(696, 14, 58, 32);

    inputGainLabel.setBounds(762, 2, 52, 14);
    inputGainSlider.setBounds(762, 14, 52, 32);

    mixLabel.setBounds(822, 2, 52, 14);
    mixSlider.setBounds(822, 14, 52, 32);

    outputGainLabel.setBounds(882, 2, 52, 14);
    outputGainSlider.setBounds(882, 14, 52, 32);

    midiMenuButton.setBounds(948, 10, 80, 28);

    // Backlit Physical Tuning Guide Strip (y = 54, h = 26)
    tuningGuideBanner.setBounds(30, 54, w - 60, 26);

    // --- MAIN STAGE (y = 86) ---
    const int stageY = 86;
    const int stageH = h - stageY - 14;

    // LEFT STAGE: HAMMER WHAMMY EXPRESSION PEDAL (Viking Coffin Lid)
    const int leftX = 30;
    const int leftW = 540;

    expressionPedal.setBounds(leftX, stageY, leftW, stageH);

    // Bottom controls on pedal shelf (Label ABOVE knob)
    const int leftShelfY = stageY + stageH - 62;
    pedalBottomLabel.setBounds(leftX + 24, leftShelfY, 56, 14);
    pedalBottomSlider.setBounds(leftX + 24, leftShelfY + 14, 56, 44);

    pedalTopLabel.setBounds(leftX + 96, leftShelfY, 56, 14);
    pedalTopSlider.setBounds(leftX + 96, leftShelfY + 14, 56, 44);

    doubleOctaveToggle.setBounds(leftX + 172, leftShelfY + 20, 130, 26);

    pedalBottomLabel.toFront(false);
    pedalBottomSlider.toFront(false);
    pedalTopLabel.toFront(false);
    pedalTopSlider.toFront(false);
    doubleOctaveToggle.toFront(false);

    // RIGHT STAGE: THE LOCKON STROBE & TRANSPOSER (Floats directly over background)
    const int rightX = leftX + leftW + 40;
    const int rightW = w - rightX - 30;

    // Section 1: Transposition Engine Header & LCD Readout
    transposerHeaderLabel.setBounds(rightX, stageY + 4, rightW, 20);
    transposerDisplayBox.setBounds(rightX, stageY + 26, rightW, 36);
    semitoneShiftBox.setBounds(rightX, stageY + 68, rightW, 28);

    // Section 2: Strobe Tuner Radar
    const int tunerY = stageY + 104;
    const int tunerH = stageH - 146;
    strobeTuner.setBounds(rightX, tunerY, rightW, tunerH);

    // Bottom Hardware Toggle Switches on Lockon Unit
    const int togglesY = stageY + stageH - 34;
    const int toggleW = (rightW - 30) / 4;
    tunerEngageToggle.setBounds(rightX, togglesY, toggleW + 15, 24);
    tunerSourceToggle.setBounds(rightX + toggleW + 15, togglesY, toggleW + 25, 24);
    tunerSpeedToggle.setBounds(rightX + (toggleW + 15) * 2 + 10, togglesY, toggleW, 24);
    tunerDisplayToggle.setBounds(rightX + (toggleW + 15) * 3 + 10, togglesY, toggleW, 24);

    // Modal if visible
    if (midiModal != nullptr)
    {
        midiModal->setBounds(getLocalBounds().reduced(60, 40));
    }
}
