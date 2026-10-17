# SENSOR family graphical design

FMX now follows the actual SENSOR FLUX GUI, rather than the earlier warm amber camera-panel interpretation.

## Source reference and reuse

Reference repository: [AakeKBergh/SensorFlux](https://github.com/AakeKBergh/SensorFlux), commit `1e67934601b3aacd980b504b20b285921c9def06`.

- `Source/InstrumentLook.h` supplies the production rotary dial geometry, dotted tick marks, shadows, metallic face/bevel, pointer, prominent value arcs, waveform paths, button bevels and red active-state treatment. These implementations are adapted in `Source/SensorLookAndFeel.h`.
- `FluxEditor::paint` in `Source/Plugin.cpp` supplies the dark vertical graphite palette, neutral etched rules, tracked product branding and section typography. FMX extracts these into `Source/SensorStyle.h`.
- The reference's production screenshot (`docs/images/sensor-flux-gui.png`) was inspected alongside the code.

These headers contain no DSP or parameter routing. Future SENSOR instruments can reuse `SensorLookAndFeel` and `SensorStyle` directly. `CustomLookAndFeel.h` remains an FMX compatibility alias, not another independent theme. The reference source is recorded here so future family-wide styling changes can be reconciled deliberately; there is no runtime dependency on the FLUX repository.

## Family rules

- Panel: graphite `#1A1B1D` to `#0C0D0F`; rules `#353638`.
- Product ink: `#E0E1E3`; secondary ink: `#8E9094`; control labels: `#C3C4C6`.
- Active-state red: `#DF686D`, with FLUX's restrained red index tip and LED accents. This replaces the previous FMX amber.
- Rotary behavior: horizontal/vertical drag and editable numeric readouts remain unchanged. Normal dials use FLUX's dotted scales and bevelled faces. Cutoff, Width, Air and Master use its prominent white value arc with a short red tip.
- Wave selectors use FLUX's four waveform glyphs, short labels and active underline. The underlying FMX choices and full accessible button names are retained.
- Chorus buttons keep FLUX's bevel/LED language, but retain their labels while active so I/II never becomes ambiguous. Slope buttons retain readable LP12/LP24 states. Press-again chorus bypass behavior is unchanged.
- Value readouts are printed directly beneath the dial, with an editor focus outline only while editing. Existing FMX units and parsing are retained: percentages, harmonic values, cents, Hz/kHz, ms/s, Q and octaves. No parameter range or scaling changes accompany the visual update.

## FMX-specific composition

The 1180 × 760 layout preserves mirrored Oscillator A/B groups, Tone / Space (Width, Air, slope and chorus), Filter plus Filter ADSR, and Amplifier ADSR / Velocity / Master. It does not reproduce FLUX's sample display, source controls, scope, sub oscillator or effects layout, and adds no placeholder controls. The product remains branded SENSOR FMX with its own subtitle.

## Verification

Linux Release VST3 and standalone targets build; the full smoke suite passes 65 checks, including existing DSP/patch/state/automation checks plus editor parameter invariance and non-overlapping control bounds. The screenshot is generated from the actual JUCE editor. DSP sources, parameter IDs/order/defaults, state serialization and sound-engine behavior are unchanged. Windows native rendering, DPI scaling and DAW interaction still require checking on Windows.
