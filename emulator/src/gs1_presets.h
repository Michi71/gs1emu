#pragma once

#include "patchdata.h"

// ============================================================
// Yamaha GS1 — 16 Factory Presets
// ============================================================
// Operator-Reihenfolge: C1=0, C2=1, M1=2, M2=3
//
// EC-Arrays (46 Werte):
//   [0]    = Basis-Level (Untergrenze des Keyboard-Scalings)
//   [1]    = Referenz-Level (Obergrenze)
//   [2..45]= 44 Keyboard-Zonen (je 2 Halbtöne, tiefste bis höchste Lage)
//   1.0 = voller Pegel, 0.0 = stumm
//
// ATE[4]       : Attack-Rate pro Operator (höher = schneller)
// DTE1Scaling  : C2-Decay skaliert von 0.5 (tief) bis DTE1Scaling (hoch)
// ============================================================

// --- (1) A-1  Harpsichord I ---
// Klar, hell, gezupft. Staccato, kurzer Sustain.
// M1/M2 mit hohen Ratios (8x, 6x) erzeugen den typischen Cembalo-Klang.
const PatchConsts gs1_HarpsichordI = {
    // -------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // -------------------------
    // C1 = 1.0  → Grundton
    // C2 = 2.0  → Oktave darüber (GS1 typisch)
    // M1 = 5.0  → moderate Obertöne, nicht zu hell
    // M2 = 6.0  → Attack‑Biss, aber nicht metallisch
    {1.0f, 2.0f, 5.0f, 6.0f},

    // -------------------------
    // 2) Detune (C1, C2, M1, M2)
    // -------------------------
    // GS1 Harpsichord war fast un‑detuned → nur leichte Breite
    {0, 2, 1, 3},

    // -------------------------
    // 3) C1EC — Grundton‑Carrier
    // -------------------------
    // Voll über die Tastatur, GS1 typisch
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // -------------------------
    // 4) C2EC — Oktave‑Carrier
    // -------------------------
    // GS1 Harpsichord verliert im Hochregister Energie
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      0.9,0.9,0.8,0.8,0.7,0.7,0.6,0.5,0.4,0.3,
      0.2,0.2,0.1,0.1,0.05,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 5) M1EC — Modulator 5×
    // -------------------------
    // Weniger Höhen → weniger metallisch
    { 0.0f, 1.0f,
      0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.3,0.2,0.2,
      0.1,0.1,0.05,0.05,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 6) M2EC — Modulator 6× (Attack‑Biss)
    // -------------------------
    // GS1 Harpsichord hat nur einen kurzen, hellen Impuls
    { 0.0f, 1.0f,
      0.5,0.6,0.7,0.8,1.0,1.0,1.0,1.0,1.0,1.0,
      0.9,0.9,0.8,0.8,0.7,0.6,0.5,0.4,0.3,0.3,
      0.2,0.2,0.1,0.1,0.05,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 7) Envelope Attack Times (ms)
    // -------------------------
    // GS1 Harpsichord = extrem perkussiv
    {2000.0f, 2000.0f, 2000.0f, 2000.0f},

    // -------------------------
    // 8) DTE1Scaling (für C2 Decay)
    // -------------------------
    // GS1 typisch: hoher Tonbereich fällt schneller ab
    6.0f
};

// --- (2) A-2  Harpsichord III ---
// Heller als Harpsichord I. Helligkeit variiert mit Anschlagsschnelligkeit.
// Höhere Modulator-Ratios (12x, 9x) → mehr Obertöne.
const PatchConsts gs1_HarpsichordIII = {
    // -------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // -------------------------
    // GS1 A‑2 war heller als A‑1 → höhere Modulator‑Ratios
    {1.0f, 2.0f, 8.0f, 6.0f},

    // -------------------------
    // 2) Detune (C1, C2, M1, M2)
    // -------------------------
    // Leichtes Schimmern, aber kein DX7‑Klingeln
    {0, 4, 3, 6},

    // -------------------------
    // 3) C1EC — Grundton‑Carrier
    // -------------------------
    // Voll über die Tastatur
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // -------------------------
    // 4) C2EC — Oktave‑Carrier
    // -------------------------
    // Heller als A‑1, aber GS1‑typisch abfallend
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      0.95,0.95,0.9,0.85,0.8,0.75,0.7,0.6,0.5,0.4,
      0.3,0.2,0.15,0.1,0.05,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 5) M1EC — Modulator 8×
    // -------------------------
    // Heller als A‑1, aber nicht DX7‑hart
    { 0.0f, 1.0f,
      0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      0.9,0.85,0.8,0.75,0.7,0.6,0.5,0.4,0.3,0.25,
      0.2,0.15,0.1,0.05,0.05,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 6) M2EC — Modulator 6× (Attack‑Biss)
    // -------------------------
    // Kürzerer, hellerer Impuls als A‑1
    { 0.0f, 1.0f,
      0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,1.0,1.0,
      0.95,0.9,0.85,0.8,0.7,0.6,0.5,0.4,0.3,0.25,
      0.2,0.15,0.1,0.05,0.05,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // -------------------------
    // 7) Envelope Attack Times (ms)
    // -------------------------
    // A‑2 war heller, aber NICHT langsamer als A‑1
    {2000.0f, 2000.0f, 2000.0f, 2000.0f},

    // -------------------------
    // 8) DTE1Scaling (C2‑Decay)
    // -------------------------
    // A‑2 fällt im Hochregister schneller ab
    7.0f
};


