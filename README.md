# SENSOR FMX

FM / SUBTRACTIVE HYBRID SYNTHESIZER — prototype 0.3

A conventional two-oscillator synthesizer that gradually opens into FM coloration. Choose a familiar waveform, shape it, add Character or transient Punch, then sculpt the result with the filter. Eight voices, sustain pedal, ±2-semitone pitch bend, VST3 and standalone targets. Width, transient Air, LP12/LP24 and a two-mode stereo chorus broaden the palette. All 31 visible parameters support automation and project recall.

![SENSOR FMX hybrid synthesizer interface](docs/sensor-fmx.png)

## Build on Windows

Install Git, CMake 3.22+ and a current Visual Studio installation with **Desktop development with C++**, MSVC and a Windows SDK. Open its Developer PowerShell:

```powershell
git clone https://github.com/AakeKBergh/SensorFMX.git
cd SensorFMX
cmake -S . -B build
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

CMake selects the installed default generator. To choose a particular Visual Studio version, use the exact generator name listed by `cmake --help` with `-G`, and `-A x64` if desired. Choose it on the first configure or use a fresh build directory when changing generator. The project does not pin a Visual Studio version.

CMake downloads JUCE 8.0.6 at commit `51a8a6d7aeae7326956d747737ccf1575e61e209`, including its VST3 SDK. No separate SDK or Windows-specific dependency is required. GitHub access is needed for the first configure. To use an existing JUCE source checkout, add `-DJUCE_DIR=C:/path/to/JUCE`.

With a multi-configuration Visual Studio generator, the plugin bundle is `build/SensorFMX_artefacts/Release/VST3/SENSOR FMX.vst3`. Copy the entire bundle into `C:/Program Files/Common Files/VST3/` and rescan your DAW; administrator permissions may be required. The standalone executable is `build/SensorFMX_artefacts/Release/Standalone/SENSOR FMX.exe`. With a single-configuration generator, configure using `-DCMAKE_BUILD_TYPE=Release`.

The previous version was reported working on Windows. This revision is built and tested in Linux; Windows compilation, GUI scaling and DAW playback need verification on Windows.

## Build on Linux / macOS

C++17, CMake 3.22+ and Git are required. On Debian/Ubuntu:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev xvfb xauth
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
xvfb-run -a ctest --test-dir build --output-on-failure
```

On macOS install Xcode command line tools and CMake; use the configure/build commands above and run CTest without `xvfb-run`. macOS is not validated here.

Standalone applications are under `build/SensorFMX_artefacts/Release/Standalone/`, VST3 bundles under `build/SensorFMX_artefacts/Release/VST3/`. Linux VST3 installation usually uses `~/.vst3/`. The standalone application's audio settings select the audio device and MIDI input. There is no on-screen MIDI keyboard.

### Prepared cloud environment

```sh
. /workspace/toolchain/activate.sh
cd /workspace/SensorFMX
cmake -S . -B build-linux -G 'Unix Makefiles' \
  -DJUCE_DIR=/workspace/toolchain/JUCE -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/workspace/toolchain/root/usr
cmake --build build-linux --parallel 2
xvfb-run -a ctest --test-dir build-linux --output-on-failure -V
```

This machine's toolchain lives outside the repository. Local Windows/Linux builds use the normal prerequisites above. No cloud paths are needed by CMake. Xvfb verifies off-screen GUI rendering, not interactive playback through a physical audio device.

## Oscillator controls

| Control | Behavior |
| --- | --- |
| Wave | Sine, Triangle, Saw or Square is the audible carrier. |
| Shape | Neutral at 50%. Sine adds a subtle second harmonic; Triangle shifts its peak; Saw adds soft saturation away from centre; Square adjusts pulse width from 10–90%, with DC compensation. |
| Harmonics | Stepped selector: 0.5, 1, 1.5, 2, 3, 4, 5, 6 or 8. These set the hidden modulator's frequency relationship. |
| Character | 0–100% steady FM coloration, with a quartic depth curve: 0 at minimum, 0.25 radians at 50%, approximately 0.71 radians at 65%, 4 radians at maximum. The lower half stays gentle. |
| Punch | Extra FM at note onset: depth `3 × Punch²` radians, decaying exponentially with an automatically selected 35–140 ms time constant. At 0% there is no transient modulation. |
| Tune | Fine tuning, ±100 cents; pitch bend remains available. |
| Level | Oscillator mixer level. |

