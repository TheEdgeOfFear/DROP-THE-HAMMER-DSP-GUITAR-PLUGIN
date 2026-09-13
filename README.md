# DONT DROP THE HAMMER

### Next-Generation Polyphonic Pitch Transposition, Whammy Expression & Radial Strobe Tuner DSP Plugin
**Created by THE EDGE OF FEAR**

![DONT DROP THE HAMMER](DROP%20THE%20HAMMER.png)

---

## Overview

**DONT DROP THE HAMMER** is a boutique, studio-grade DSP guitar audio plugin designed for modern heavy guitarists, metal producers, and sound designers. Engineered from the ground up in modern C++20 with JUCE 8, it combines an ultra-transparent polyphonic drop transposer, a dual-octave whammy expression pedal rocker, a precision radial strobe tuner, and full MIDI integration into a seamless edge-to-edge hardware-inspired interface.

Whether dropping standard E tuning down to Drop A, shifting semitones in real-time with an expression pedal, or tuning on stage with a laser-locked strobe radar, **DONT DROP THE HAMMER** delivers zero-artifact, pristine 32-bit floating-point audio.

---

## Key Features

### 1. Zero-Artifact Polyphonic Pitch Shifter
- **Pristine Headroom**: 32-bit float internal processing with transparent audio headroom. Zero unwanted harmonic distortion or overdrive on high-output guitar pickups.
- **Sub-Sample Hermite Interpolation**: 4-point cubic Hermite interpolation with a 256-sample safety buffer offset ensures read pointers never cross write boundaries, eliminating all clicks, pops, and discontinuities.
- **Instantaneous Transposition**: Drop or pitch up across +/- 24 semitones with continuous parameter smoothing (zero zipper noise during rapid pedal sweeps).

### 2. Hammer Whammy Expression Pedal Unit
- **3D Articulated Rocker Plate**: Rendered with visceral black diamond plate metal and blood splatter artwork that tilts dynamically with mouse drag or external MIDI expression controllers.
- **Dynamic 3D Lighting & Cast Shadow**: Real-time perspective transforms, cast drop shadow depth, and metallic specular highlights react to rocker position.
- **Customizable Range Limits**: Independent `BOTTOM` (-24 to +24 ST) and `TOP` (-24 to +24 ST) rotary limit dials.
- **2X Octave Mode**: Doubles the effective expression throw up to +/- 4 octaves (+48 / -48 ST) for extreme dive bombs and octave screeches.
- **Engage Footswitch**: True bypass stomp button with glowing crimson LED halo ring.