// --- (3) A-4  Clavichord II ---
// Abgeleitet aus KaoX "KY GS1 Clavichord II NU" von Nori Ubukata.
// KaoX-Routing: OP2->OP1(0.85), OP4->OP3(0.365), OP4->OP2(0.365 Cross!)
// Cross-Modulation M2->M1 erzeugt den typischen Clavichord-Buzz.
const PatchConsts gs1_ClavichordII = {
    // Ratios: C1=1x(Grund), C2=1x(Grund), M1=3x, M2=4x
    // KaoX: OP1=0.5(1x), OP3=0.6(2x), OP2=0.65(3x), OP4=0.725(4x)
    // C2 auf 1x reduziert (statt KaoX 2x) — CROSSMOD braucht Headroom
    {1.0f, 1.0f, 3.0f, 4.0f},

    // Detune: KaoX COARSE_TUNE_OSCFM2=0.503 -> leichtes Detune auf Stack 2
    {0, 3, 0, 0},

    // C1EC -- Carrier Grundton (KaoX OP1: KEYB_SCALE_ON=0 -> flach)
    { 0.0f, 0.85f,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },

    // C2EC -- Carrier Oktave (KaoX OP3: KEYB_SCALE_ON=0 -> flach)
    { 0.0f, 0.80f,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },

    // M1EC -- Modulator 3x (KaoX OP2: KEYB=0.5=neutral, SCALE_ON=0.75 -> flach!)
    { 0.0f, 0.60f,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },

    // M2EC -- Modulator 4x (Mittelwert KaoX OP4 KEYB=0.425 + OP8 KEYB=0.34)
    // Moderat negatives Keyboard-Scaling: weniger Buzz in hohen Lagen
    { 0.0f, 0.55f,
      1.00,0.98,0.95,0.93,0.91,0.89,0.86,0.84,0.82,0.80,0.79,
      0.77,0.75,0.73,0.71,0.70,0.68,0.67,0.65,0.64,0.62,0.61,
      0.59,0.58,0.57,0.55,0.54,0.53,0.52,0.51,0.50,0.48,0.47,
      0.46,0.45,0.44,0.43,0.42,0.42,0.41,0.40,0.39,0.38,0.37 },

    // ATE: Alle KaoX-Attacks=0.0 (instant) -> schneller Anschlag
    {4000.0f, 3500.0f, 4500.0f, 4000.0f},

    // DTE1Scaling
    4.0f,

    // DTE: KaoX DECAY C1=0.34, C2=0.305, M1=0.36, M2=0.64(langsam!)
    // M2 zerfaellt langsam -> Buzz bleibt laenger erhalten
    {5, 6, 4, 2},

    // RTE: KaoX RELEASE alle 0.5 -> moderates Release
    {60, 60, 60, 60},

    {0, 0, 0, 0},             // IL
    {0, 0, 0, 0},             // SL
    {3, 0},                   // FMmode: Stack1=CROSSMOD (M2->M1 Buzz!), Stack2=NORM
    "Clavichord II"
};

// --- (4) B-1  Vibraphone ---
// Glocken-Mallet-Klang. Inharmonische Ratio (C2=3.5, M2=3.5) erzeugt
// den metallischen Schimmer. Langsamer Sustain, Vibrato-Pedal.
const PatchConsts gs1_Vibraphone = {
    // ============================================================
    // Abgeleitet aus KaoX "PC GS1 Vibraphones NU" von Nori Ubukata
    // KaoX nutzt 8 Ops (2x4), wir mappen die Primaerpaare:
    //   C1=OP1, M1=OP2, C2=OP5, M2=OP6
    // KaoX OP4/OP8 (Self-Feedback-Modulatoren) haben kein Aequivalent.
    // ============================================================

    // Ratios: Carrier=1x, Modulator=4x (KaoX FREQ_RATIO 0.725 -> 4. Harmonische)
    {1.0f, 1.0f, 8.0f, 8.0f},

    // Detune: KaoX hat nahezu 0 -- nur OP6 hat 0.5075 (~1 cent)
    {0, 0, 10, 10},

    // C1EC -- Carrier Stack 1 (KaoX OP1: KEYB_SCALE_ON=0 -> flach)
    { 0.0f, 0.95f,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },

    // C2EC -- Carrier Stack 2 (KaoX OP5: KEYB_SCALE_ON=0 -> flach)
    { 0.0f, 0.95f,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },

    // M1EC -- Modulator Stack 1 (KaoX OP2: KEYB=0.245, SCALE_ON=0.75)
    // Starkes negatives Keyboard-Scaling: Modulation faellt nach oben ab
    { 0.0f, 0.85f,
      1.0, 1.0, 1.0, 0.98,0.95,0.92,0.88,0.84,0.80,0.75,0.70,0.65,
      0.60,0.55,0.50,0.45,0.40,0.36,0.32,0.28,0.25,0.22,0.20,0.18,
      0.16,0.14,0.13,0.12,0.11,0.10,0.10,0.09,0.09,0.08,0.08,0.08,
      0.07,0.07,0.07,0.07,0.07,0.07,0.07,0.07 },

    // M2EC -- Modulator Stack 2 (KaoX OP6: KEYB=0.245, SCALE_ON=0.75)
    { 0.0f, 0.85f,
      1.0, 1.0, 1.0, 0.98,0.95,0.92,0.88,0.84,0.80,0.75,0.70,0.65,
      0.60,0.55,0.50,0.45,0.40,0.36,0.32,0.28,0.25,0.22,0.20,0.18,
      0.16,0.14,0.13,0.12,0.11,0.10,0.10,0.09,0.09,0.08,0.08,0.08,
      0.07,0.07,0.07,0.07,0.07,0.07,0.07,0.07 },

    // ATE: Alle KaoX-Attacks=0.0 (instant) -> sehr schnell
    {3500.0f, 3000.0f, 4000.0f, 4000.0f},

    // DTE1Scaling
    3.5f,

    // DTE: Aus KaoX DECAY (OP1=0.25, OP2=0.345, OP5=0.345, OP6=0.395)
    {6, 4, 4, 3},

    // RTE: Aus KaoX RELEASE (OP1=0.41, OP2=0.585, OP5=0.435, OP6=0.585)
    {50, 60, 50, 60},

    {0, 0, 0, 0},             // IL
    {0, 0, 0, 0},             // SL
    {3, 0},                   // FMmode: NORM/NORM
    "Vibraphone"
};

