#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace Tunings
{

struct TuningString
{
    std::string noteName;
    int midiNoteNumber;
    double centsOffset;
    double frequencyHz;
};

struct TuningPreset
{
    std::string category;              // e.g. "Pantera", "Machine Head", "Metalcore", "Standard Shifts", "User"
    std::string name;                  // e.g. "D Standard (425 Hz)"
    std::string description;           // e.g. "Vulgar Display of Power tone"
    std::string physicalTuningGuide;   // e.g. "TUNE GUITAR TO: E-A-D-G-B-E (Standard)"
    int pitchShiftSemitones;           // Semitone shift applied by plugin (e.g. -2)
    double referenceA4Hz;              // Reference frequency (e.g. 425.0, 435.0, 440.0)
    double fineCentsOffset;            // Fine cents detune (e.g. +40.0 cents for Robb Flynn)
    bool isFactory;                    // If true, cannot be overwritten or deleted
    std::vector<TuningString> strings; // Target string pitches
};

inline double calculateFrequency(int midiNote, double referenceA4, double centsOffset = 0.0)
{
    const double semitones = static_cast<double>(midiNote - 69) + (centsOffset / 100.0);
    return referenceA4 * std::pow(2.0, semitones / 12.0);
}

inline TuningString makeString(const std::string& noteName, int midiNote, double referenceA4, double centsOffset = 0.0)
{
    return TuningString{
        noteName,
        midiNote,
        centsOffset,
        calculateFrequency(midiNote, referenceA4, centsOffset)
    };
}

inline std::string getSemitoneDescription(int semitones)
{
    switch (semitones)
    {
        case -12: return "-12 (Octave Down / 8ve)";
        case -11: return "-11 (F Standard / Major 7th Down)";
        case -10: return "-10 (F# Standard / Minor 7th Down)";
        case -9:  return "-9 (G Standard / Major 6th Down)";
        case -8:  return "-8 (G# / Ab Standard / Minor 6th Down)";
        case -7:  return "-7 (A Standard / Perfect 5th Down)";
        case -6:  return "-6 (A# / Bb Standard / Tritone Down)";
        case -5:  return "-5 (B Standard / Perfect 4th Down)";
        case -4:  return "-4 (C Standard / Major 3rd Down)";
        case -3:  return "-3 (C# Standard / Minor 3rd Down)";
        case -2:  return "-2 (D Standard / Whole Step Down)";
        case -1:  return "-1 (Eb / D# Standard / Half Step Down)";
        case 0:   return "0 (Standard Tuning - E)";
        case 1:   return "+1 (F Standard / Half Step Up)";
        case 2:   return "+2 (F# Standard / Whole Step Up)";
        case 3:   return "+3 (G Standard / Minor 3rd Up)";
        case 4:   return "+4 (G# Standard / Major 3rd Up)";
        case 5:   return "+5 (A Standard / Perfect 4th Up)";
        case 6:   return "+6 (A# Standard / Tritone Up)";
        case 7:   return "+7 (B Standard / Perfect 5th Up)";
        case 8:   return "+8 (C Standard / Minor 6th Up)";
        case 9:   return "+9 (C# Standard / Major 6th Up)";
        case 10:  return "+10 (D Standard / Minor 7th Up)";
        case 11:  return "+11 (D# Standard / Major 7th Up)";
        case 12:  return "+12 (Octave Up / 8ve)";
        default:  return (semitones > 0 ? "+" : "") + std::to_string(semitones);
    }
}

inline std::vector<TuningPreset> getBuiltInPresets()
{
    std::vector<TuningPreset> presets;

    // --- STANDARD PITCH SHIFTS (-12 to +12) ---
    for (int st = -12; st <= 12; ++st)
    {
        presets.push_back({
            "Standard Shifts",
            getSemitoneDescription(st),
            "Pitch shifts standard guitar tuning by " + std::to_string(st) + " semitones.",
            "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
            st,
            440.0,
            0.0,
            true,
            {
                makeString("E2", 40 + st, 440.0),
                makeString("A2", 45 + st, 440.0),
                makeString("D3", 50 + st, 440.0),
                makeString("G3", 55 + st, 440.0),
                makeString("B3", 59 + st, 440.0),
                makeString("E4", 64 + st, 440.0)
            }
        });
    }

    // --- PANTERA ---
    presets.push_back({
        "Pantera",
        "E Standard (435 Hz)",
        "Used on tracks like Cowboys from Hell. Dimebag tuned ~435 Hz.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        0,
        435.0,
        0.0,
        true,
        {
            makeString("E2", 40, 435.0),
            makeString("A2", 45, 435.0),
            makeString("D3", 50, 435.0),
            makeString("G3", 55, 435.0),
            makeString("B3", 59, 435.0),
            makeString("E4", 64, 435.0)
        }
    });

    presets.push_back({
        "Pantera",
        "E Standard (425 Hz)",
        "Dimebag quarter-step down signature tuning. A = 425 Hz.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        0,
        425.0,
        0.0,
        true,
        {
            makeString("E2", 40, 425.0),
            makeString("A2", 45, 425.0),
            makeString("D3", 50, 425.0),
            makeString("G3", 55, 425.0),
            makeString("B3", 59, 425.0),
            makeString("E4", 64, 425.0)
        }
    });

    presets.push_back({
        "Pantera",
        "D Standard (425 Hz)",
        "Vulgar Display of Power / Far Beyond Driven crushing tone.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -2,
        425.0,
        0.0,
        true,
        {
            makeString("D2", 38, 425.0),
            makeString("G2", 43, 425.0),
            makeString("C3", 48, 425.0),
            makeString("F3", 53, 425.0),
            makeString("A3", 57, 425.0),
            makeString("D4", 62, 425.0)
        }
    });

    presets.push_back({
        "Pantera",
        "D# / Eb Standard",
        "Used on various 90s Pantera tracks.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -1,
        440.0,
        0.0,
        true,
        {
            makeString("D#2", 39, 440.0),
            makeString("G#2", 44, 440.0),
            makeString("C#3", 49, 440.0),
            makeString("F#3", 54, 440.0),
            makeString("A#3", 58, 440.0),
            makeString("D#4", 63, 440.0)
        }
    });

    presets.push_back({
        "Pantera",
        "C# Standard",
        "Heavy downtuned Pantera tracks.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -3,
        440.0,
        0.0,
        true,
        {
            makeString("C#2", 37, 440.0),
            makeString("F#2", 42, 440.0),
            makeString("B2", 47, 440.0),
            makeString("E3", 52, 440.0),
            makeString("G#3", 56, 440.0),
            makeString("C#4", 61, 440.0)
        }
    });

    presets.push_back({
        "Pantera",
        "Drop D",
        "Standard Drop D tuning.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        0,
        440.0,
        0.0,
        true,
        {
            makeString("D2", 38, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("G3", 55, 440.0),
            makeString("B3", 59, 440.0),
            makeString("E4", 64, 440.0)
        }
    });

    presets.push_back({
        "Pantera",
        "Drop C",
        "Drop D physical tuning shifted 2 semitones down.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -2,
        440.0,
        0.0,
        true,
        {
            makeString("C2", 36, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    presets.push_back({
        "Pantera",
        "Drop G (Dimebag Trendkill)",
        "The Great Southern Trendkill: D standard with 6th string dropped to low G.",
        "PHYSICAL GUITAR SETUP: TUNE TO G-G-C-F-A-D (Trendkill Setup)",
        0,
        440.0,
        0.0,
        true,
        {
            makeString("G1", 31, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    // --- MACHINE HEAD ---
    // Robb Flynn tunes roughly 40 cents sharp: 440 * 2^(40/1200) = 450.3 Hz
    const double machineHeadA4 = 450.3;

    presets.push_back({
        "Machine Head",
        "Drop B (+40 Cents Sharp)",
        "Signature Robb Flynn tone on The Blackening, Locust. Tuned +40 cents sharp (A = 450.3 Hz).",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -3,
        machineHeadA4,
        0.0,
        true,
        {
            makeString("B1", 35, machineHeadA4),
            makeString("F#2", 42, machineHeadA4),
            makeString("B2", 47, machineHeadA4),
            makeString("E3", 52, machineHeadA4),
            makeString("G#3", 56, machineHeadA4),
            makeString("C#4", 61, machineHeadA4)
        }
    });

    presets.push_back({
        "Machine Head",
        "C# Standard (+40 Cents Sharp)",
        "Aesthetics of Hate and classic tracks tuned +40 cents sharp (A = 450.3 Hz).",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -3,
        machineHeadA4,
        0.0,
        true,
        {
            makeString("C#2", 37, machineHeadA4),
            makeString("F#2", 42, machineHeadA4),
            makeString("B2", 47, machineHeadA4),
            makeString("E3", 52, machineHeadA4),
            makeString("G#3", 56, machineHeadA4),
            makeString("C#4", 61, machineHeadA4)
        }
    });

    presets.push_back({
        "Machine Head",
        "Drop C (Davidian / Burn My Eyes)",
        "Early classic Machine Head tuning.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -2,
        440.0,
        0.0,
        true,
        {
            makeString("C2", 36, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    presets.push_back({
        "Machine Head",
        "D Standard (40 Cents Sharp)",
        "D Standard adjusted to Robb Flynn's +40 cent sharp reference (A = 450.3 Hz).",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -2,
        machineHeadA4,
        0.0,
        true,
        {
            makeString("D2", 38, machineHeadA4),
            makeString("G2", 43, machineHeadA4),
            makeString("C3", 48, machineHeadA4),
            makeString("F3", 53, machineHeadA4),
            makeString("A3", 57, machineHeadA4),
            makeString("D4", 62, machineHeadA4)
        }
    });

    // --- METALCORE ---
    presets.push_back({
        "Metalcore",
        "Drop D",
        "Classic 2000s metalcore starting point.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        0,
        440.0,
        0.0,
        true,
        {
            makeString("D2", 38, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("G3", 55, 440.0),
            makeString("B3", 59, 440.0),
            makeString("E4", 64, 440.0)
        }
    });

    presets.push_back({
        "Metalcore",
        "Drop C",
        "Defining genre middle-ground (Killswitch Engage, As I Lay Dying).",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -2,
        440.0,
        0.0,
        true,
        {
            makeString("C2", 36, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    presets.push_back({
        "Metalcore",
        "Drop B",
        "Heavy modern metalcore (Parkway Drive, Architects).",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -3,
        440.0,
        0.0,
        true,
        {
            makeString("B1", 35, 440.0),
            makeString("F#2", 42, 440.0),
            makeString("B2", 47, 440.0),
            makeString("E3", 52, 440.0),
            makeString("G#3", 56, 440.0),
            makeString("C#4", 61, 440.0)
        }
    });

    presets.push_back({
        "Metalcore",
        "Drop A (6-String Baritone)",
        "Ultra-heavy & progressive metalcore.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -5,
        440.0,
        0.0,
        true,
        {
            makeString("A1", 33, 440.0),
            makeString("E2", 40, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("F#3", 54, 440.0),
            makeString("B3", 59, 440.0)
        }
    });

    // --- DEATHCORE ---
    presets.push_back({
        "Deathcore",
        "Drop A (7-String)",
        "Standard for modern deathcore (Whitechapel, Suicide Silence).",
        "PHYSICAL GUITAR SETUP: TUNE TO B-E-A-D-G-B-E (7-Str Drop A low A)",
        0,
        440.0,
        0.0,
        true,
        {
            makeString("A1", 33, 440.0),
            makeString("E2", 40, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("G3", 55, 440.0),
            makeString("B3", 59, 440.0),
            makeString("E4", 64, 440.0)
        }
    });

    presets.push_back({
        "Deathcore",
        "Drop G (7-String)",
        "Downtempo & slam deathcore (Drop A 7-str shifted -2).",
        "PHYSICAL GUITAR SETUP: TUNE TO A-E-A-D-G-B-E (7-String Drop A)",
        -2,
        440.0,
        0.0,
        true,
        {
            makeString("G1", 31, 440.0),
            makeString("D2", 38, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    presets.push_back({
        "Deathcore",
        "Drop E (8-String)",
        "8-string sub-bass chugs.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-E-A-D-G-B-E (8-String Drop E)",
        0,
        440.0,
        0.0,
        true,
        {
            makeString("E1", 28, 440.0),
            makeString("B1", 35, 440.0),
            makeString("E2", 40, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("G3", 55, 440.0),
            makeString("B3", 59, 440.0),
            makeString("E4", 64, 440.0)
        }
    });

    presets.push_back({
        "Deathcore",
        "Drop B (6-String Mid-2000s)",
        "Classic mid-2000s 6-string deathcore.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -3,
        440.0,
        0.0,
        true,
        {
            makeString("B1", 35, 440.0),
            makeString("F#2", 42, 440.0),
            makeString("B2", 47, 440.0),
            makeString("E3", 52, 440.0),
            makeString("G#3", 56, 440.0),
            makeString("C#4", 61, 440.0)
        }
    });

    // --- COMMON DEATH METAL ---
    presets.push_back({
        "Common Death Metal",
        "D Standard",
        "Classic death metal tuning (Death, Morbid Angel).",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -2,
        440.0,
        0.0,
        true,
        {
            makeString("D2", 38, 440.0),
            makeString("G2", 43, 440.0),
            makeString("C3", 48, 440.0),
            makeString("F3", 53, 440.0),
            makeString("A3", 57, 440.0),
            makeString("D4", 62, 440.0)
        }
    });

    presets.push_back({
        "Common Death Metal",
        "C Standard",
        "Dismember, Entombed, Gorguts. 4 semitones down.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -4,
        440.0,
        0.0,
        true,
        {
            makeString("C2", 36, 440.0),
            makeString("F2", 41, 440.0),
            makeString("A#2", 46, 440.0),
            makeString("D#3", 51, 440.0),
            makeString("G3", 55, 440.0),
            makeString("C4", 60, 440.0)
        }
    });

    presets.push_back({
        "Common Death Metal",
        "B Standard (6-String)",
        "Carcass, At The Gates. 5 semitones down.",
        "PHYSICAL GUITAR SETUP: TUNE TO E-A-D-G-B-E (Standard)",
        -5,
        440.0,
        0.0,
        true,
        {
            makeString("B1", 35, 440.0),
            makeString("E2", 40, 440.0),
            makeString("A2", 45, 440.0),
            makeString("D3", 50, 440.0),
            makeString("F#3", 54, 440.0),
            makeString("B3", 59, 440.0)
        }
    });

    presets.push_back({
        "Common Death Metal",
        "Drop B (6-String)",
        "C# Standard with low string dropped to B.",
        "PHYSICAL GUITAR SETUP: TUNE TO D-A-D-G-B-E (Drop D)",
        -3,
        440.0,
        0.0,
        true,
        {
            makeString("B1", 35, 440.0),
            makeString("F#2", 42, 440.0),
            makeString("B2", 47, 440.0),
            makeString("E3", 52, 440.0),
            makeString("G#3", 56, 440.0),
            makeString("C#4", 61, 440.0)
        }
    });

    return presets;
}

} // namespace Tunings
