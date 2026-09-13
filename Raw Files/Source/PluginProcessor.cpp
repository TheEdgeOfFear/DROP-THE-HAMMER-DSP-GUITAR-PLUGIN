#include "PluginProcessor.h"
#include "PluginEditor.h"

TuningsAudioProcessor::TuningsAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    tunerFifoBuffer.resize(tunerFifoSize, 0.0f);
    tunerAnalysisScratch.resize(2048, 0.0f);
}

TuningsAudioProcessor::~TuningsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout TuningsAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"pitchShift", 1},
        "Pitch Shift Semitones",
        -12, 12, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"referenceHz", 1},
        "Reference Frequency Hz",
        juce::NormalisableRange<float>(420.0f, 460.0f, 0.1f),
        440.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"tunerSource", 1},
        "Tuner Source Post-Shift",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"tunerSpeed", 1},
        "Tuner Fast Speed",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"tunerDisplay", 1},
        "Tuner Flats Display",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pedalPos", 1},
        "Expression Pedal Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"pedalBottom", 1},
        "Pedal Bottom Pitch",
        -24, 24, -12));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"pedalTop", 1},
        "Pedal Top Pitch",
        -24, 24, 12));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"pedalOctave2", 1},
        "Pedal Double Octave Mode",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"pedalBypass", 1},
        "Pedal Bypass",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGain", 1},
        "Input Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1},
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"tunerEngaged", 1},
        "Tuner Engaged",
        true));

    return { params.begin(), params.end() };
}

const juce::String TuningsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TuningsAudioProcessor::acceptsMidi() const
{
    return true;
}

bool TuningsAudioProcessor::producesMidi() const
{
    return false;
}

bool TuningsAudioProcessor::isMidiEffect() const
{
    return false;
}

double TuningsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TuningsAudioProcessor::getNumPrograms()
{
    return static_cast<int>(presetManager.getAllPresets().size());
}

int TuningsAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex.load();
}

void TuningsAudioProcessor::setCurrentProgram(int index)
{
    applyPreset(index);
}

const juce::String TuningsAudioProcessor::getProgramName(int index)
{
    const auto& list = presetManager.getAllPresets();
    if (index >= 0 && index < static_cast<int>(list.size()))
        return list[static_cast<size_t>(index)].name;
    return {};
}

void TuningsAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void TuningsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    pitchShifter.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    pitchDetector.prepare(sampleRate);
    tunerFifo.reset();
    setLatencySamples(pitchShifter.getLatencySamples());
}

void TuningsAudioProcessor::releaseResources()
{
    pitchShifter.reset();
}

bool TuningsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TuningsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    // Process MIDI & Mappings
    midiManager.processMidi(midiMessages, parameters, [this](int presetIdx) {
        applyPreset(presetIdx);
    });

    // Parameters
    const float inGain = parameters.getRawParameterValue("inputGain")->load();
    const float outGain = parameters.getRawParameterValue("outputGain")->load();
    const float mixVal = parameters.getRawParameterValue("mix")->load();
    const int baseSemitones = static_cast<int>(parameters.getRawParameterValue("pitchShift")->load());
    const float refHz = parameters.getRawParameterValue("referenceHz")->load();
    const bool isPostTuner = parameters.getRawParameterValue("tunerSource")->load() > 0.5f;

    const float pedalPos = parameters.getRawParameterValue("pedalPos")->load();
    const int pedalBottom = static_cast<int>(parameters.getRawParameterValue("pedalBottom")->load());
    const int pedalTop = static_cast<int>(parameters.getRawParameterValue("pedalTop")->load());
    const bool pedalOct2 = parameters.getRawParameterValue("pedalOctave2")->load() > 0.5f;
    const bool pedalBypass = parameters.getRawParameterValue("pedalBypass")->load() > 0.5f;

    // Apply Input Gain
    buffer.applyGain(inGain);

    // If Tuner Source is PRE, feed tuner with raw guitar signal
    if (!isPostTuner && totalNumInputChannels > 0)
    {
        const float* inRead = buffer.getReadPointer(0);
        int start1, size1, start2, size2;
        tunerFifo.prepareToWrite(numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            std::copy(inRead, inRead + size1, tunerFifoBuffer.begin() + start1);
        if (size2 > 0)
            std::copy(inRead + size1, inRead + size1 + size2, tunerFifoBuffer.begin() + start2);
        tunerFifo.finishedWrite(size1 + size2);
    }

    // Calculate effective pitch shift
    // 1. Base semitones
    double totalSemitones = static_cast<double>(baseSemitones);

    // 2. Reference Hz deviation from 440 Hz
    if (std::abs(refHz - 440.0f) > 0.01f)
    {
        totalSemitones += 12.0 * std::log2(static_cast<double>(refHz) / 440.0);
    }

    // 3. Fine cents
    totalSemitones += currentFineCents.load() / 100.0;

    // 4. Expression Pedal
    if (!pedalBypass)
    {
        const double mult = pedalOct2 ? 2.0 : 1.0;
        const double pRange = static_cast<double>(pedalTop - pedalBottom) * mult;
        const double pShift = (static_cast<double>(pedalBottom) * mult) + (static_cast<double>(pedalPos) * pRange);
        totalSemitones += pShift;
    }

    // Process Pitch Shifter with Dry/Wet Mix (0-100%)
    pitchShifter.process(buffer, totalSemitones, mixVal);

    // If Tuner Source is POST, feed tuner with pitch shifted signal
    if (isPostTuner && totalNumOutputChannels > 0)
    {
        const float* outRead = buffer.getReadPointer(0);
        int start1, size1, start2, size2;
        tunerFifo.prepareToWrite(numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            std::copy(outRead, outRead + size1, tunerFifoBuffer.begin() + start1);
        if (size2 > 0)
            std::copy(outRead + size1, outRead + size1 + size2, tunerFifoBuffer.begin() + start2);
        tunerFifo.finishedWrite(size1 + size2);
    }

    // Apply Output Gain
    buffer.applyGain(outGain);

    for (int ch = 0; ch < totalNumOutputChannels; ++ch)
    {
        float* chData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            // Transparent safety ceiling: never color signal in [-1.0, 1.0] range
            float s = chData[i];
            if (s > 1.0f) s = 1.0f;
            else if (s < -1.0f) s = -1.0f;
            chData[i] = s;
        }
    }
}

