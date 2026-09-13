#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "TuningModel.h"
#include "PitchShifterEngine.h"
#include "PitchDetector.h"
#include "MidiManager.h"
#include "PresetManager.h"

class TuningsAudioProcessor : public juce::AudioProcessor
{
public:
    TuningsAudioProcessor();
    ~TuningsAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    MidiManager& getMidiManager() { return midiManager; }
    PresetManager& getPresetManager() { return presetManager; }

    TunerDetectionResult getLatestTunerResult();

    void applyPreset(int globalIndex);
    int getSelectedPresetIndex() const { return currentPresetIndex.load(); }

    void setFineCentsOffset(double cents) { currentFineCents.store(cents); }
    double getFineCentsOffset() const { return currentFineCents.load(); }

    std::string getCurrentTuningGuide() const { return currentTuningGuide; }

private:
    juce::AudioProcessorValueTreeState parameters;
    MidiManager midiManager;
    PresetManager presetManager;

    PitchShifterEngine pitchShifter;
    PitchDetector pitchDetector;

    std::atomic<int> currentPresetIndex{0};
    std::atomic<double> currentFineCents{0.0};
    std::string currentTuningGuide{"PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)"};

    // Strobe tuner audio buffer transfer
    static constexpr int tunerFifoSize = 4096;
    juce::AbstractFifo tunerFifo{tunerFifoSize};
    std::vector<float> tunerFifoBuffer;
    std::vector<float> tunerAnalysisScratch;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TuningsAudioProcessor)
};