Default patch: A = Saw, Shape 50%, Harmonics 2, Character 12%, Punch 10%, Tune 0 ct, Level 70%. B = Triangle, Shape 50%, Harmonics 2, Character 7%, Punch 0%, Tune −3 ct, Level 30%. Width starts at 20%, Air at 10%, Velocity at 45% and Chorus is off. Filter starts at 7.2 kHz in LP12 mode, low resonance and +1 octave envelope. Amp attack is 10 ms, decay 300 ms, sustain 80%, release 400 ms.

## Tone / Space and performance

| Control | Behavior |
| --- | --- |
| Width | 0–100%. One centred oscillator at zero; two quiet wings blend in with symmetric detuning up to ±7 cents and increasing stereo spread. Three subvoices per oscillator maximum, with a retained centre signal for mono compatibility. |
| Air | 0–100%. Low-level noise high-passed at 2.8 kHz, strongest at note onset and automatically decaying with a 65–155 ms time constant. Air is breath; Punch is tonal bite. |
| LP12 / LP24 | Two-pole low-pass, or a second two-pole stage in series. The first stage carries the resonance control; the second uses Q=0.707. A 20 ms crossfade smooths slope changes. |
| Chorus I / II | Both off bypasses the effect. I is slow and subtle; II uses more depth and wet signal for pads. Press an active button again to bypass, or press the other to switch. No extra chorus parameters. |
| Velocity | 0–100%, default 45%. At zero note velocity does not change amplitude or cutoff. At maximum, note velocity scales amplitude and soft notes close the filter by up to 1.25 octaves. |

## Four musical starting points

These reproducible acceptance recipes are specified in `Tests/PalettePatches.h`. They are manual recipes rather than additional preset-browser controls. Unlisted settings use the default patch.

| Recipe | Oscillators / macros | Filter | Amp |
| --- | --- | --- | --- |
| Airy Pluck | Triangle + Saw; Character 12/6%, Punch 35%, Width 30%, Air 40%, Chorus I | LP12, 4.2 kHz, Env +2 oct, Decay 160 ms, Sustain 0% | Attack 4 ms, Decay 250 ms, Sustain 8%, Release 200 ms |
| Soft Pad | Saw + Triangle; levels 50/35%, Character 8/4%, Punch 0%, Width 85%, Air 8%, Chorus II | LP12, 6.5 kHz, Env +0.7 oct, Attack 600 ms | Attack 700 ms, Decay 1 s, Sustain 80%, Release 2 s |
| Wide Lead | Saw + Square; B Shape 46%, B Level 16%, Character 38/15%, Punch 15%, Width 55%, Air 8%, Chorus I | LP24, 6 kHz, Env +1.4 oct, Decay 350 ms, Sustain 50% | Attack 8 ms, Decay 200 ms, Sustain 80%, Release 250 ms |
| Bass | Saw + Square; levels 75/25%, Character 35%, Punch 40%, Width/Air/Chorus off | LP24, 700 Hz, Q=1, Env +3 oct, Attack 3 ms, Decay 200 ms, Sustain 0% | Attack 4 ms, Decay 180 ms, Sustain 40%, Release 120 ms |

Play the bass in the low register; the pad recipe is tested as a four-note chord. Tests render each recipe for eight seconds, check peak headroom, note release, stereo behavior and appropriate transient or swell behavior. To export 16-bit stereo WAV examples, set `SENSOR_FMX_DEMOS` to an output directory when running the tests. To export an off-screen GUI image, set `SENSOR_FMX_SCREENSHOT` to a PNG filename. These are test-only options, not audio-runtime requirements.

```powershell
$env:SENSOR_FMX_DEMOS = "$PWD/build/demos"
ctest --test-dir build -C Release --output-on-failure -V
```

## Signal path and DSP

For each MIDI voice:

`A/B wave + Shape + Character/Punch → stereo unison → A/B mixer + Air → stereo LP12/LP24 + filter ADSR → amp ADSR / velocity`

The summed voices feed `stereo Chorus → smoothed Master`. Width is applied before the filters. Every voice has independent phases, modulator and Punch states, detuned subvoices, Air generator/envelope, stereo filter state and envelopes. Air is mixed before the filter, so it follows the same tonal contour. Each delay line in the chorus is allocated in prepare, never in the callback.

Character uses `4 × value⁴` radians. It remains gentle through the lower 60–70%, with stronger metallic sounds reserved for the top end. Punch retains its separate transient modulation. The hidden modulator remains a sine at carrier frequency times the selected musical ratio. Phases remain continuous through tuning and macro changes. Wave changes crossfade over 10 ms; continuous oscillator settings, Width gains/detune, Air, cutoff, resonance, velocity and master are smoothed. Chorus bypass/mix changes ramp over 25 ms and its rate/depth over 50 ms.

