# sample2gs1 — Sample → GS1 preset (reverse engineering)

Turns a sample library of a real instrument (mono WAV) into a GS1 double-stack
preset that approximates the instrument in the GS1's character. Two stages:

1. **Stage 1 — heuristic mapper** (`analyze.py`): extracts per-note features
   (fundamental, amplitude envelope, brightness, partial structure) and derives
   a preset directly. Fast, interpretable, data-driven.
2. **Stage 2 — analysis-by-synthesis** (`optimize.py`): renders the preset
   through `libgs1emu` and refines it with **CMA-ES** against the target samples
   (spectral + envelope + harmonic loss over several notes). It finds the best
   compromise *within* the engine's characteristics and exploits parameter
   interactions the heuristic cannot see.

> Honest expectation: a recognizable, GS1-flavored approximation — not a 1:1
> clone. Pure 4-operator FM can only hint at the hammer noise / inharmonicity of
> acoustic sounds. Works well for harmonic timbres (EP, organ, vibraphone, piano).

## Requirements
- Library built: `cmake --build build --target gs1emu`
- Python: `numpy`, `cma`  (`pip3 install numpy cma`)
- Render CLI compiled (see below).

## Sample format
- Mono WAV, filename `NNN-VVV.wav` where `NNN` = MIDI note, `VVV` = velocity
  (e.g. `060-127.wav` = C4, velocity 127).
- As dry as possible (no reverb), known pitch, a handful of notes across the full
  range (for the keyboard EC curves), sustain long enough to analyze.

## Workflow
```bash
# 0) Compile the render CLI
c++ -O2 -std=c++17 -I emulator/src \
    tools/sample2gs1/gs1_render_params.cpp build/emulator/libgs1emu.a \
    -o tools/sample2gs1/out/gs1_render_params

# 1) Stage 1: features → base preset
python3 tools/sample2gs1/analyze.py samples/Steinway Steinway
#   → out/steinway_base.gs1   (seed for stage 2)
#   → out/gs1_Steinway_gen.h  (C++ PatchConsts snippet, STAGE 1 only)
#   → out/report.txt          (feature table + fit rationale)

# 2) Stage 2: CMA-ES refinement (45 iterations)
python3 tools/sample2gs1/optimize.py samples/Steinway Steinway 45
#   → out/steinway_opt.gs1     (optimized preset)

# 3) optimized .gs1 → paste-ready C++ PatchConsts header
python3 tools/sample2gs1/gs1_to_header.py \
    out/steinway_opt.gs1 gs1_SteinwayOpt "Steinway Opt" \
    ../../emulator/src/gs1_steinway_opt.h
```

> **Important:** `gs1_<Name>_gen.h` (from `analyze.py`) is only the **Stage 1**
> result. The optimized preset lives in `out/<name>_opt.gs1` — use
> `gs1_to_header.py` to turn it into the final header.

The Steinway example is already wired into the engine as program **37**
(`emulator/src/gs1_steinway_opt.h`, `gs1_SteinwayOpt`), so it is playable and
scrollable in the standalone. It was optimized with the double stack **on**, so
press `x` in the standalone for the full sound (base tone + hammer-attack layer).

## Files
| File | Purpose |
|------|---------|
| `analyze.py` | Stage 1: feature extraction + heuristic preset fit |
| `optimize.py` | Stage 2: CMA-ES analysis-by-synthesis (multi-note loss) |
| `gs1_render_params.cpp` | CLI: renders a `.gs1` text preset → raw float32 (stdout) |
| `render_note.cpp` | renders the `gs1_<Name>_gen.h` preset to WAV (A/B audition) |
| `gs1_to_header.py` | `.gs1` (e.g. optimized) → paste-ready C++ `PatchConsts` block |
| `out/` | generated artifacts (presets, WAVs, binaries) — not versioned |

## `.gs1` preset format
Text format, one line `key value…` per `PatchConsts` field (e.g. `Ratio 1 1 1 3`,
`M1EC <46 values>`, `DTEKbdScale 1.17`). Python writes it, the C++ CLI reads it —
so nothing needs to be recompiled per optimizer iteration.

## Loss (Stage 2)
Per eval note: log-magnitude spectrum (shared log-frequency grid) + envelope
(time-normalized) + **harmonics** (first 12 partial amplitudes relative to the
fundamental, strongly weighted → preserves low-note harmonic richness). Averaged
over C2/C3/C4/C5/C6. Sample-rate independent (engine 34687 Hz vs. samples at any rate).

## Engine parameters the tool uses
All existing `PatchConsts` fields plus two fine axes for good approximations:
- `BaseLevelDb[4]` — base-stack modulator brightness / timbre.
- `OutLevelDb` — per-preset output level.
- `DTEKbdScale` — upper keyboard decay-scaling factor (default 3.0); lower values
  give high notes a longer / more natural decay (important for acoustic pianos).