TunerDetectionResult TuningsAudioProcessor::getLatestTunerResult()
{
    const bool isEngaged = parameters.getRawParameterValue("tunerEngaged")->load() > 0.5f;
    if (!isEngaged)
    {
        TunerDetectionResult res;
        res.hasSignal = false;
        res.noteName = "OFF";
        return res;
    }

    const int available = tunerFifo.getNumReady();
    if (available > 0)
    {
        int start1, size1, start2, size2;
        tunerFifo.prepareToRead(available, start1, size1, start2, size2);

        if (size1 > 0)
            pitchDetector.pushSamples(tunerFifoBuffer.data() + start1, size1);
        if (size2 > 0)
            pitchDetector.pushSamples(tunerFifoBuffer.data() + start2, size2);

        tunerFifo.finishedRead(size1 + size2);
    }

    const float refHz = parameters.getRawParameterValue("referenceHz")->load();
    const bool useFlats = parameters.getRawParameterValue("tunerDisplay")->load() > 0.5f;

    return pitchDetector.analyze(refHz, useFlats);
}

void TuningsAudioProcessor::applyPreset(int globalIndex)
{
    const auto& list = presetManager.getAllPresets();
    if (globalIndex < 0 || globalIndex >= static_cast<int>(list.size()))
        return;

    currentPresetIndex.store(globalIndex);
    const auto& p = list[static_cast<size_t>(globalIndex)];

    currentTuningGuide = p.physicalTuningGuide;
    currentFineCents.store(p.fineCentsOffset);

    if (auto* param = parameters.getParameter("pitchShift"))
        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.pitchShiftSemitones)));

    if (auto* param = parameters.getParameter("referenceHz"))
        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.referenceA4Hz)));
}

bool TuningsAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TuningsAudioProcessor::createEditor()
{
    return new TuningsAudioProcessorEditor(*this);
}

void TuningsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty("selectedPresetIndex", currentPresetIndex.load(), nullptr);
    state.setProperty("fineCents", currentFineCents.load(), nullptr);
    state.addChild(midiManager.exportToValueTree(), -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TuningsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        auto vt = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(vt);
        const int pIdx = vt.getProperty("selectedPresetIndex", 0);
        currentPresetIndex.store(pIdx);
        currentFineCents.store(vt.getProperty("fineCents", 0.0));

        const auto& allPresets = presetManager.getAllPresets();
        if (pIdx >= 0 && pIdx < static_cast<int>(allPresets.size()))
        {
            currentTuningGuide = allPresets[static_cast<size_t>(pIdx)].physicalTuningGuide;
        }

        auto midiTree = vt.getChildWithName("MidiMappings");
        if (midiTree.isValid())
            midiManager.importFromValueTree(midiTree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TuningsAudioProcessor();
}
