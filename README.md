# SENSOR FMX

FM / SUBTRACTIVE HYBRID SYNTHESIZER

A C++17 / JUCE 8.0.6 instrument with VST3 and standalone targets, eight independent voices and 22 automatable parameters. All controls use AudioProcessorValueTreeState attachments; the complete parameter state is saved with the host project.

## Build

CMake 3.22+, a C++17 compiler and Git are required. The first configure downloads JUCE at a fixed commit. Alternatively pass `-DJUCE_DIR=/path/to/JUCE` for an existing JUCE 8.0.6 checkout.

On Debian/Ubuntu install the development prerequisites:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev xvfb xauth
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
xvfb-run -a ctest --test-dir build --output-on-failure
```

### Windows (Visual Studio 2022, x64)

Install Git, CMake 3.22 or newer, and Visual Studio 2022 with **Desktop development with C++**, including MSVC v143 and a Windows SDK. Run these commands in PowerShell:

```powershell
git clone https://github.com/AakeKBergh/SensorFMX.git
cd SensorFMX
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

CMake downloads the complete JUCE source at commit `51a8a6d7aeae7326956d747737ccf1575e61e209` (JUCE 8.0.6), including its bundled VST3 SDK. No separately installed JUCE or VST3 SDK is required. Internet access to GitHub is required on the first configure. For an existing JUCE checkout, add `-DJUCE_DIR=C:/path/to/JUCE` to the configure command.

The Windows plugin is `build/SensorFMX_artefacts/Release/VST3/SENSOR FMX.vst3`; copy the entire bundle into `C:/Program Files/Common Files/VST3/` (administrator permissions may be needed), then rescan plugins in your DAW. The standalone executable is `build/SensorFMX_artefacts/Release/Standalone/SENSOR FMX.exe`.

On macOS use Xcode command line tools and CMake. Linux is the validated platform for this initial version; Windows/macOS compilation and host compatibility still require testing on those systems.

VST3 bundles are under `build/SensorFMX_artefacts/Release/VST3/`. Copy `SENSOR FMX.vst3` into the host's VST3 directory (Linux: `~/.vst3/`) and rescan. Standalone executables are under `build/SensorFMX_artefacts/Release/Standalone/`. Select an audio device and MIDI input in the standalone audio settings. The interface receives notes from MIDI; it does not include an on-screen keyboard.

### Prepared cloud machine

The restricted cloud image uses a local toolchain instead of system package installation:

```sh
. /workspace/toolchain/activate.sh
cd /workspace/SensorFMX
cmake -S . -B build-linux -G 'Unix Makefiles' \
  -DJUCE_DIR=/workspace/toolchain/JUCE -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/workspace/toolchain/root/usr
cmake --build build-linux --parallel 2
xvfb-run -a ctest --test-dir build-linux --output-on-failure
```

The corresponding artifacts are in `build-linux/SensorFMX_artefacts/Release/`. A real DAW and audio device are needed for interactive auditioning; Xvfb tests only exercise off-screen UI rendering.

## Signal path

Each MIDI note owns two FM engines: a sine carrier is phase-modulated by a sine modulator at `carrier frequency × ratio`. Both phases advance continuously within a note. Fine tune is in cents, FM depth in radians and FM decay in seconds (exponential time constant). FM depth decays toward zero after note onset. Oscillator levels feed a mono mixer, then a resonant TPT low-pass filter, then an amplitude ADSR. The filter has its own ADSR with a bipolar amount in octaves. Voices are mixed to identical left/right outputs and passed through a smoothed master gain.

Each voice has independent oscillator phases, filter state and envelopes. Minimum attack/release times prevent instantaneous normal note edges. Cutoff, resonance, filter amount, oscillator settings and master gain use 20 ms smoothing. MIDI pitch bend spans ±2 semitones and sustain pedal is supported. Eight voices are available, including release tails; additional notes are ignored while all eight are occupied to avoid abrupt voice-stealing clicks. High FM depth/ratio and high notes can alias; oversampling and band limiting are outside this initial implementation. Resonance can boost peaks, so lower master volume for extreme patches.

## Source layout

- `Parameters.h`: stable parameter IDs, ranges, defaults and host automation.
- `FMOscillator.h`: reusable phase modulation engine and FM decay.
- `SynthVoice.h`: `SynthVoice` and `SynthSound`, per-voice filter and envelopes.
- `PluginProcessor.*`: MIDI, voice mixing, master gain and project state.
- `PluginEditor.*`: labelled rotary controls, APVTS attachments and graphite panels.
- `Tests/SmokeTests.cpp`: audio, pitch, eight-voice mixing, sustain/release, automation extremes, state and GUI checks.

JUCE is a separate dependency. Review its licensing terms before distributing a plugin.