Chorus is a feed-forward stereo fractional delay, centred at 12 ms with quadrature modulation. I uses 0.18 Hz / ±0.6 ms / 18% wet; II uses 0.24 Hz / ±1.4 ms / 34% wet. There is no feedback and no additional delay or reverb effect. Chorus state remains running through bypass, avoiding stale history on reactivation. An empty synth output becomes silent once the voices and short chorus tail finish.

At Character/Punch 0 and Shape 50%, the carrier waveforms remain conventional. Saw/Square use polyBLEP edge correction. Triangle is piecewise linear. Extreme phase modulation, Shape and high notes can still alias; oversampling is not implemented. Resonance boosts peaks, so lower master volume for extreme patches.

Eight MIDI voices remain available, including release tails. Additional notes are ignored while all voices are occupied to avoid abrupt voice stealing. At Width zero the side oscillators advance silently so enabling Width does not start disconnected phases. Mono hosts receive a fold-down, without phase-inversion widening. The maximum configuration is 8 MIDI voices × 2 oscillators × 3 subvoices; no allocation, resizing or shared noise-generator locks occur in processBlock/renderNextBlock.

## GUI and automation

The GUI uses SENSOR FLUX's actual production drawing code: cooler graphite surfaces, tracked family branding, metallic bevelled rotary dials, dotted scales, waveform glyph buttons, and restrained red indicators. Larger Cutoff, Width, Air and Master controls use white arcs with red tips. Unboxed value readouts retain FMX's musical units and editing behavior.

Four compact, aligned areas contain mirrored oscillators; Tone / Space with Width/Air and slope/chorus controls; Filter plus Filter ADSR; and Amp / Performance with Velocity and a separated Master. Active chorus buttons retain readable labels and a small LED. See [SENSOR family design](docs/sensor-family-design.md) for source provenance, palette tokens, reusable components and model-specific adaptations. This redesign changes no synthesis or parameter behavior.

Shared parameter IDs (`aLevel`, `aFine`, filter, envelopes, master, and B equivalents) are retained. New oscillator IDs are `aWave`, `aShape`, `aHarmonics`, `aCharacter`, `aPunch` and B equivalents. Removed technical parameters are not exposed as host controls. Schema v3 state restores every parameter; missing parameters use deterministic new-patch defaults. Legacy v1 project state keeps common values, maps old ratios to the nearest supported harmonic ratio, and approximates old FM depth with a sine carrier and Character. V1/V2 states load Width, Air and Chorus off, LP12, and full legacy velocity response. V2 Character is remapped to preserve its previous cubic modulation depth under the new quartic curve. Only saved scalar Character values can be remapped; older host automation envelopes retain their original normalized values and may sound different after the curve change. The old FM decay behavior cannot be reproduced exactly, and old automation lanes for removed IDs cannot migrate automatically.

## Source layout and tests

- `HybridOscillator.h`: familiar carrier waveforms, Shape, macro mapping, smoothing and Punch.
- `UnisonLayer.h`, `AirGenerator.h`, `VoiceFilter.h`, `SimpleChorus.h`: reusable stereo/tone processors.
- `SynthVoice.h`, `SynthSound.h`: per-voice DSP, envelopes and MIDI behavior.
- `Parameters.h`: stable APVTS parameter definitions, ranges and defaults.
- `PluginProcessor.*`: voice mixing, master gain, state versioning and legacy fallback.
- `PluginEditor.*`, `CameraKnob.h`, `WaveSelector.h`, `HardwareButtons.h`, `SensorLookAndFeel.h`, `SensorStyle.h`: UI components, host attachments and procedural hardware styling.
- `Tests/SmokeTests.cpp`, `PaletteTests.h`, `PalettePatches.h`: conventional waveform references, harmonic spectra, Character/Punch/Shape, ratio mapping, automation, polyphony, pitch, sustain/release, state and GUI rendering.

Linux VST3 and Standalone builds pass, and the smoke suite passes 65 checks. Validation covers three sample rates (44.1/48/96 kHz), waveform and macro spectra, stereo/mono behavior, chorus modes, LP12/LP24 frequency slopes, velocity, eight-note polyphony, state migration, GUI attachments and the four musical recipes. Windows/macOS compilation, display scaling and DAW playback still require verification on those platforms.

JUCE is a separate dependency; review its licensing terms before distribution.
