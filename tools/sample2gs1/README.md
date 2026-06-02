# sample2gs1 — Sample → GS1-Preset (Reverse-Engineering)

Erzeugt aus einer Sample-Bibliothek eines echten Instruments (mono WAV) einen
GS1-Double-Stack-Preset, der das Instrument im GS1-Charakter annähert. Zwei
Stufen:

1. **Stufe 1 — heuristischer Mapper** (`analyze.py`): extrahiert pro Note
   Features (Grundton, Amplituden-Hüllkurve, Helligkeit, Partial-Struktur) und
   leitet daraus direkt einen Preset ab. Schnell, interpretierbar, datengetrieben.
2. **Stufe 2 — Analysis-by-Synthesis** (`optimize.py`): rendert den Preset über
   `libgs1emu` und verfeinert ihn mit **CMA-ES** gegen die Ziel-Samples
   (Spektral- + Hüllkurven- + Harmonik-Loss über mehrere Noten). Findet den
   besten Kompromiss *innerhalb* der Engine-Eigenheiten und nutzt
   Parameter-Wechselwirkungen, die die Heuristik nicht sieht.

> Ehrliche Erwartung: eine erkennbare, GS1-gefärbte Annäherung — kein 1:1-Klon.
> Reine 4-Operator-FM kann Hammergeräusch/Inharmonizität akustischer Klänge nur
> andeuten. Für harmonische Klänge (EP, Orgel, Vibraphon, Klavier) gut.

## Voraussetzungen
- Lib gebaut: `cmake --build build --target gs1emu`
- Python: `numpy`, `cma`  (`pip3 install numpy cma`)
- Render-CLI gebaut (siehe unten).

## Sample-Format
- Mono WAV, Dateiname `NNN-VVV.wav` mit `NNN` = MIDI-Note, `VVV` = Velocity
  (z.B. `060-127.wav` = C4, Velocity 127).
- Möglichst trocken (kein Reverb), bekannte Tonhöhe, ein paar Noten über den
  ganzen Umfang (für die Keyboard-EC-Kurven), Sustain lang genug zur Analyse.

## Workflow
```bash
# 0) Render-CLI kompilieren
c++ -O2 -std=c++17 -I emulator/src \
    tools/sample2gs1/gs1_render_params.cpp build/emulator/libgs1emu.a \
    -o tools/sample2gs1/out/gs1_render_params

# 1) Stufe 1: Features → Base-Preset
python3 tools/sample2gs1/analyze.py samples/Steinway Steinway
#   → out/steinway_base.gs1   (Seed für Stufe 2)
#   → out/gs1_Steinway_gen.h  (C++ PatchConsts-Snippet, einfügefertig)
#   → out/report.txt          (Feature-Tabelle + Fit-Begründung)

# 2) Stufe 2: CMA-ES-Feinschliff (45 Iterationen)
python3 tools/sample2gs1/optimize.py samples/Steinway Steinway 45
#   → out/steinway_opt.gs1     (optimiertes Preset)
```

## Dateien
| Datei | Zweck |
|-------|-------|
| `analyze.py` | Stufe 1: Feature-Extraktion + heuristischer Preset-Fit |
| `optimize.py` | Stufe 2: CMA-ES Analysis-by-Synthesis (Multi-Noten-Loss) |
| `gs1_render_params.cpp` | CLI: rendert ein `.gs1`-Textpreset → rohe float32 (stdout) |
| `render_note.cpp` | rendert den `gs1_<Name>_gen.h`-Preset zu WAV (A/B-Audition) |
| `out/` | generierte Artefakte (preset, WAVs, Binärdateien) — nicht versioniert |

## `.gs1`-Preset-Format
Textformat, eine Zeile `Schlüssel Wert…` pro `PatchConsts`-Feld (z.B.
`Ratio 1 1 1 3`, `M1EC <46 Werte>`, `DTEKbdScale 1.17`). Python schreibt es,
die C++-CLI liest es — so muss pro Optimierungs-Iteration nichts neu kompiliert
werden.

## Loss (Stufe 2)
Pro Eval-Note: Log-Magnitude-Spektrum (gemeinsames Log-Frequenz-Grid) +
Hüllkurve (zeitnormiert) + **Harmonik** (erste 12 Partialamplituden relativ zum
Grundton, stark gewichtet → erhält die Tiefen-Obertonfülle). Gemittelt über
C2/C3/C4/C5/C6. SR-unabhängig (Engine 34687 Hz vs. Samples beliebig).

## Engine-Parameter, die das Tool nutzt
Alle existierenden `PatchConsts`-Felder plus zwei feine Achsen für gute
Annäherungen:
- `BaseLevelDb[4]` — Modulator-Helligkeit / Timbre des Basis-Stacks.
- `OutLevelDb` — Preset-Ausgangspegel.
- `DTEKbdScale` — oberer Keyboard-Decay-Faktor (default 3.0); kleiner = hohe
  Töne klingen länger/natürlicher aus (wichtig für akustische Klaviere).