// --- (5) B-2  Celeste ---
// Süßer Glockenklang, obere Oktaven. C2/M2 bei Ratio 5/5.5 → spezifischer
// inharmonischer Glanz. Heller und höher als Vibraphone.
const PatchConsts gs1_Celeste = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-Celeste: süßer, glockiger Obertonschimmer
    // C2 = 5.0  → heller Oberton
    // M2 = 5.5  → inharmonischer Glanz
    {1.0f, 5.0f, 1.0f, 5.5f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schimmern, aber kein Chorus
    {0, 6, 0, 8},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, nicht zu hell
    { 0.0f, 1.0f,
      0.85,0.85,0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,0.95,0.9,0.85,0.8,0.75,0.7,
      0.6,0.5,0.4,0.3,0.2,0.15,0.1,0.1,0.05,0.05,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 4) C2EC — heller Obertonträger
    // ---------------------------------------------------------
    // GS1-Celeste ist NICHT extrem hell → sanfter Abfall
    { 0.0f, 0.7f,
      0.35,0.4,0.45,0.5,0.55,0.6,0.6,0.6,0.6,0.6,
      0.55,0.55,0.5,0.45,0.4,0.35,0.3,0.25,0.2,0.15,
      0.1,0.1,0.05,0.05,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — weicher Modulator (1×)
    // ---------------------------------------------------------
    // Warm, sweet, nicht metallisch
    { 0.0f, 0.5f,
      0.3,0.35,0.4,0.45,0.5,0.5,0.5,0.5,0.5,0.45,
      0.4,0.35,0.3,0.25,0.2,0.15,0.1,0.1,0.05,0.05,
      0.05,0.05,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — inharmonischer Glanz (5.5×)
    // ---------------------------------------------------------
    // Attack-Peak, aber nicht so hart wie Vibraphone
    { 0.0f, 0.8f,
      0.4,0.45,0.5,0.55,0.6,0.65,0.7,0.7,0.65,0.6,
      0.55,0.5,0.45,0.4,0.35,0.3,0.25,0.2,0.15,0.1,
      0.1,0.05,0.05,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Celeste = weicher als Vibraphone, aber klarer Attack
    {3000.0f, 2500.0f, 3500.0f, 2500.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    3.0f
};


// --- (6) C-1  Acoustic Piano I ---
// Warmer Flügel-Sound (GS1-typisch "mellow"). Abgeleitet aus dem
// Original-Patch YAMAHA GS1.SYX #4 "GRAND PIAN" (DX7 Algo 16, Yamahas
// offizielle GS1->DX7 Übertragung).
//
// Übersetzung DX7-Topology → GS1-Topology (M1→C1 ∥ M2→C2):
//   DX7 OP1 (0.5x, lvl 99, KVS=0)  →  GS1 C1  (Body-Carrier)
//   DX7 OP2 (0.5x, lvl 84, KVS=3)  →  GS1 M1  (Body-Mod, lange Sub-Mod)
//   DX7 OP3 (1.0x, lvl 99/L1=79)   →  GS1 C2  (Percussive Carrier)
//   DX7 OP4 (5.0x, lvl 81, KVS=4)  →  GS1 M2  (Hammer-Bite Modulator)
// (OP5+OP6 mit Self-Feedback approximiert durch leichte M2-Detune.)
//
// Faktor ×2 auf alle Ratios kompensiert DX7-transpose +24 → die GS1-Tasten
// klingen in der gespielten Oktave statt 2 Oktaven tiefer.
const PatchConsts gs1_AcousticPianoI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1 GRAND PIAN — zwei parallele Stacks mit unterschiedlicher Funktion:
    //   Stack A (Body):  M1 (1×) → C1 (1×)   → warme, weiche Klangbasis
    //   Stack B (Hammer):M2 (6×) → C2 (2×)   → Oktave + Hammer-Bite
    //
    // M2=6× (statt 8×) reduziert die obertonreiche Helligkeit deutlich.
    // Kombiniert mit erhöhter M2-Detune simuliert das den "wood-hammer"
    // Charakter ähnlich OP4+OP5+OP6 Self-Feedback im DX7-Original.
    {1.0f, 2.0f, 1.0f, 4.0f},

    // ---------------------------------------------------------
    // 2) Detune (C1, C2, M1, M2) — cents
    // ---------------------------------------------------------
    // M2-Detune erhöht (3 → 9) für mehr Inharmonik im Attack — ersetzt
    // teilweise die OP5+OP6 Self-FB Modulation aus dem DX7-Original und
    // macht den Anschlag perkussiver/holziger statt orgelartig.
    {0, 4, 0, 14},

    // ---------------------------------------------------------
    // 3) C1EC — Body-Carrier (DX7 OP1: lvl 99, KVS=0, kein Keyboard-Scaling)
    // ---------------------------------------------------------
    // OP1 hatte KVS=0 (kein Velocity-Einfluss) und LSD=RSD=0 (kein
    // Keyboard-Scaling) → konstant voller Pegel über die gesamte Tastatur.
    // Das ist der "Fundament-Carrier", der unabhängig von Anschlagsstärke
    // jeder Note ihre Grundwärme gibt → ANTWORT auf "Body fehlt".
    //
    // In Kombination mit M1 (=1×) entsteht ein klassisches Klavier-Spektrum
    // mit starker 1./2./3. Harmonischer.
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 4) C2EC — Percussive Carrier (DX7 OP3: lvl 99, L1=79!, RSD=13 LIN-)
    // ---------------------------------------------------------
    // OP3 hat L1=79 statt 99 → max Peak ≈ 0.85 (10dB unter C1).
    // R2=96 (extrem schnelles Decay zu L2=65) → das ist der HAMMER-IMPULS
    // der die Attackphase prägt. Geschieht im Emulator über DTE1Scaling
    // (siehe Punkt 8).
    //
    // Wichtig: pow(zoneVal, 0.1)-Kurve im Emulator komprimiert lineare
    // Werte stark — selbst zoneVal=0.05 ergibt nur -8dB Dämpfung.
    // Deshalb nutzen wir EXPONENTIELLEN Abfall mit kleinen Werten am
    // oberen Ende für hörbaren Diskant-Cutoff (echtes Klavier).
    { 0.0f, 0.85f,
      1.00,1.00,1.00,1.00,1.00,1.00,0.95,0.90,0.85,0.80,
      0.75,0.70,0.65,0.60,0.55,0.50,0.45,0.40,0.35,0.30,
      0.25,0.20,0.15,0.12,0.10,0.08,0.06,0.05,0.04,0.03,
      0.02,0.015,0.01,0.008,0.006,0.005,0.004,0.003,0.002,0.0015,
      0.001,0.001,0.0005,0.0005 },

    // ---------------------------------------------------------
    // 5) M1EC — Body-Modulator (DX7 OP2: lvl 84, BP=46, RSD=21 LIN-)
    // ---------------------------------------------------------
    // OP2 erzeugt am C1 die typischen Klavier-Obertöne (2./3./4. Harmonik).
    // Peak 0.42 (reduziert von 0.55) — weniger sustaining Mod-Helligkeit,
    // da Body-Stack im langen Sustain dominiert (siehe DT-Analyse: M1
    // klingt ~25s aus → muss schwächer sein damit es nicht orgel-artig
    // klingt).
    //
    // BP=46 (≈MIDI 67 = G4) → links flach voll, rechts ab Index ~23
    // exponentiell abfallend für hörbaren Höhen-Cutoff trotz pow(0.1)-Kurve.
    { 0.0f, 0.42f,
      1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,
      1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,
      1.00,1.00,0.95,0.85,0.70,0.55,0.40,0.28,0.18,0.12,
      0.08,0.05,0.03,0.02,0.01,0.006,0.003,0.002,0.001,0.0008,
      0.0005,0.0003,0.0002,0.0001 },

    // ---------------------------------------------------------
    // 6) M2EC — Hammer-Modulator (DX7 OP4: lvl 81, BP=0, RSD=17, KVS=4)
    // ---------------------------------------------------------
    // Klassischer Piano-Hammer-Bite mit Ratio 6× — weniger metallisch als
    // 8× (kein DX7-Tine-Klingeln) und nähert sich dem "wood-thump" eines
    // echten Klavier-Hammers an.
    //
    // Peak 0.65 (reduziert von 0.75) — gleicher Grund wie M1: M2 klingt
    // architektur-bedingt ~25s lang aus. Wenn Peak zu hoch, wird der
    // Modulator zur Sustain-Helligkeit (= Orgel im Attack). Bei 0.65 ist
    // der Anfangs-Bite noch deutlich, aber das Sustain bleibt warm.
    //
    // STRATEGIE gegen "Orgel im Attack" ohne Code-Änderung: Wir
    // konzentrieren den Bite in der Bass-/Tenor-Lage (volle Werte bis
    // Index ~12), dann exponentieller Abfall durch Mittellage.
    // Werte bewusst < 0.01 im Diskant — dort soll der Klang glockig-warm
    // sein, kein FM-Bite.
    { 0.0f, 0.65f,
      1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,0.95,0.90,
      0.80,0.70,0.60,0.50,0.40,0.30,0.22,0.16,0.12,0.08,
      0.05,0.035,0.025,0.018,0.012,0.008,0.005,0.003,0.002,0.0015,
      0.001,0.0008,0.0005,0.0003,0.0002,0.0001,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (Inkrement-Rate pro Sample)
    // ---------------------------------------------------------
    // ATEs deutlich erhöht für knackigeren, perkussiveren Anschlag
    // ("nicht orgel im attack" Feedback).
    //   3000 → 4500 : Body-OPs ~3.4ms Attack im Bass (war 5ms)
    //   4500 → 9000 : Hammer-Mod ~1.7ms Attack im Bass (war 3.4ms)
    // M2 muss schneller einschwingen als die Body-OPs damit die FM-
    // Modulation BEIM Anschlag-Peak (nicht danach) am stärksten ist —
    // das gibt den knackigen Initial-Bite.
    {4500.0f, 4500.0f, 4500.0f, 9000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling (Decay-Skalierung für C2 über Tastatur)
    // ---------------------------------------------------------
    // Erhöht von 8 → 14 für noch schnelleres C2-Decay im Diskant:
    //   Bass  (KNOTE=0) : DT[1] = DTE[1] × 0.5 → ~30 s
    //   Tenor (KNOTE=44): DT[1] = DTE[1] × ~7 → ~2.2 s
    //   Diskant (KNOTE=87): DT[1] = DTE[1] × 14 → ~1.1 s
    // Entspricht der realen Klaviersaiten-Physik.
    14.0f,

    // ---------------------------------------------------------
    // 9) DTE[4] — Pro-Operator-Decay-Skalierung
    // ---------------------------------------------------------
    // ECHTER HAMMER-BITE durch schnelles M2-Decay:
    //   DTE[0] = 2.0  → C1 Body-Carrier, langes Decay (Sustain)
    //   DTE[1] = 2.0  → C2 Hammer-Carrier (mit DTE1Scaling=14 oben)
    //   DTE[2] = 1.5  → M1 Body-Modulator, leicht schneller als default
    //                   (1.0) → reduziert sustaining Mod-Helligkeit
    //   DTE[3] = 40.0 → M2 Hammer-Modulator EXTREM schnell (40× default!)
    //
    // M2-Decay-Zeiten mit DTE[3]=40:
    //   Bass  (KNOTE=0)  : DT = 40×0.5 = 20    → ~1.5 s
    //   Tenor (KNOTE=44) : DT = 40×1.625 = 65  → ~0.46 s
    //   Diskant (KNOTE=87): DT = 40×3 = 120     → ~0.25 s
    // Das ist die realistische Klavier-Hammer-Decay-Charakteristik: nach
    // <500ms ist der Modulator weg, übrig bleibt der saubere Carrier-Klang.
    // Endlich ECHTER Hammer-Bite statt "Orgel im Attack".
    {2, 2, 1, 60}
};


// --- (7) C-4  Electric Piano I ---
// Rhodes-artiger E-Piano-Sound. Klassischer FM-Tine-Sound (14x Modulator).
// Mit Sustain, Vibrato oder Tremolo; Aftertouch-Variationen.
const PatchConsts gs1_ElectricPianoI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-EP: zwei Grundton-Carrier + zwei Modulatoren
    // M1 = 1× → weicher Body
    // M2 = 7× → Tine-Glanz, aber nicht DX7-scharf
    //{1.0f, 1.0f, 1.0f, 7.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schimmern, aber kein Rhodes-Chorus
    //{0, 8, 2, 10},
    {0, 12, 3, 12},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, leicht abfallend
    { 0.0f, 0.65f,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      0.9,0.9,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8 },

    // ---------------------------------------------------------
    // 4) C2EC — zweiter Grundtonträger
    // ---------------------------------------------------------
    // Etwas heller, aber GS1-dumpf
    { 0.0f, 0.35f,
      0.2,0.3,0.3,0.4,0.5,0.5,0.6,0.6,0.7,0.7,
      0.8,0.8,0.9,0.9,1.0,1.0,1.0,1.0,0.9,0.9,
      0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.2,0.1,0.1,
      0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,
      0.1,0.1,0.1,0.1 },

    // ---------------------------------------------------------
    // 5) M1EC — weicher Body-Modulator (1×)
    // ---------------------------------------------------------
    // Warm, Rhodes-artig, aber nicht metallisch
    { 0.0f, 0.7f,
      0.9,0.9,0.6,0.6,0.6,0.6,0.7,0.7,0.7,0.8,
      0.8,0.9,0.9,1.0,0.9,0.8,0.8,0.7,0.7,0.7,
      0.7,0.6,0.6,0.5,0.5,0.4,0.4,0.3,0.3,0.2,
      0.2,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.05,0.05,
      0.05,0.05,0.05,0.05 },

    // ---------------------------------------------------------
    // 6) M2EC — Tine-Glanz (7×)
    // ---------------------------------------------------------
    // Kürzer als DX7, weicher, GS1-typisch
    { 0.0f, 0.55f,
      0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,0.9,0.9,0.8,0.8,0.7,0.7,0.7,0.6,
      0.6,0.5,0.5,0.5,0.5,0.5,0.5,0.4,0.3,0.2,
      0.1,0.1,0.1,0.05,0.01,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // GS1-typisch: weicher Attack, kein DX7-Klick
    {2000.0f, 2000.0f, 4400.0f, 4400.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    3.0f,

    {2, 2, 1, 1},       // DTE: Perkussives Ausklingen
    {250, 250, 250, 250}, // RTE: Schnelles Release (typisch E-Piano)
    {0, 0, 0, 0},       // IL
    {0, 0, 0, 0},       // SL: Piano-Modus (klingt immer aus)
    {0, 0}              // FMmode: Beides NORM (keine Crossmod, kein Feedback)
};


// --- (8) C-6  Electric Piano III ---
const PatchConsts gs1_ElectricPianoIII = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-EP III: heller als EP I, aber nicht DX7-scharf
    // C2 = 7× → obertonreicher Body
    // M2 = 15× → Attack-Glanz
    {1.0f, 7.0f, 1.0f, 15.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schimmern, aber kein Chorus
    {0, 0, 3, 2},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, leicht abfallend
    { 0.0f, 1.0f,
      0.5,0.5,0.5,0.5,0.6,0.6,0.7,0.7,0.8,0.8,
      0.9,0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      0.9,0.9,0.9,0.8,0.8,0.7,0.7,0.6,0.6,0.5,
      0.5,0.4,0.4,0.4 },

    // ---------------------------------------------------------
    // 4) C2EC — obertonreicher Body (7×)
    // ---------------------------------------------------------
    // GS1-typisch: hell, aber nicht DX7-scharf
    { 0.0f, 0.06f,
      0.6,0.6,0.6,0.6,0.6,0.6,0.7,0.7,0.7,0.7,
      0.7,0.7,0.6,0.6,0.5,0.5,0.4,0.4,0.4,0.4,
      0.4,0.4,0.4,0.3,0.3,0.3,0.3,0.3,0.2,0.2,
      0.2,0.2,0.1,0.1,0.1,0.1,0.1,0.05,0.05,0.05,
      0.05,0.05,0.05,0.05 },

    // ---------------------------------------------------------
    // 5) M1EC — weicher Modulator (1×)
    // ---------------------------------------------------------
    // Warm, Rhodes-artig, aber nicht metallisch
    { 0.0f, 0.27f,
      1.0,1.0,1.0,1.0,1.0,1.0,0.9,0.9,0.8,0.8,
      0.7,0.7,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.5,0.5,0.5,0.4,0.4,0.3,0.3,0.3,0.2,
      0.2,0.1,0.1,0.1,0.05,0.05,0.05,0.01,0.01,0.01,
      0.01,0.01,0.01,0.01 },

    // ---------------------------------------------------------
    // 6) M2EC — Attack-Glanz (15×)
    // ---------------------------------------------------------
    // Sehr kurz, sehr hell → GS1-Bell-Glanz
    { 0.0f, 0.3f,
      1.0,1.0,1.0,1.0,1.0,0.9,0.9,0.8,0.8,0.7,
      0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.05,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // EP III = schneller Attack als EP I
    {1800.0f, 1800.0f, 1500.0f, 2000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    15.0f
};


// --- (9) D-1  String I ---
// Solo-Streicher. Langsamer Attack (kleines ATE!), leicht verstimmt für
// natürliches Vibrato-Schweben. Vibrato und Lautstärke-Pedal empfohlen.
const PatchConsts gs1_StringI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-Strings: warm, leicht nasal, nicht metallisch
    // M1 = 3× → weiche Bogen-Obertöne
    // M2 = 2× → warme zweite Harmonische
    {1.0f, 1.0f, 3.0f, 2.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schweben, aber kein Chorus
    {0, 6, -3, 4},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, leicht abfallend
    { 0.0f, 1.0f,
      0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,
      0.15,0.1,0.05,0.0 },

    // ---------------------------------------------------------
    // 4) C2EC — zweiter Layer
    // ---------------------------------------------------------
    // Etwas heller, aber GS1-dumpf
    { 0.0f, 0.9f,
      0.8,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.9,0.8,0.8,0.7,0.7,
      0.6,0.5,0.4,0.3,0.2,0.2,0.1,0.1,0.05,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — Bogen-Obertöne (3×)
    // ---------------------------------------------------------
    // Weich, warm, leicht nasal
    { 0.0f, 0.6f,
      0.4,0.5,0.5,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.5,0.5,0.5,0.4,0.4,0.3,0.3,
      0.3,0.2,0.2,0.2,0.1,0.1,0.1,0.05,0.05,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — warme zweite Harmonische (2×)
    // ---------------------------------------------------------
    // Gibt dem Klang Körper und Wärme
    { 0.0f, 0.5f,
      0.3,0.4,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,
      0.5,0.4,0.4,0.4,0.3,0.3,0.3,0.2,0.2,0.1,
      0.1,0.1,0.05,0.05,0.02,0.02,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // GS1-Strings: langsamer Attack, weicher Bogen
    {300.0f, 250.0f, 400.0f, 400.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.0f
};


// --- (10) D-3  String Ensemble I ---
// Streichersatz. Tonfarbe variiert mit Anschlagsschnelligkeit.
// C2 eine Oktave höher (Ratio=2), reichere Modulation.
const PatchConsts gs1_StringEnsembleI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Ensemble = breiter, obertonreicher als String I
    // C2 = 2× → Oktave
    // M1 = 3× → Ensemble-Schimmer
    // M2 = 4× → höhere Harmonische für Breite
    {1.0f, 2.0f, 3.0f, 4.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schweben, aber kein Chorus
    {0, 10, -5, 8},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, leicht abfallend, GS1-dumpf
    { 0.0f, 1.0f,
      0.8,0.9,0.9,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1,
      0.1,0.05,0.0,0.0 },

    // ---------------------------------------------------------
    // 4) C2EC — Oktavlayer
    // ---------------------------------------------------------
    // Heller, aber GS1-typisch abfallend
    { 0.0f, 0.8f,
      0.6,0.7,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.7,0.7,0.7,0.6,0.6,0.5,0.5,
      0.4,0.4,0.3,0.3,0.2,0.2,0.1,0.1,0.05,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — Ensemble-Schimmer (3×)
    // ---------------------------------------------------------
    // Weich, breit, leicht chorartig
    { 0.0f, 0.7f,
      0.5,0.6,0.6,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.6,0.6,0.6,0.5,0.5,0.4,0.4,0.3,0.3,0.2,
      0.2,0.1,0.1,0.05,0.05,0.02,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — höhere Harmonische (4×)
    // ---------------------------------------------------------
    // Gibt Breite und „Chor“-Charakter
    { 0.0f, 0.6f,
      0.4,0.5,0.5,0.6,0.6,0.6,0.6,0.5,0.5,0.5,
      0.4,0.4,0.3,0.3,0.2,0.2,0.1,0.1,0.05,0.05,
      0.02,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Ensemble = langsamer Attack als String I
    {400.0f, 300.0f, 500.0f, 500.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.2f
};


// --- (11) E-1  Brass I ---
// Hornartiger Blechbläser. Modulator Ratio 1:1 und 2:1 erzeugt
// FM-typischen Bläser-Buzz. Tonfarbe über Anschlagsgeschwindigkeit variierbar.
const PatchConsts gs1_BrassI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-Brass: warm, leicht nasal, nicht metallisch
    // M1 = 1× → Grund-Buzz
    // M2 = 2× → Brass-Brillanz
    {1.0f, 1.0f, 1.0f, 2.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schimmern, aber kein Chorus
    {0, 4, 0, 6},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, leicht nasal, GS1-dumpf
    { 0.0f, 1.0f,
      0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0 },

    // ---------------------------------------------------------
    // 4) C2EC — zweiter Layer
    // ---------------------------------------------------------
    // Etwas heller, aber GS1-typisch abfallend
    { 0.0f, 0.9f,
      0.5,0.6,0.7,0.8,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.8,0.8,0.7,0.7,0.6,
      0.6,0.5,0.4,0.3,0.2,0.2,0.1,0.1,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — Brass-Buzz (1×)
    // ---------------------------------------------------------
    // Weich, warm, leicht nasal
    { 0.0f, 0.8f,
      0.4,0.5,0.6,0.7,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8,0.8,0.7,0.7,0.6,0.6,0.5,
      0.4,0.3,0.2,0.1,0.1,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — Brass-Brillanz (2×)
    // ---------------------------------------------------------
    // Gibt dem Klang Präsenz, aber ohne DX7-Schärfe
    { 0.0f, 0.7f,
      0.3,0.4,0.5,0.6,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.6,0.6,0.6,0.5,0.5,0.4,0.4,
      0.3,0.3,0.2,0.2,0.1,0.1,0.05,0.05,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Brass = schneller Attack, aber nicht „knallend“
    {1000.0f, 800.0f, 2000.0f, 2000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.5f
};


// --- (12) E-2  Brass II ---
// Trompeten-artiger Bläser, heller als Brass I. Modulator 2x/3x
// erhöht den Oberton-Anteil für mehr Schärfe und Brillanz.
const PatchConsts gs1_BrassII = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Brass II = heller, schärfer als Brass I
    // M1 = 2× → zweite Harmonische = Brass-Grundcharakter
    // M2 = 3× → Brillanz, aber nicht metallisch
    {1.0f, 1.0f, 2.0f, 3.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Leichtes Schimmern, aber kein Chorus
    {0, 6, 0, 8},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, aber heller als Brass I
    { 0.0f, 1.0f,
      0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0 },

    // ---------------------------------------------------------
    // 4) C2EC — heller Layer
    // ---------------------------------------------------------
    // GS1-typisch: hell, aber abfallend
    { 0.0f, 1.0f,
      0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,0.9,0.8,0.7,0.6,0.5,0.4,
      0.3,0.2,0.1,0.1,0.05,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — Brass-Grundcharakter (2×)
    // ---------------------------------------------------------
    // Gibt den typischen „Horn-Buzz“
    { 0.0f, 0.9f,
      0.5,0.6,0.7,0.8,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.8,0.8,0.7,0.7,0.6,
      0.5,0.4,0.3,0.2,0.1,0.1,0.05,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — Brillanz (3×)
    // ---------------------------------------------------------
    // Heller als M1, aber GS1-dumpf im Hochregister
    { 0.0f, 0.8f,
      0.4,0.5,0.6,0.7,0.8,0.8,0.8,0.7,0.7,0.6,
      0.6,0.5,0.5,0.4,0.3,0.3,0.2,0.2,0.1,0.1,
      0.05,0.05,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Brass II = schneller Attack als Brass I
    {1500.0f, 1000.0f, 2500.0f, 2500.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.8f
};


// --- (13) E-6  Synth Brass III ---
// Standard Synth-Brass. Klassischer Analogsynthesizer-Charakter mit
// FM-Modulation. Detune für Ensemble-Breite.
const PatchConsts gs1_SynthBrassIII = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Synth Brass = heller, synthetischer als Brass I/II
    // M1 = 2× → Grund-Sägezahn
    // M2 = 3× → Brillanz
    {1.0f, 1.0f, 2.0f, 3.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Breite, aber kein Chorus
    {0, 10, 4, 12},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Warm, aber heller als Brass I/II
    { 0.0f, 1.0f,
      0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0 },

    // ---------------------------------------------------------
    // 4) C2EC — heller Layer
    // ---------------------------------------------------------
    // GS1-typisch: hell, aber abfallend
    { 0.0f, 1.0f,
      0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.9,0.8,0.7,
      0.6,0.5,0.4,0.3,0.2,0.1,0.05,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — Synth-Sägezahn (2×)
    // ---------------------------------------------------------
    // Gibt den typischen Synth-Brass-Körper
    { 0.0f, 1.0f,
      0.5,0.6,0.7,0.8,0.9,1.0,1.0,1.0,1.0,1.0,
      1.0,1.0,1.0,1.0,1.0,0.9,0.8,0.7,0.6,0.5,
      0.4,0.3,0.2,0.1,0.1,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — Brillanz (3×)
    // ---------------------------------------------------------
    // Heller, aber GS1-dumpf im Hochregister
    { 0.0f, 0.9f,
      0.4,0.5,0.6,0.7,0.8,0.9,0.9,0.8,0.7,0.6,
      0.5,0.4,0.3,0.2,0.1,0.1,0.05,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Synth Brass = schneller Attack, aber nicht „knallend“
    {1500.0f, 1200.0f, 2500.0f, 2500.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    2.0f
};


// --- (14) F-1  Electronic Organ I ---
// Hammond-artiger Orgel-Klang. Harmonische Ratios (1:2:1:2).
// Sofortiger Attack/Release. Ensemble-Effekt = Rotary-Lautsprecher.
const PatchConsts gs1_ElectronicOrganI = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // GS1-Organ: harmonisch, wie Hammond-Drawbars
    // 1:2:1:2 → Grundton + Oktave + leichte Obertöne
    {1.0f, 2.0f, 1.0f, 2.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // GS1-Organ war *streng gestimmt*, aber ein Hauch Breite tut gut
    {0, 0, 0, 0},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger
    // ---------------------------------------------------------
    // Voll über die gesamte Tastatur
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 4) C2EC — Oktavlayer
    // ---------------------------------------------------------
    // GS1-typisch: leicht dumpfer als C1
    { 0.0f, 0.8f,
      0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,0.8,
      0.8,0.8,0.8,0.8 },

    // ---------------------------------------------------------
    // 5) M1EC — leichte Modulation (1×)
    // ---------------------------------------------------------
    // Gibt dem Klang Wärme, aber keine FM-Schärfe
    { 0.0f, 0.7f,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7 },

    // ---------------------------------------------------------
    // 6) M2EC — zweite Harmonische (2×)
    // ---------------------------------------------------------
    // Gibt Präsenz, aber bleibt GS1-dumpf
    { 0.0f, 0.6f,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Orgel = sofortiger Attack
    {10000.0f, 10000.0f, 10000.0f, 10000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.0f
};


// --- (15) F-2  Electronic Organ II ---
// Reichere Orgel mit mehr Obertönen (Ratio 3x, 4x). Leichter Detune
// simuliert den Rotary-Effekt ohne Chorus-Einheit. Lautstärke-Pedal empfohlen.
const PatchConsts gs1_ElectronicOrganII = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Organ II = heller, obertonreicher als Organ I
    // 1:2:2:4 → wie Hammond-Drawbars 8' + 4' + 4' + 2'
    {1.0f, 2.0f, 2.0f, 4.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Minimal, aber etwas breiter als Organ I
    {0, 2, 0, 3},

    // ---------------------------------------------------------
    // 3) C1EC — Grundtonträger (8')
    // ---------------------------------------------------------
    // Voll, warm, GS1-dumpf
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 4) C2EC — Oktavlayer (4')
    // ---------------------------------------------------------
    // Heller als C1, aber GS1-typisch abfallend
    { 0.0f, 0.9f,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.8,0.8,0.7,0.7,0.6,
      0.5,0.4,0.3,0.2,0.15,0.1,0.05,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — 4'-Harmonic (2×)
    // ---------------------------------------------------------
    // Gibt dem Klang Präsenz und „Drawbar“-Charakter
    { 0.0f, 0.8f,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,
      0.7,0.7,0.7,0.7,0.6,0.6,0.5,0.5,0.4,0.3,
      0.2,0.2,0.1,0.1,0.05,0.05,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — 2'-Harmonic (4×)
    // ---------------------------------------------------------
    // Heller, aber GS1-dumpf → kein FM-Metall
    { 0.0f, 0.7f,
      0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,
      0.4,0.4,0.3,0.3,0.2,0.2,0.1,0.1,0.05,0.05,
      0.02,0.02,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Orgel = sofortiger Attack
    {10000.0f, 10000.0f, 10000.0f, 10000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.0f
};


// --- (16) F-5  Pipe Organ ---
// Pfeifenorgel. Harmonische Obertonreihe (1:2:4:8). Kein Detune —
// streng gestimmt. Sofortiger Attack, Lautstärke-Pedal empfohlen.
const PatchConsts gs1_PipeOrgan = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Pipe Organ = harmonisch, wie Drawbars 16' + 8' + 4' + 2'
    // 1×, 2×, 4×, 8×
    {1.0f, 2.0f, 4.0f, 8.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Pipe Organ = absolut sauber gestimmt
    {0, 0, 0, 0},

    // ---------------------------------------------------------
    // 3) C1EC — Grundton (16')
    // ---------------------------------------------------------
    // Voll, warm, GS1-dumpf
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 4) C2EC — Oktave (8')
    // ---------------------------------------------------------
    // Heller, aber GS1-typisch abfallend
    { 0.0f, 0.9f,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,
      0.9,0.9,0.9,0.9,0.9,0.8,0.8,0.7,0.7,0.6,
      0.5,0.4,0.3,0.2,0.15,0.1,0.05,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 5) M1EC — 4'-Harmonic (4×)
    // ---------------------------------------------------------
    // Gibt dem Klang Präsenz und „Pipe“-Charakter
    { 0.0f, 0.8f,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,
      0.6,0.6,0.6,0.6,0.5,0.5,0.4,0.4,0.3,0.2,
      0.2,0.1,0.1,0.05,0.05,0.02,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — 2'-Harmonic (8×)
    // ---------------------------------------------------------
    // Sehr hell, aber GS1-dumpf → kein FM-Metall
    { 0.0f, 0.7f,
      0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,
      0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,0.4,
      0.3,0.3,0.2,0.2,0.1,0.1,0.05,0.05,0.02,0.02,
      0.01,0.01,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times (ms)
    // ---------------------------------------------------------
    // Pipe Organ = sofortiger Attack
    {10000.0f, 10000.0f, 10000.0f, 10000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling
    // ---------------------------------------------------------
    1.0f
};

// E PIANO  (DX7 Alg 5, derived from patchEP1)
const PatchConsts patchE_PIANO = {
    {0.50, 1.00, 0.50, 18.00},  // C1,C2,M1,M2 Ratios.s.
    {+0, +3, +2, +0}, // C1,C2,M1,M2 detune in cents +-16 in cents +-16
    {
        0.0, 0.65, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
        0.9, 0.9,  0.9, 1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        0.9, 0.9,  0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8
    },
    {
        0.0, 0.35, 0.2, 0.3, 0.3, 0.4, 0.5, 0.5, 0.6, 0.6, 0.7, 0.7,
        0.8, 0.8,  0.9, 0.9, 1,   1,   1,   1,   0.9, 0.9, 0.8, 0.7,
        0.6, 0.5,  0.4, 0.3, 0.2, 0.2, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1,  0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1
    },
    {   
        0.0,  0.7,  0.9,  0.9,  0.6,  0.6, 0.6, 0.6, 0.7, 0.7,
        0.7,  0.8,  0.8,  0.9,  0.9,  1,   0.9, 0.8, 0.8, 0.7,
        0.7,  0.7,  0.7,  0.6,  0.6,  0.5, 0.5, 0.4, 0.4, 0.3,
        0.3,  0.2,  0.2,  0.1,  0.1,  0.1, 0.1, 0.1, 0.1, 0.1,
        0.05, 0.05, 0.05, 0.05, 0.05, 0.05
    },
    {
        0.1, 0.55, 0.9, 0.9, 1,   1,    1,    1,   1,   1,
        1,   1,    1,   0.9, 0.9, 0.8,  0.8,  0.7, 0.7, 0.7,
        0.6, 0.6,  0.6, 0.5, 0.5, 0.5,  0.5,  0.5, 0.5, 0.4,
        0.3, 0.2,  0.1, 0.1, 0.1, 0.05, 0.01, 0.0, 0.0, 0.0,
        0.0, 0.0,  0.0, 0.0, 0.0, 0.0
    },
    {2000, 2000, 4400, 4400}, 
    3.0
};

// MARIMBA  (DX7 Alg 28, derived from patchBell)
const PatchConsts patchMARIMBA = {
    {1.00, 1.00, 7.00, 5.00},  // C1,C2,M1,M2 Ratios.s → asymmetrisch, spannend
    {+6, -1, +0, +4}, // C1,C2,M1,M2 detune in cents +-16 → lebendige Schwebung

    // Carrier EC: langsam ausschwingend
    {
        0.0, 0.6, 1.0, 1.0, 1.0, 1.0, 0.95, 0.95, 0.9, 0.9,
        0.85, 0.85, 0.8, 0.8, 0.75, 0.75, 0.7, 0.7, 0.65, 0.65,
        0.6, 0.6, 0.55, 0.55, 0.5, 0.5, 0.45, 0.45, 0.4, 0.4,
        0.3, 0.3, 0.2, 0.2, 0.15, 0.15, 0.1, 0.1, 0.07, 0.07,
        0.05, 0.05, 0.03, 0.03, 0.01, 0.01
    },

    // Modulator M1EC: starker Anschlag, langes Plateau
    {
        0.0, 0.5, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.85, 0.85,
        0.8, 0.8, 0.75, 0.75, 0.7, 0.7, 0.6, 0.6, 0.5, 0.5,
        0.4, 0.4, 0.3, 0.3, 0.2, 0.2, 0.15, 0.15, 0.1, 0.1,
        0.08, 0.08, 0.05, 0.05, 0.03, 0.03, 0.02, 0.02, 0.01, 0.01,
        0.01, 0.01, 0.01, 0.01, 0.01, 0.01
    },

    // Modulator M2EC: sanfter Verlauf, Schimmer
    {
        0.1, 0.3, 0.6, 0.6, 0.6, 0.6, 0.55, 0.55, 0.5, 0.5,
        0.45, 0.45, 0.4, 0.4, 0.35, 0.35, 0.3, 0.3, 0.25, 0.25,
        0.2, 0.2, 0.15, 0.15, 0.1, 0.1, 0.08, 0.08, 0.06, 0.06,
        0.04, 0.04, 0.03, 0.03, 0.02, 0.02, 0.01, 0.01, 0.005, 0.005,
        0.005, 0.005, 0.005, 0.005, 0, 0
    },

    // Modulator M3EC: kurzer metallischer Peak
    {
        0.2, 0.8, 1.0, 1.0, 0.9, 0.9, 0.8, 0.8, 0.7, 0.7,
        0.6, 0.6, 0.5, 0.5, 0.3, 0.3, 0.2, 0.2, 0.1, 0.1,
        0.05, 0.05, 0.02, 0.02, 0.01, 0.01, 0.01, 0.01, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    },

    {3000, 2500, 3600, 4200},   // ATE: verlängerte Attack/Decay → weiches Einschwingen
    24.0                        // DTE1Scaling: leichte Anpassung für Detune-Verlauf
};

// BRASS SECT  (DX7 Alg 22, derived from patchBrass)
const PatchConsts patchBRASS_SECT = {
    {1.00, 1.00, 1.00, 1.00},  // C1,C2,M1,M2 Ratios.: leicht gestaffelt, mit Obertonbetonung
    {+0, -2, +0, +0}, // C1,C2,M1,M2 detune in cents +-16 in Cent: Schwebung und Breite
    // C1EC – Carrier 1 Envelope Curve
    {
        0.0, 0.3, 0.6, 0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    // C2EC – Carrier 2 Envelope Curve
    {
        0.0, 0.4, 0.7, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    // M1EC – Modulator 1 Envelope Curve
    {
        0.0, 0.7, 0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    // M2EC – Modulator 2 Envelope Curve
    {
        0.0, 0.6, 0.85, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    {3000, 3000, 3000, 3000},   // ATE – schneller Attack für perkussiven Einschlag
    1.0                         // DTE1Scaling – volle Dynamik
};

// GRAND PIAN  (DX7 Alg 16, derived from patchEPiano1)
const PatchConsts patchGRAND_PIAN = {
    {0.50, 1.00, 0.50, 5.00},  // C1,C2,M1,M2 Ratios.s: asymmetrisch, simulieren Tine + Resonanzkörper
    {+0, +0, +0, +0}, // C1,C2,M1,M2 detune in cents +-16: gegengesetzt für weiche Schwebung

    // Carrier EC – schnelles Attack, aber lange Ausschwingphase
    {
        0.0, 0.4, 0.85, 0.85, 0.85, 0.85, 0.8, 0.8, 0.75, 0.75,
        0.7, 0.7, 0.65, 0.65, 0.6, 0.6, 0.5, 0.5, 0.4, 0.4,
        0.3, 0.3, 0.2, 0.2, 0.15, 0.15, 0.1, 0.1, 0.05, 0.05,
        0.03, 0.03, 0.01, 0.01, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    },

    // M1EC – starker Impuls, mittellang ausschwingend
    {
        0.0, 0.6, 1.0, 1.0, 1.0, 1.0, 0.9, 0.9, 0.8, 0.8,
        0.7, 0.7, 0.6, 0.6, 0.5, 0.5, 0.4, 0.4, 0.3, 0.3,
        0.2, 0.2, 0.1, 0.1, 0.05, 0.05, 0.02, 0.02, 0.01, 0.01,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    },

    // M2EC – langes, sanftes Decay für Schimmer
    {
        0.1, 0.4, 0.8, 0.8, 0.8, 0.8, 0.7, 0.7, 0.6, 0.6,
        0.5, 0.5, 0.4, 0.4, 0.3, 0.3, 0.2, 0.2, 0.1, 0.1,
        0.08, 0.08, 0.06, 0.06, 0.04, 0.04, 0.02, 0.02, 0.01, 0.01,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    },

    // M3EC – für ganz feinen Glanz
    {
        0.05, 0.2, 0.3, 0.3, 0.25, 0.25, 0.2, 0.2, 0.15, 0.15,
        0.1, 0.1, 0.08, 0.08, 0.06, 0.06, 0.04, 0.04, 0.03, 0.03,
        0.02, 0.02, 0.01, 0.01, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    },

    {1600, 1200, 2000, 2300},   // ATE – schneller Attack, moderate Decay
    16.0                        // DTE1Scaling – neutraler Verlauf
};

// PIPE  (DX7 Alg 6, derived from patchOrgan)
const PatchConsts patchPIPE = {
    {2.00, 1.00, 1.00, 0.50},  // C1,C2,M1,M2 Ratios.: harmonisch gestaffelt
    {-2, +2, +0, +0}, // C1,C2,M1,M2 detune in cents +-16: stabil, keine Modulation
    // C1EC – Carrier 1 Envelope Curve
    {
        0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    // C2EC – Carrier 2 Envelope Curve
    {
        0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    },
    // M1EC – Modulator 1 Envelope Curve
    {
        0.0, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
        0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
        0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
        0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
        0.9, 0.9, 0.9, 0.9, 0.9, 0.9
    },
    // M2EC – Modulator 2 Envelope Curve
    {
        0.0, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8,
        0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8,
        0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8,
        0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8,
        0.8, 0.8, 0.8, 0.8, 0.8, 0.8
    },
    {5000, 5000, 5000, 5000},   // ATE – langsamer Attack für weichen Start
    1.0                         // DTE1Scaling – kaum Skalierung
};

// ============================================================
// Lookup-Array für alle 16 Factory-Presets (Index 0..15)
// Einbinden in CGS1Emu: patches[i] = gs1FactoryPresets[i];
// ============================================================
static const PatchConsts* const gs1FactoryPresets[16] = {
    &gs1_HarpsichordI,      // (1)  A-1
    &gs1_HarpsichordIII,    // (2)  A-2
    &gs1_ClavichordII,      // (3)  A-4
    &gs1_Vibraphone,        // (4)  B-1
    &gs1_Celeste,           // (5)  B-2
    &gs1_AcousticPianoI,    // (6)  C-1
    &gs1_ElectricPianoI,    // (7)  C-4
    &gs1_ElectricPianoIII,  // (8)  C-6
    &gs1_StringI,           // (9)  D-1
    &gs1_StringEnsembleI,   // (10) D-3
    &gs1_BrassI,            // (11) E-1
    &gs1_BrassII,           // (12) E-2
    &gs1_SynthBrassIII,     // (13) E-6
    &gs1_ElectronicOrganI,  // (14) F-1
    &gs1_ElectronicOrganII, // (15) F-2
    &gs1_PipeOrgan          // (16) F-5
};
