#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <juce_audio_basics/juce_audio_basics.h>

class PitchShifterEngine
{
public:
    PitchShifterEngine() = default;

    void prepare(double sampleRate, int maxBlockSize, int numChannels)
    {
        juce::ignoreUnused(maxBlockSize);
        currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
        channelCount = numChannels > 0 ? numChannels : 2;

        // Window size ~55ms: optimal balance between ultra-low latency and smooth polyphonic tracking for drop-tunings
        windowSizeSamples = static_cast<int>(currentSampleRate * 0.055);
        if (windowSizeSamples < 1024) windowSizeSamples = 1024;
        if (windowSizeSamples > 8192) windowSizeSamples = 8192;

        delayBuffer.setSize(channelCount, bufferSize);
        delayBuffer.clear();

        writePos = 0;
        head1 = 0.0;
        head2 = static_cast<double>(windowSizeSamples) * 0.5;
        smoothedShiftRatio = 1.0;
    }

    void reset()
    {
        delayBuffer.clear();
        writePos = 0;
        head1 = 0.0;
        head2 = static_cast<double>(windowSizeSamples) * 0.5;
        smoothedShiftRatio = 1.0;
    }

    int getLatencySamples() const noexcept
    {
        return static_cast<int>(baseDelaySamples);
    }

    // Process block with target semitones (fractional) and dry/wet mix (0.0 = 100% dry, 1.0 = 100% wet)
    void process(juce::AudioBuffer<float>& buffer, double targetSemitones, float mix = 1.0f)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0) return;

        const int numChannels = std::min(buffer.getNumChannels(), channelCount);
        const double targetRatio = std::pow(2.0, targetSemitones / 12.0);

        const float* const* channelRead = buffer.getArrayOfReadPointers();
        float* const* channelWrite = buffer.getArrayOfWritePointers();

        const double halfWindow = static_cast<double>(windowSizeSamples) * 0.5;
        const double invWindow = 1.0 / static_cast<double>(windowSizeSamples);

        const float wetGain = std::clamp(mix, 0.0f, 1.0f);
        const float dryGain = 1.0f - wetGain;

        for (int i = 0; i < numSamples; ++i)
        {
            // Smooth ratio parameter changes to prevent zipper noise on fast pedal sweeps (response ~15ms)
            smoothedShiftRatio += (targetRatio - smoothedShiftRatio) * 0.003;
            const double rate = 1.0 - smoothedShiftRatio;

            // Write current input samples to circular buffer
            for (int ch = 0; ch < numChannels; ++ch)
            {
                delayBuffer.setSample(ch, writePos, channelRead[ch][i]);
            }

            // Advance primary read head
            head1 += rate;
            while (head1 >= windowSizeSamples) head1 -= windowSizeSamples;
            while (head1 < 0.0)                head1 += windowSizeSamples;

            // Secondary head is strictly 180 degrees out-of-phase
            head2 = head1 + halfWindow;
            if (head2 >= windowSizeSamples) head2 -= windowSizeSamples;

            // Equal-power Hann crossfade window weights (w1 + w2 = 1.0 identically everywhere)
            const double w1 = 0.5 * (1.0 - std::cos(juce::MathConstants<double>::twoPi * head1 * invWindow));
            const double w2 = 1.0 - w1;

            // Compute delay positions with guaranteed safety baseDelay offset (always in the past)
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const double readPos1 = static_cast<double>(writePos) - baseDelaySamples - head1;
                const double readPos2 = static_cast<double>(writePos) - baseDelaySamples - head2;

                const float s1 = readInterpolated(ch, readPos1);
                const float s2 = readInterpolated(ch, readPos2);
                const float wetSignal = static_cast<float>(s1 * w1 + s2 * w2);

                if (dryGain > 0.0001f)
                {
                    // Delay-compensated dry signal (exact phase alignment with baseDelay)
                    const double dryPos = static_cast<double>(writePos) - baseDelaySamples;
                    const float drySignal = readInterpolated(ch, dryPos);
                    channelWrite[ch][i] = drySignal * dryGain + wetSignal * wetGain;
                }
                else
                {
                    // 100% pure wet pitch-shifted signal
                    channelWrite[ch][i] = wetSignal;
                }
            }

            writePos = (writePos + 1) & bufferMask;
        }
    }

private:
    double currentSampleRate{44100.0};
    int channelCount{2};
    int windowSizeSamples{2420};
    static constexpr double baseDelaySamples{256.0};

    static constexpr int bufferSize{32768};
    static constexpr int bufferMask{32767};

    juce::AudioBuffer<float> delayBuffer;
    int writePos{0};
    double head1{0.0};
    double head2{0.0};
    double smoothedShiftRatio{1.0};

    // Sub-sample 4-point cubic Hermite interpolation with mask wrapping
    inline float readInterpolated(int channel, double pos) const noexcept
    {
        double wrappedPos = std::fmod(pos, static_cast<double>(bufferSize));
        if (wrappedPos < 0.0)
            wrappedPos += bufferSize;

        const int i0 = static_cast<int>(wrappedPos);
        const float frac = static_cast<float>(wrappedPos - static_cast<double>(i0));

        const int im1 = (i0 - 1) & bufferMask;
        const int ip1 = (i0 + 1) & bufferMask;
        const int ip2 = (i0 + 2) & bufferMask;

        const float* buf = delayBuffer.getReadPointer(channel);
        const float y0 = buf[im1];
        const float y1 = buf[i0 & bufferMask];
        const float y2 = buf[ip1];
        const float y3 = buf[ip2];

        // Cubic Hermite
        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }
};
