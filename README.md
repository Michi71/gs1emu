# GS1-EMU — Yamaha GS1 Grand Synthesizer Emulator

A software emulator for the **Yamaha GS1** (1981), one of the first commercial FM synthesizers ever produced. The emulator aims to be as hardware-accurate as possible, using the same log-domain signal path, lookup tables, and operator topology as the original chips.

> **Original engine by [giulioz](https://github.com/giulioz/gs1.git).**  
> This fork extends the original with authentic GS1 control features, improved presets, a polyphonic voice engine, and a terminal-based patch editor.

---

## About the Yamaha GS1

The GS1 (1981) was Yamaha's first commercially available FM synthesizer — a grand piano-sized instrument priced at around $16,000. It predates the DX7 by two years and uses a dedicated FM VLSI chip set that Yamaha developed in collaboration with Stanford University's John Chowning.

```
FM Module (2× per GS1):

        +-----+                                         +-----------+
        |     V                                         V           |
        |   +-----------------------+   +-----------------------+   |
   Pi   |   |     Modulator 1       |   |     Modulator 2       |   | Pi
   Pi/2 |   | VRG / EG / EC / PG/OP |   | VRG / EG / EC / PG/OP |   | Pi/2
        |   +----------+------------+   +------------+----------+   |
        |              |                             |              |
        +--------------+                             +--------------+
                       |                             |
                       V                             V
             +------------------+         +------------------+
             |    Carrier 1     |         |    Carrier 2     |
             | VRG/EG/EC/PG/OPC |         | VRG/EG/EC/PG/OPC |
             +------------------+         +------------------+
                       |                             |
                       +-------------+---------------+
                                     V
                              DAC / Output
```

Two such FM modules run in parallel — 4 operators total per voice (2 Carriers + 2 Modulators), with configurable cross-modulation between stacks.

**Hardware Specs:**
- 88 keys, A1–C7
- 16-note polyphony
- 34,687 Hz internal sample rate
- 8 EGs (2 per operator), keyboard scaling per operator

---

## Features

### FM Engine
- **2-stack FM synthesis** per voice: each stack = Carrier + Modulator
- **Log-sin / exp lookup tables** — same approach as original Yamaha FM chips
- **28-bit phase accumulators**, 10-bit phase resolution (intentional quantization)
- **4 FM routing modes** per stack:
  - `NORM` — standard FM
  - `PI/2` — modulator with half-intensity self-feedback
  - `PI` — modulator with full self-feedback
  - `CROSS` — cross-modulation between stacks

### Double Stack (Layer Engine)

The original GS1 has a **Double Stack** switch that engages a second, independent 4-operator FM stack per voice. This fork extends it from a simple "voice doubler" into a full **per-preset layer engine** — the second stack can either thicken the primary sound (chorus) or contribute a completely separate sound layer (e.g. a hammer/pluck transient added on top of a sustained body).

Two mix modes are selectable per preset via `DS_MixMode`:

| Mode | Value | Behaviour |
|------|-------|-----------|
| **BLEND** | `0` | Stack 2 is a slightly detuned clone of stack 1, averaged in (`(base·wB + layer·wL)/(wB+wL)`) — a chorus/ensemble thickening. Used by the sustained voices (Strings, Brass, Organs). |
| **ADD** | `1` | Stack 2 is an *independent* layer summed on top of stack 1 at full level (`base·wB + layer·wL`) — typically a short attack transient. Used by the percussive voices (Harpsichords, Clavichord, Vibraphone, Celeste, Pianos). |

In ADD mode the layer is a self-contained second patch with its own ratios, FM routing, envelopes, and keyboard scaling. Because it is summed rather than averaged, the primary body is left untouched and only the attack/transient character is enriched. For example the Acoustic Piano adds a soft felt-hammer thump, while the Harpsichords add a low, woody key-mechanism *thunk* that contrasts with the bright native pluck.

All layer parameters are **backward-compatible** via sentinel defaults — a preset that sets none of them behaves exactly as the original single-stack patch.

| Parameter | Description |
|-----------|-------------|
| `DS_Ratio[4]` | Operator frequency ratios for stack 2. `≤ 0` ⇒ inherit stack 1's `Ratio[]`. |
| `DS_FMmode[2]` | FM routing for the two stack-2 pairs. `< 0` ⇒ inherit stack 1's `FMmode[]`. |
| `DS_LevelDb[4]` | Per-operator level offset in dB (carriers = loudness, modulators = brightness / FM index). |
| `DS_ATScale[4]` / `DS_DTScale[4]` | Attack-/decay-time scalers relative to stack 1's envelopes. |
| `DS_Detune[4]` | Per-operator detune for BLEND-mode chorus width. |
| `DS_MixBase` / `DS_MixLayer` | Mix weights for stack 1 (base) and stack 2 (layer). |
| `DS_ModKbdDb` | Modulator attenuation per octave above C2 — keeps high notes from turning metallic. |
| `DS_DTKbdTrack` | Exponent on the keyboard decay-tracking factor (`1.0` = follow stack 1, `0` = flat) — keeps transient length uniform across the keyboard. |
| `DS_MixMode` | `0` = BLEND (chorus), `1` = ADD (layer). |

### Voice Engine
- **16-voice polyphony** with intelligent voice stealing:
  1. Silent voices first
  2. Quietest releasing voice
  3. Oldest active voice
- **Sustain pedal** (MIDI CC 64) support
- **Keyboard scaling** via 46-point EC arrays (44 zones × 2 semitones)
- **Velocity sensitivity** per operator

### Controls (matching original GS1 panel)

| Control | Range | Description |
|--------|-------|-------------|
| **Detune** | 5 positions | RANDOM 2 / RANDOM 1 / OFF / STATIC 1 / STATIC 2 |
| **Tremolo** | On/Off, 1–6 Hz, 0–100% | Amplitude modulation on carriers only |
| **Vibrato** | On/Off, 4–10 Hz, 0–100% | Pitch modulation, all operators in sync |
| **Ensemble** | On/Off | Chorus effect with 3 modulated delay lines |
| **EQ Bass** | ±12 dB @ 100 Hz | Low shelf filter |
| **EQ Mid** | ±12 dB @ 600 Hz | Peaking bell filter (Q=1.0) |
| **EQ Treble** | ±12 dB @ 6 kHz | High shelf filter |
| **Master Volume** | 0.0–2.0 (1.0 = unity, 2.0 = +6 dB) | Global output gain, like the original GS1 volume slider (`setMasterVolume`) |

#### Detune Modes
| Mode | Behaviour |
|------|-----------|
| `RANDOM 2` | ±8 cents random offset assigned at each note-on |
| `RANDOM 1` | ±3 cents random offset assigned at each note-on |
| `OFF` | No detuning |
| `STATIC 1` | Fixed per-voice offset, up to ±5 cents (stable chorus) |
| `STATIC 2` | Fixed per-voice offset, up to ±12 cents (wider chorus) |

### Factory Presets (16 voices)
The 16 factory-programmed voices from the original GS1 card reader:

| # | Voice | | # | Voice |
|---|-------|-|---|-------|
| 1 | Harpsichord I | | 9 | Acoustic Piano I |
| 2 | Harpsichord III | | 10 | Electric Piano I |
| 3 | Clavichord I | | 11 | Brass I |
| 4 | Clavichord II | | 12 | Brass II |
| 5 | Vibraphone | | 13 | Synth Brass III |
| 6 | Celeste | | 14 | Electronic Organ I |
| 7 | String I | | 15 | Electronic Organ II |
| 8 | String Ensemble I | | 16 | Pipe Organ |

### Extended Preset Pack (programs 17–36)

A curated 20-preset pack appended after the factory voices (scrollable in the standalone via `+`/`-`). Each preset is built by the `gs1MakeEpDual()` factory in `gs1_presets_extended.h`, which **inherits all native values** (ratios, detune, EC keyboard-scaling curves, envelopes, FM routing) from a category-appropriate factory base — so the instrument's true timbre is preserved — and then differentiates the variants along a few musical axes. The second stack runs as a detuned BLEND/chorus on the inherited voicing.

| # | Category (base preset) | Variants |
|---|------------------------|----------|
| 17–24 | **Electric Piano** (`ElectricPianoI`, native ratio 1/1/1/1) | Soft / Clear / Hard / GS1 Deluxe Digital, Warm Pad, Bellish, Chorus Wide, Vintage GS1 |
| 25–28 | **Grand Piano** (`AcousticPianoI`, native ratio 1/2/1/4) | Soft / Clear / Hammer / GS1 Deluxe |
| 29–32 | **Vibraphone** (`Vibraphone`, native ratio 1/1/8/8 → metallic) | Soft / Clear / Hard / GS1 Deluxe |
| 33–36 | **Marimba** (`ClavichordII`, native ratio 1/1/3/4 → woody) | Soft / Clear / Hard / GS1 Deluxe |

`gs1MakeEpDual(base, name, attackScale, bAttackScale, bDetune, brightness, mixLayer, outLevelDb = 0, decayScale = 1)`:

| Axis | Effect |
|------|--------|
| `attackScale` | Factor on the native attack **rate** (`ATE` is a per-sample increment, *not* a time): `<1` = slower/softer, `>1` = faster/harder. |
| `bAttackScale` | Same for the layer-B (chorus) stack. |
| `bDetune` | Layer-B chorus detune in cents (symmetrically spread). |
| `brightness` | Modulator (M1/M2) level offset in dB → FM index = **timbre**. Negative = darker/woodier, positive = more bite. The main axis separating variants within a family. |
| `mixLayer` | Layer-B mix weight (BLEND/chorus). |
| `outLevelDb` | Whole-preset output level in dB (loudness matching). |
| `decayScale` | Factor on the native decay **rate** (`DTE`); `>1` = shorter, more percussive tail (e.g. the marimba's quick woody pluck). |

These map onto two general-purpose `PatchConsts` fields usable by any preset:
- **`BaseLevelDb[4]`** — per-operator level offset for the base stack (modulators = brightness/FM index, carriers = loudness). Always active, mirrors `DS_LevelDb`.
- **`OutLevelDb`** — per-preset output gain (also used to tame the slightly hot factory Harpsichords).

---

## Architecture

```
gs1emu/
├── emulator/
│   └── src/
│       ├── gs1emu.h          # CGS1Emu class, VoiceState, GS1BiquadFilter
│       ├── gs1emu.cpp         # FM engine, voice management, LFO, EQ
│       ├── gs1_presets.h      # PatchConsts structs, factory preset array
│       ├── delayline.h/.cpp   # Interpolated delay line (ensemble effect)
│       └── patchdata.h        # Raw patch parameter tables
├── editor/
│   └── src/
│       ├── main.cpp           # Entry point
│       ├── tui.h/.cpp         # ncurses-based patch editor (80×24)
│       ├── audio_engine.h/.cpp
│       ├── sysex.h/.cpp       # SysEx import/export
│       └── curve_generator.h/.cpp
├── test/
│   └── standalone.cpp         # SDL2 + PortMidi test application
└── CMakeLists.txt
```

### Signal Path
```
MIDI Note-On
    │
    ▼
findVoice()  ──  Voice Stealing
    │
    ▼
noteOn()  ──  CW calculation, Detune, EC scaling
    │
    ▼ (per sample, per active voice)
fmGenSample()
    ├── Envelope Generator (Attack → Decay → Sustain → Release)
    ├── Tremolo attenuation (log-domain, carriers only)
    ├── Phase accumulator update (+ Vibrato pitch shift)
    ├── FM operator computation (NORM / PI/2 / PI / CROSS)
    ├── Base/layer level offsets (BaseLevelDb / DS_LevelDb)
    ├── Double Stack mix (BLEND chorus / ADD layer) — if enabled
    └── Per-preset output gain (OutLevelDb)
    │
    ▼
processBlock()
    ├── Sum all active voices
    ├── Master Volume (global output gain)
    ├── Anti-aliasing lowpass (8 kHz, Butterworth)
    ├── 3-Band EQ (Bass → Mid → Treble)
    └── Ensemble (3× modulated delay lines) or Mono
```

---

## Dependencies

| Library | Purpose |
|---------|---------|
| [SDL2](https://libsdl.org/) | Audio output, thread-safe audio locking |
| [PortMidi](https://portmedia.sourceforge.net/) | MIDI input (virtual port) |
| [ncurses](https://invisible-island.net/ncurses/) | Patch editor TUI |

### macOS (Homebrew)
```bash
brew install sdl2 portmidi ncurses
```

### Linux (apt)
```bash
sudo apt install libsdl2-dev libportmidi-dev libncurses-dev
```

---

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This produces:
- `build/gs1_standalone` — standalone test application
- `build/editor/gs1_editor` — ncurses patch editor

---

## Standalone Test Application

Connects to a virtual MIDI port named `PicoGS1`. Connect any DAW, sequencer, or MIDI controller to this port to play.

```
./build/gs1_standalone
```

### Key Bindings

| Key | Function |
|-----|----------|
| `+` / `-` | Next / previous patch |
| `x` | Toggle Double Stack (layer engine) |
| `e` | Toggle Ensemble |
| `d` | Cycle Detune mode (RANDOM2 → RANDOM1 → OFF → STATIC1 → STATIC2) |
| `t` | Toggle Tremolo |
| `[` / `]` | Tremolo Speed −/+ 0.5 Hz |
| `{` / `}` | Tremolo Depth −/+ 0.1 |
| `v` | Toggle Vibrato |
| `n` / `m` | Vibrato Speed −/+ 0.5 Hz |
| `,` / `.` | Vibrato Depth −/+ 0.1 |
| `b` / `B` | EQ Bass −1 dB / +1 dB |
| `f` / `F` | EQ Mid −1 dB / +1 dB |
| `h` / `H` | EQ Treble −1 dB / +1 dB |
| `9` / `0` | Master Volume −/+ 0.1 (0.0–2.0) |
| `q` | Quit |

---

## Technical Notes

### Sample Rate
The emulator runs at **34,687 Hz** — the historically accurate internal clock rate of the GS1 hardware. This is not 44.1 kHz or 48 kHz; using the correct rate is essential for accurate pitch and envelope timing.

### Log-Domain Signal Path
Like the original chips, amplitude values are stored in **log-domain** (0 = maximum loudness, 4095 = silence, 256 units ≈ 6 dB). The lookup tables `logsinTable` and `expTable` replicate the hardware's log-sin / exp conversion, including the characteristic harmonic distortion of the original.

### Phase Accumulator
28-bit accumulators advance each sample, but only the upper **10 bits** are used as the lookup index — the remaining bits are intentionally discarded. This 10-bit quantization error contributes to the slightly gritty texture characteristic of early Yamaha FM.

### Vibrato Implementation
All 4 operators are pitch-shifted equally using a multiplicative factor on the phase increment: `CW[n] * (1 + fraction)`. This preserves the FM modulation index exactly — the timbre stays constant while the pitch modulates.

### EQ Filters
All three EQ bands use **biquad IIR filters** based on the [RBJ Audio EQ Cookbook](https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html). At 0 dB, all bands are mathematically flat (passthrough).

---

## Credits

| | |
|-|-|
| **Original FM engine** | [giulioz](https://github.com/giulioz/gs1.git) |
| **FM synthesis theory** | John Chowning (Stanford CCRMA) |
| **Reference hardware** | Yamaha GS1 (1981) |

---

## License

See the original repository at https://github.com/giulioz/gs1.git for licensing terms of the base engine. All extensions in this fork are provided under the same license.