### 3. Radial Strobe Tuner & Transposer Hub
- **Concentric Laser Strobe Rings**: Dual counter-rotating radial strobe segments (Neon Blue when locked in-tune, Blood Red when drifting).
- **Pitch Detection Engine**: Real-time YIN autocorrelation tracking with +/- 0.1 cent precision.
- **Fine Cents Needle & Note Display**: Bold pitch readout with laser lock bar, cent offset readout, and bottom centering needle.
- **Pre / Post DSP Monitoring**: Toggle between monitoring your raw physical guitar DI signal (`PRE-RAW`) or the pitch-shifted output (`POST-SHIFT`).
- **Flexible Reference Frequency**: Calibrate A4 between 420.0 Hz and 460.0 Hz (double-click to instant 440.0 Hz reset).
- **Strobe Speed & Accidental Display**: Toggle between normal / fast strobe rotation, and sharp (#) / flat (b) naming conventions.

### 4. Preset Management & Master Section
- **Multi-Category Preset Browser**: Factory presets tailored for Drop D, D Standard, Drop C, Drop B, Drop A, Drop F, Whammy dive bombs, and custom artist tunings.
- **Interactive Preset Modification**: Save custom user presets and manage category banks.
- **Master Stage Dials**: Precision `INPUT` gain (0.0x to 2.0x), `MIX` Dry/Wet blend (0% to 100%), and `OUTPUT` master volume with double-click reset.

### 5. Universal MIDI Learn & CC Automation
- **Right-Click MIDI Mapping**: Right-click any dial, switch, or expression pedal to open the interactive MIDI mapping modal.
- **Hardware Controller Support**: Compatible with Behringer FCB1010, Line 6 FBV, Roland EV-5, expression pedals, and generic MIDI CC knobs/faders.

---

## How to Install

### Option A: 1-Click Automated VST3 Installer (Recommended)
1. Navigate to the `VST3/` folder.
2. Right-click **`INSTALL_DONT_DROP_THE_HAMMER_VST3.bat`** and select **Run as Administrator** (or double-click; it will request elevation).
3. The script will automatically remove older versions and deploy `DONT DROP THE HAMMER.vst3` into `C:\Program Files\Common Files\VST3\`.
4. Open your DAW (Cakewalk Sonar, Reaper, Cubase, FL Studio, Ableton Live, Studio One, Bitwig) and run a VST rescan.

### Option B: Manual VST3 Installation
1. Copy the entire `DONT DROP THE HAMMER.vst3` directory from `VST3/` into:
   ```
   C:\Program Files\Common Files\VST3\
   ```
2. Rescan plugins in your DAW.

### Option C: Standalone Executable (No DAW Required)
1. Open the `Standalone/` folder.
2. Run **`DONT DROP THE HAMMER.exe`**.
3. Under `Audio/MIDI Settings`, select your **ASIO Audio Interface** (e.g., Focusrite Scarlett, MOTU, Universal Audio, or Generic ASIO/FL Studio ASIO) and choose your active input/output channels and sample rate.

---

## How to Use the DSP Guitar Pedal

```
 Guitar DI In ---> [INPUT GAIN] ---> [PITCH SHIFTER DSP] ---> [MIX BLEND] ---> [OUTPUT] ---> Amp Sim / DAW
                         |                     ^
                         v                     |
                [RADIAL STROBE]       [EXPRESSION ROCKER /
                 (PRE / POST)          SEMITONE TRANSPOSER]
```

### 1. Instant Drop Tuning & Transposition
1. Select the semitone shift via the **Pitch Transposition** drop-down on the right stage (e.g., `-2 ST` for D Standard, `-4 ST` for Drop C).
2. The glowing crimson LCD readout displays the active transposition offset and physical guitar setup recommendations.
3. The DSP handles full polyphony across chords, single-note leads, and low-tuned palm mutes without tracking latency.

### 2. Using the Expression Pedal
1. Click and drag the **Black Diamond Plate Rocker** up and down to sweep pitch in real-time.
2. Adjust the lower deck dials:
   - **BOTTOM**: Pitch at heel-down position (default: `-12 ST` / 1 octave down).
   - **TOP**: Pitch at toe-down position (default: `+12 ST` / 1 octave up).
   - **2X OCTAVE**: Toggle ON to double the range (e.g., `-24 ST` to `+24 ST`).
3. Click the **ENGAGE** chrome footswitch or stomp button to bypass or enable expression pitch shifting instantly.

### 3. Tuning Your Guitar with the Radial Strobe
1. Click **ENGAGE TUNER** on the right unit or click directly on the circular radar lens.
2. Pluck a string:
   - If the note is flat, the strobe rings rotate counter-clockwise.
   - If the note is sharp, the strobe rings rotate clockwise.
   - When in tune, the strobe segments stop rotating and lock into a glowing **Neon Blue** laser bar.
3. Switch **MONITOR** to `PRE-RAW` to tune your physical guitar strings, or `POST-SHIFT` to confirm the transposed output frequency.

### 4. Mapping External MIDI Controllers
1. Right-click the **Expression Pedal Rocker** or any knob.
2. Select **MIDI Learn / Assign CC**.
3. Move your hardware MIDI foot pedal (e.g., CC #11 or CC #1) or turn a knob on your MIDI keyboard. The plugin will instantly bind and lock the parameter.

---

## Building from Source

### Prerequisites
- Windows 10 / 11 64-bit
- Visual Studio 2022 (MSVC v143 or later) with C++20 support
- CMake 3.22+
- Git

### Build Instructions
```powershell
# Clone the repository
git clone https://github.com/TheEdgeOfFear/DROP-THE-HAMMER-DSP-GUITAR-PLUGIN.git
cd DROP-THE-HAMMER-DSP-GUITAR-PLUGIN/Raw Files

# Configure build with CMake (JUCE 8 FetchContent)
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Release targets (VST3 and Standalone)
cmake --build build --config Release --target DontDropTheHammer_VST3 DontDropTheHammer_Standalone
```

All the raw files are supplied for your adjustments and builds - stay RAD Metal Heads... The Edge Of Fear 

Youtube Channel - https://www.youtube.com/@theedgeoffearmetal

Instagram - https://www.instagram.com/theedgeoffear/

Soundcloud - https://soundcloud.com/user-290758847
---

## License & Credits

- **Developer**: THE EDGE OF FEAR
- **Framework**: JUCE 8.0.4 (C++20)
- **Engine**: Custom Real-Time Polyphonic Time-Domain Pitch Shifter & YIN Autocorrelation Strobe Radar
- **License**: See [LICENSE](LICENSE) file.
