#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <juce_core/juce_core.h>

struct TunerDetectionResult
{
    bool hasSignal{false};
    double frequencyHz{0.0};
    std::string noteName{"--"};
    int octave{0};
    double centsOffset{0.0}; // -50.0 to +50.0
    bool isInTune{false};    // within +/- 1.5 cents
};

class PitchDetector
{
public:
    PitchDetector()
    {
        buffer.resize(bufferCapacity, 0.0f);
    }

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
        writeIndex = 0;
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }

    void pushSamples(const float* channelData, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[writeIndex] = channelData[i];
            writeIndex = (writeIndex + 1) % bufferCapacity;
        }
    }

    TunerDetectionResult analyze(double referenceA4Hz, bool useFlats)
    {
        TunerDetectionResult result;

        // Extract linear analysis frame of 4096 samples (supports down to 30Hz cleanly)
        constexpr int frameSize = 4096;
        constexpr int halfFrame = frameSize / 2; // 2048

        std::vector<float> frame(frameSize);
        int readIdx = (writeIndex - frameSize + bufferCapacity) % bufferCapacity;
        float sumSquares = 0.0f;

        for (int i = 0; i < frameSize; ++i)
        {
            frame[i] = buffer[readIdx];
            sumSquares += frame[i] * frame[i];
            readIdx = (readIdx + 1) % bufferCapacity;
        }

        const float rms = std::sqrt(sumSquares / static_cast<float>(frameSize));
        if (rms < 0.002f) // Sensitive threshold to track sustained notes
        {
            result.hasSignal = false;
            return result;
        }

        result.hasSignal = true;

        // Frequency search limits (30 Hz to 1100 Hz)
        const int minPeriod = std::max(20, static_cast<int>(currentSampleRate / 1100.0));
        const int maxPeriod = std::min(halfFrame - 2, static_cast<int>(currentSampleRate / 30.0));

        // Coarse YIN search with 2:1 decimation for ultra-fast real-time response (~0.3ms)
        const int step = 2;
        const int dsMinTau = std::max(1, minPeriod / step);
        const int dsMaxTau = maxPeriod / step;

        std::vector<float> diff(dsMaxTau + 1, 0.0f);

        for (int tau = dsMinTau; tau <= dsMaxTau; ++tau)
        {
            float d = 0.0f;
            const int tauOffset = tau * step;
            for (int j = 0; j < halfFrame; j += step)
            {
                const float delta = frame[j] - frame[j + tauOffset];
                d += delta * delta;
            }
            diff[tau] = d;
        }

        // Cumulative mean normalized difference
        std::vector<float> cmnd(dsMaxTau + 1, 0.0f);
        cmnd[0] = 1.0f;
        float runningSum = 0.0f;

        for (int tau = 1; tau <= dsMaxTau; ++tau)
        {
            runningSum += diff[tau];
            cmnd[tau] = runningSum > 0.00001f ? (diff[tau] * static_cast<float>(tau) / runningSum) : 1.0f;
        }

        // Absolute threshold search (standard YIN threshold 0.18)
        const float threshold = 0.18f;
        int coarseTau = -1;

        for (int tau = dsMinTau; tau <= dsMaxTau; ++tau)
        {
            if (cmnd[tau] < threshold)
            {
                while (tau + 1 <= dsMaxTau && cmnd[tau + 1] < cmnd[tau])
                    tau++;
                coarseTau = tau;
                break;
            }
        }

        // Fallback to global minimum
        if (coarseTau == -1)
        {
            float minVal = 1000.0f;
            for (int tau = dsMinTau; tau <= dsMaxTau; ++tau)
            {
                if (cmnd[tau] < minVal)
                {
                    minVal = cmnd[tau];
                    coarseTau = tau;
                }
            }
            if (minVal > 0.50f)
            {
                result.hasSignal = false;
                return result;
            }
        }

        // Fine search at full resolution around coarseTau * step
        const int centerTau = coarseTau * step;
        const int fineMin = std::max(minPeriod, centerTau - step * 2);
        const int fineMax = std::min(maxPeriod, centerTau + step * 2);

        int bestTau = centerTau;
        float bestDiff = 1e12f;

        for (int tau = fineMin; tau <= fineMax; ++tau)
        {
            float d = 0.0f;
            for (int j = 0; j < halfFrame; ++j)
            {
                const float delta = frame[j] - frame[j + tau];
                d += delta * delta;
            }
            if (d < bestDiff)
            {
                bestDiff = d;
                bestTau = tau;
            }
        }

        // Parabolic interpolation for sub-cent period accuracy
        double period = bestTau;
        if (bestTau > minPeriod && bestTau < maxPeriod)
        {
            float dPrev = 0.0f, dNext = 0.0f;
            for (int j = 0; j < halfFrame; ++j)
            {
                const float del1 = frame[j] - frame[j + bestTau - 1];
                dPrev += del1 * del1;
                const float del2 = frame[j] - frame[j + bestTau + 1];
                dNext += del2 * del2;
            }
            const double s0 = dPrev;
            const double s1 = bestDiff;
            const double s2 = dNext;
            const double denom = 2.0 * (2.0 * s1 - s0 - s2);
            if (std::abs(denom) > 1e-6)
            {
                const double delta = (s0 - s2) / denom;
                period = static_cast<double>(bestTau) + delta;
            }
        }

        if (period <= 0.0)
        {
            result.hasSignal = false;
            return result;
        }

        result.frequencyHz = currentSampleRate / period;

        // Calculate MIDI note and cents relative to referenceA4Hz
        const double semitonesAboveA4 = 12.0 * std::log2(result.frequencyHz / referenceA4Hz);
        const double exactMidi = 69.0 + semitonesAboveA4;
        const int roundedMidi = static_cast<int>(std::round(exactMidi));

        result.centsOffset = (exactMidi - static_cast<double>(roundedMidi)) * 100.0;
        result.isInTune = std::abs(result.centsOffset) <= 1.5;

        // Note names
        static const char* sharpNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        static const char* flatNames[]  = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

        const int noteIndex = ((roundedMidi % 12) + 12) % 12;
        result.octave = (roundedMidi / 12) - 1;
        result.noteName = useFlats ? flatNames[noteIndex] : sharpNames[noteIndex];

        return result;
    }

private:
    double currentSampleRate{44100.0};
    static constexpr int bufferCapacity = 8192;
    std::vector<float> buffer;
    int writeIndex{0};
};
