#pragma once

static constexpr int PATCH_NAME_MAX = 16;

struct PatchConsts {
    float Ratio[4];
    int Detune[4];
    float C1EC[46];
    float C2EC[46];
    float M1EC[46];
    float M2EC[46];
    float ATE[4];
    float DTE1Scaling;

    int DTE[4] = {2, 2, 1, 1};          // Decay Time Rates
    int RTE[4] = {100, 100, 100, 100};  // Release Time Rates
    int IL[4] = {0, 0, 0, 0};           // Initial Levels (meist 0)
    int SL[4] = {0, 0, 0, 0};           // Sustain Levels (Wichtig für Orgel/Strings!)
    int FMmode[2] = {0, 0};             // Mode für Stack 1 & Stack 2
    char Name[PATCH_NAME_MAX + 1] = {0}; // Patch-Name (max 16 Zeichen + Null)

    // --- Double-Stack-Konfiguration (zuschaltbarer 2. Stack-Pair) ---
    // Nur aktiv wenn setDoubleStacksOn(true). Steuert, wie sich der
    // verdoppelte Stack vom Original unterscheidet.
    //
    // DS_Detune: Cent-Offset pro Operator (C1,C2,M1,M2), ADDITIV zu Detune[].
    //   Die CARRIER (C1,C2) müssen verstimmt werden, sonst laufen beide Stacks
    //   frequenzgleich und es entsteht keine hörbare Schwebung.
    //   Default = breites Ensemble (~±7 Cent Carrier, ±9 Cent Modulator).
    float DS_Detune[4] = {7.0f, -7.0f, 9.0f, -9.0f};
    // Mix-Gewichte Basis-Stack : Layer-Stack (Ausgabe wird normiert).
    float DS_MixBase  = 1.0f;
    float DS_MixLayer = 1.0f;
    // Envelope-Skalierung für Stack 2 (1.0 = identisch zu Stack 1).
    // >1 = schneller (Attack/Decay), <1 = langsamer → gibt der Schicht ein
    // Eigenleben (z.B. langsamerer Layer-Attack für Streicher-Swell).
    float DS_ATScale[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float DS_DTScale[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    // --- Layer-Modus: Stack 2 als EIGENSTÄNDIGE Klangschicht ---
    // Macht aus dem Double-Stack mehr als nur Chorus: eigene Ratios/Topologie
    // → z.B. ein kurzer Hammer-Attack-Layer unter einem sauberen Body.
    //
    // DS_Ratio: eigene Operator-Ratios (C1,C2,M1,M2) für Stack 2.
    //   Wert <= 0  → erbt Ratio[] von Stack 1 (= Chorus-Verhalten, default).
    float DS_Ratio[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // DS_FMmode: eigene FM-Topologie für Stack 2 (Stack-1- & Stack-2-Pair).
    //   Wert < 0   → erbt FMmode[] von Stack 1 (default).
    int DS_FMmode[2] = {-1, -1};
    // DS_LevelDb: Pegel-Offset pro Operator für Stack 2 in dB
    //   (negativ = leiser). 0 = unverändert (default). Bei Carriern wirkt es
    //   auf die Lautstärke, bei Modulatoren auf den FM-Index (Helligkeit).
    float DS_LevelDb[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // DS_ModKbdDb: zusätzliche Dämpfung der LAYER-Modulatoren (M1,M2) pro Oktave
    //   OBERHALB von C2 (KNOTE 15), in dB. Reduziert den FM-Index (Helligkeit)
    //   zu hohen Tönen hin, damit der Anschlag nicht "metallisch" wird: seine
    //   spektrale Helligkeit steigt dann NICHT proportional mit der Tonhöhe
    //   (natürlicher Filzhammer statt fest mitskalierender Bite).
    //   0 = aus (default, rückwärtskompatibel). Wirkt nur auf Stack 2.
    float DS_ModKbdDb = 0.0f;
    // DS_DTKbdTrack: wie stark die LAYER-Decayzeit der Tastatur folgt (Exponent
    //   auf den Keyboard-Faktor). Wirkt NUR auf Stack 2.
    //   1.0 = exakt wie Stack 1 (tiefe Töne lang, hohe sehr kurz) — default,
    //         rückwärtskompatibel.
    //   0.0 = keine Tastaturverfolgung → überall gleich lange Klick-Transiente.
    //   ~0.3–0.5 = gleichmäßiger Anschlag über die Tastatur (kein "Bambus" in
    //         der Tiefe, kein "Tick/Zischen" in der Höhe).
    float DS_DTKbdTrack = 1.0f;
    // DS_MixMode: wie Stack 2 in die Summe eingeht.
    //   0 = BLEND  (Mittelwert (base*wB + layer*wL)/(wB+wL)) → Chorus, default.
    //               Gleichlauter Zweitstack ohne Übersteuerung.
    //   1 = ADD    (base*wB + layer*wL, geclamped) → echter Layer. Der Grundklang
    //               bleibt auf vollem Pegel, die (kurze) Layer-Schicht wird
    //               oben drauf addiert (Hammer-Bite, Anschlags-Transient).
    int DS_MixMode = 0;

    // BaseLevelDb: Pegel-Offset pro Operator für den BASIS-Stack (Stack 1) in dB.
    //   Spiegelbild zu DS_LevelDb, wirkt aber IMMER (auch ohne Double-Stack).
    //   Bei Carriern (C1=0,C2=1) → Lautstärke; bei Modulatoren (M1=2,M2=3) →
    //   FM-Index = HELLIGKEIT/Timbre. Negativ = dunkler/weicher (weniger
    //   Obertöne, "holziger"), positiv = heller (mehr Bite).
    //   0 = unverändert (default, rückwärtskompatibel). Das ist die zentrale
    //   Achse, um Preset-Varianten EINER Klangfamilie klanglich zu trennen.
    float BaseLevelDb[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // OutLevelDb: Ausgangspegel des GESAMTEN Presets in dB (linearer Gain auf
    //   den fertig gemischten Stimmen-Ausgang). Dient der Lautstärke-Angleichung
    //   zwischen Presets (z.B. leise EPs anheben, heiße Harpsichords absenken).
    //   0 = unverändert (default). Positiv = lauter, negativ = leiser.
    float OutLevelDb = 0.0f;

    // DTEKbdScale: oberer Endwert des Keyboard-Decay-Scalings für die Operatoren
    //   C1/M1/M2 (DT[0],[2],[3]). Der Decay-Faktor läuft von 0.5 (tiefste Taste)
    //   bis DTEKbdScale (höchste Taste). Höher = hohe Töne klingen viel schneller
    //   aus. 3.0 = Originalverhalten (default, rückwärtskompatibel); kleinere
    //   Werte (~1.0–1.5) geben hohen Tönen einen längeren, natürlicheren Ausklang
    //   (z.B. für akustische Klaviere). C2 (DT[1]) nutzt weiter DTE1Scaling.
    float DTEKbdScale = 3.0f;
};

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
    6.0f,
    {3, 3, 2, 2},               // DTE
    {80, 80, 80, 80},           // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "Harpsichord I",
    // ===== LAYER: Mechanik-"Thunk" (Tastatur/Docke) =====
    // Der native Cembalo-Anschlag ist bereits sehr hell/scharf; ein heller
    // Pluck-Layer geht darin unter. Stattdessen ein tiefer, holziger mecha-
    // nischer Anschlag (Docken-/Tastenmechanik), der hörbar kontrastiert.
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer (kein Chorus)
    1.0f, 1.0f,                  // DS_MixBase : DS_MixLayer (voll, Basis ist laut)
    {0.5f, 0.5f, 0.5f, 0.5f},    // DS_ATScale: schneller Mechanik-Knock
    {90.0f, 160.0f, 130.0f, 8.0f}, // DS_DTScale: kurz, aber mit etwas Körper
    {1.0f, 1.0f, 2.0f, 3.0f},    // DS_Ratio: tiefer, holziger Knock
    {0, 0},                      // DS_FMmode: NORM
    {13.0f, 10.0f, 2.0f, 0.0f},  // DS_LevelDb (laut genug, um durch helle Basis zu kommen)
    4.0f,                        // DS_ModKbdDb: oben weniger metallisch
    0.35f,                       // DS_DTKbdTrack: gleichmäßige Länge
    1,                           // DS_MixMode: ADD
    {0.0f, 0.0f, 0.0f, 0.0f},    // BaseLevelDb
    -4.0f                        // OutLevelDb: war zu heiß/laut → etwas absenken
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
    7.0f,
    {3, 3, 2, 2},               // DTE
    {80, 80, 80, 80},           // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "Harpsichord III",
    // ===== LAYER: Mechanik-"Thunk" (etwas heller als A-1) =====
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 1.0f,                  // DS_MixBase : DS_MixLayer (voll)
    {0.5f, 0.5f, 0.5f, 0.5f},    // DS_ATScale: schneller Knock
    {90.0f, 160.0f, 130.0f, 8.0f}, // DS_DTScale: kurz mit Körper
    {1.0f, 1.0f, 3.0f, 4.0f},    // DS_Ratio: holziger Knock, etwas heller
    {0, 0},                      // DS_FMmode: NORM
    {13.0f, 10.0f, 2.0f, 0.0f},  // DS_LevelDb (laut genug für die helle Basis)
    4.5f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1,                           // DS_MixMode: ADD
    {0.0f, 0.0f, 0.0f, 0.0f},    // BaseLevelDb
    -4.0f                        // OutLevelDb: war zu heiß/laut → etwas absenken
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
    "Clavichord II",
    // ===== LAYER: intimer Tangenten-Anschlag (key contact) =====
    // Leiser, dezent metallischer Kontakt-Klick. Layer in NORM (sauberer
    // Klick), unabhängig vom CROSSMOD-Buzz des Grund-Stacks.
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 0.4f,                  // DS_MixBase : DS_MixLayer (sehr dezent)
    {0.6f, 0.6f, 0.5f, 0.5f},    // DS_ATScale
    {160.0f, 280.0f, 220.0f, 12.0f}, // DS_DTScale: kurzer Kontakt-Klick
    {1.0f, 2.0f, 5.0f, 4.0f},    // DS_Ratio
    {0, 0},                      // DS_FMmode: NORM (sauberer Klick)
    {3.0f, 2.0f, 1.0f, 0.0f},    // DS_LevelDb (zurückhaltend, intim)
    4.0f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1                            // DS_MixMode: ADD
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
    "Vibraphone",
    // ===== LAYER: weicher Mallet-Anschlag (yarn mallet) =====
    // Runder, eher dunkler "Thunk" des weichen Schlägels auf der Metallplatte
    // (kein heller Bite). Weicher Attack, tiefere Modulator-Ratios.
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 0.55f,                 // DS_MixBase : DS_MixLayer
    {0.4f, 0.4f, 0.35f, 0.35f},  // DS_ATScale: weicher Schlägel
    {120.0f, 220.0f, 180.0f, 8.0f}, // DS_DTScale: kurzer Mallet-Thunk
    {1.0f, 2.0f, 4.0f, 3.0f},    // DS_Ratio: dunkler/weicher als Klavier
    {0, 0},                      // DS_FMmode: NORM
    {5.0f, 3.0f, 1.0f, 0.0f},    // DS_LevelDb
    3.0f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1                            // DS_MixMode: ADD
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
    3.0f,
    {3, 2, 2, 2},               // DTE
    {80, 80, 80, 80},           // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "Celeste",
    // ===== LAYER: heller Glocken-Ping-Anschlag =====
    // Kurzer, glockig-heller "Ping" des Hammers auf der Metallplatte
    // (heller als Vibraphon, aber tonal dank Rolloff nach oben).
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 0.5f,                  // DS_MixBase : DS_MixLayer
    {0.5f, 0.5f, 0.45f, 0.45f},  // DS_ATScale
    {140.0f, 260.0f, 200.0f, 9.0f}, // DS_DTScale: kurzer Bell-Ping
    {1.0f, 3.0f, 7.0f, 5.0f},    // DS_Ratio: glockig-hell (C2=3 für Bell-Anteil)
    {0, 0},                      // DS_FMmode: NORM
    {5.0f, 3.0f, 2.0f, 1.0f},    // DS_LevelDb
    4.0f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1                            // DS_MixMode: ADD
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
    {2, 2, 1, 60},              // DTE: M2 Hammer-Bite extrem schnell
    {100, 100, 100, 100},       // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "Acoustic Piano I",
    // ===== DOUBLE-STACK = ECHTER LAYER (kein Chorus) =====
    // Stack 2 ist hier KEIN verstimmter Klon, sondern eine eigenständige
    // Hammer-Attack-Schicht: ein heller, perkussiver Transient (~100 ms),
    // der dem Grundklang den fehlenden Anschlags-"Klick"/Bite gibt.
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: 0 → keine Schwebung, reiner Layer
    1.0f, 0.6f,                  // DS_MixBase : DS_MixLayer (Layer ≈ -4.4 dB
                                 //   gedrosselt → Anschlag tritt weniger vor)
    // DS_ATScale < 1 → langsamerer, weicherer Layer-Attack: rundet den harten
    // "mit-Stock-angeschlagen"-Transienten zu einem Filzhammer-Thump ab.
    {0.4f, 0.4f, 0.3f, 0.3f},    // DS_ATScale: weicher Filz-Anschlag
    // DS_DTScale: sehr schneller Carrier-Decay → Layer ist ein kurzer
    // Transient (~100 ms), kein anhaltender Ton. Modulator hält kurz nach,
    // damit der Bite hell ist, M2 fällt extrem schnell.
    {130.0f, 260.0f, 200.0f, 7.0f},
    // ----- Layer-spezifische Klangfarbe (überschreibt Stack-1-Vererbung) -----
    // Moderate Modulator-Ratios → Helligkeit sitzt nah am Grundton, das
    // Spektrum füllt sich glatt auf (kein isolierter HF-Cluster = kein
    // "Zischeln/Rauschen" wie bei M1=14/M2=11). C2=2 (Oktave) gibt dem
    // Klick Körper statt Inharmonizität.
    {1.0f, 2.0f, 6.0f, 4.0f},    // DS_Ratio: C1=Grundton, C2=Oktave-Klick,
                                 //   M1=6 / M2=4 → heller, aber tonaler Bite
    {0, 0},                      // DS_FMmode: NORM
    {6.0f, 4.0f, 2.0f, 1.0f},    // DS_LevelDb: Carrier +6/+4 (Präsenz),
                                 //   Modulatoren nur +2/+1 → moderater Index
    4.0f,                        // DS_ModKbdDb: Bite pro Oktave über C2 um 4 dB
                                 //   leiser → hohe Töne nicht metallisch
    0.35f,                       // DS_DTKbdTrack: Layer-Länge fast tastatur-
                                 //   unabhängig → gleicher Anschlag tief wie hoch
    1                            // DS_MixMode: 1 = ADD (Layer oben drauf, kein Chorus)
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
    {0, 0},             // FMmode: Beides NORM (keine Crossmod, kein Feedback)
    "Electric Piano I",
    // ===== LAYER: Tine-"Bark"/Bite (Rhodes-Anschlag) =====
    // Der charakteristische helle Tine-Bark beim Anschlag, oben drauf addiert.
    // Recht direkt (Bark ist sofort da), kurz, mittenbetont-hell.
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 0.35f,                 // DS_MixBase : DS_MixLayer (subtiler, weniger fremd)
    {0.7f, 0.7f, 0.6f, 0.6f},    // DS_ATScale: etwas weicherer Bite
    {110.0f, 200.0f, 160.0f, 7.0f}, // DS_DTScale: kurzer Bark
    {1.0f, 2.0f, 6.0f, 4.0f},    // DS_Ratio: heller Tine-Bite
    {0, 0},                      // DS_FMmode: NORM
    {4.0f, 2.0f, 1.0f, 0.0f},    // DS_LevelDb (Bark dezenter, weniger metallisch)
    4.0f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1                            // DS_MixMode: ADD
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
    15.0f,
    {2, 2, 1, 1},               // DTE
    {250, 250, 250, 250},       // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "Elec Piano III",
    // ===== LAYER: weicherer Tine-Bite (mellower EP) =====
    {0.0f, 0.0f, 0.0f, 0.0f},    // DS_Detune: reiner Layer
    1.0f, 0.3f,                  // DS_MixBase : DS_MixLayer (subtiler)
    {0.7f, 0.7f, 0.6f, 0.6f},    // DS_ATScale: weicherer Bite
    {110.0f, 200.0f, 170.0f, 7.0f}, // DS_DTScale: kurzer Bite
    {1.0f, 2.0f, 5.0f, 4.0f},    // DS_Ratio: etwas weicher als EP I
    {0, 0},                      // DS_FMmode: NORM
    {3.0f, 2.0f, 1.0f, 0.0f},    // DS_LevelDb (dezenter)
    4.0f,                        // DS_ModKbdDb
    0.35f,                       // DS_DTKbdTrack
    1                            // DS_MixMode: ADD
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
    1.0f,
    {1, 1, 1, 1},               // DTE: langsames Ausklingen für Bogen
    {60, 60, 60, 60},           // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "String I",
    // Double-Stack: Solo-Streicher → weite Verstimmung; Layer-Carrier schwingt
    // deutlich langsamer ein (AT 0.7) → evolvierender Ensemble-Swell.
    {10.0f, -10.0f, 8.0f, -7.0f}, // DS_Detune (cents)
    1.0f, 1.0f,                   // DS_MixBase : DS_MixLayer
    {0.70f, 0.70f, 0.80f, 0.80f}, // DS_ATScale
    {1.0f, 1.0f, 1.0f, 1.0f}      // DS_DTScale
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
    // Gibt Breite und „Chor"-Charakter
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
    1.2f,
    {1, 1, 1, 1},               // DTE
    {80, 80, 80, 80},           // RTE
    {0, 0, 0, 0},               // IL
    {0, 0, 0, 0},               // SL
    {0, 0},                     // FMmode
    "String Ens I",
    // Double-Stack: breitester Satz → maximale Verstimmung + gestaffelter
    // Layer-Attack für üppiges Streicher-Ensemble.
    {12.0f, -12.0f, 9.0f, -9.0f}, // DS_Detune (cents)
    1.0f, 1.0f,                   // DS_MixBase : DS_MixLayer
    {0.65f, 0.70f, 0.80f, 0.80f}, // DS_ATScale
    {1.05f, 1.05f, 1.0f, 1.0f}    // DS_DTScale
};


// --- (11) E-1  Brass I ---
// Hornartiger Blechbläser (French Horn / Brass Section).
// PI/2 Self-Feedback auf Stack 1 erzeugt den typischen FM-Brass-Buzz.
// Brightness-Hüllkurve: Modulatoren decayen von hellem Anschlag auf
// ein warmes Sustain-Level (SL niedrig). Carrier bleiben konstant (SL hoch).
const PatchConsts gs1_BrassI = {
    // Ratios: beide Stacks 1:1 → Grundton-Modulation (kein Inharmonik)
    {1.0f, 1.0f, 1.0f, 1.0f},

    // Leichtes Detune für lebendigen Ensemble-Sound
    {0, 8, -2, 5},

    // C1EC — Carrier Stack 1: voll und gleichmäßig
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.5,0.3,
      0.15,0.05,0.0,0.0 },

    // C2EC — Carrier Stack 2: leicht leiser
    { 0.0f, 0.92f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.4,0.2,
      0.1,0.0,0.0,0.0 },

    // M1EC — Modulator Stack 1 (PI/2 Feedback): hoch für Brass-Brillanz
    // Flat über die gesamte Tastatur — Brass ändert nicht den Charakter mit der Höhe
    { 0.0f, 0.90f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.5,0.3,
      0.1,0.0,0.0,0.0 },

    // M2EC — Modulator Stack 2 (NORM): etwas weniger FM für wärmeren Body
    { 0.0f, 0.80f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.4,0.2,
      0.1,0.0,0.0,0.0 },

    // ATE: Modulatoren sofort (FM-Index gleich hoch) → Carrier langsam (Horn-Swell ~12ms)
    {1200.0f, 1200.0f, 30000.0f, 25000.0f},

    2.0f,                           // DTE1Scaling
    {1, 1, 8, 6},                   // DTE: Carrier konstant, Modulatoren decay in ~1.5s
    {80, 80, 70, 70},               // RTE
    {0, 0, 0, 0},                   // IL
    {252, 252, 175, 170},           // SL: Carrier voll, Modulatoren auf warmes Sustain
    {1, 0},                         // FMmode: PI/2 (Stack 1 Buzz) + NORM (Stack 2 Body)
    "Brass I",
    // Double-Stack: Layer etwas lauter (8:11 wie zuvor), Carrier-Swell versetzt.
    {6.0f, -6.0f, 9.0f, -9.0f},  // DS_Detune (cents)
    8.0f, 11.0f,                 // DS_MixBase : DS_MixLayer
    {0.80f, 0.80f, 1.0f, 1.0f},  // DS_ATScale
    {1.0f, 1.0f, 1.10f, 1.10f}   // DS_DTScale
};


// --- (12) E-2  Brass II ---
// Trompete. PI/2 auf beiden Stacks → heller, schneidender FM-Buzz.
// Modulatoren decayen schnell auf niedrigeres SL → klassische Trompeten-
// Brightness-Hüllkurve (schrill im Attack, wärmer im Sustain).
const PatchConsts gs1_BrassII = {
    {1.0f, 1.0f, 1.0f, 1.0f},

    // Leichtes Ensemble-Detune
    {0, 10, -3, 6},

    // C1EC — Carrier Stack 1: voll
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.4,0.2,
      0.1,0.0,0.0,0.0 },

    // C2EC — Carrier Stack 2
    { 0.0f, 0.95f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.3,0.1,
      0.0,0.0,0.0,0.0 },

    // M1EC — hoher FM-Index für Trompeten-Brillanz
    { 0.0f, 0.95f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.4,0.2,
      0.05,0.0,0.0,0.0 },

    // M2EC — etwas weniger
    { 0.0f, 0.88f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.3,0.1,
      0.0,0.0,0.0,0.0 },

    // ATE: Modulatoren sofort, Carrier schnell (~4ms Punch)
    {3500.0f, 3000.0f, 30000.0f, 28000.0f},

    2.5f,                           // DTE1Scaling
    {1, 1, 12, 10},                 // DTE: Carrier konstant, Modulatoren decay in ~1s
    {100, 90, 80, 80},              // RTE
    {0, 0, 0, 0},                   // IL
    {252, 252, 185, 180},           // SL: Carrier voll, Modulatoren auf mittleres Sustain
    {1, 1},                         // FMmode: PI/2 auf beiden Stacks → heller Trompeten-Buzz
    "Brass II",
    // Double-Stack: Trompete → breiter, Layer minimal betont, gestaffelter Swell.
    {7.0f, -7.0f, 10.0f, -9.0f}, // DS_Detune (cents)
    1.0f, 1.10f,                 // DS_MixBase : DS_MixLayer
    {0.85f, 0.85f, 1.0f, 1.0f},  // DS_ATScale
    {1.0f, 1.0f, 1.10f, 1.10f}   // DS_DTScale
};


// --- (13) E-6  Synth Brass III ---
// Breiter Synth-Brass à la Jupiter/JX. PI/2 auf beiden Stacks.
// Mehr Detune als Brass I+II für den typischen Ensemble-Synth-Charakter.
// Schnellerer Brightness-Decay → punchiger, synthetischer.
const PatchConsts gs1_SynthBrassIII = {
    {1.0f, 1.0f, 1.0f, 1.0f},

    // Breites Detune: Ensemble-Synth-Charakter
    {0, 20, 10, -14},

    // C1EC — Carrier Stack 1
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.4,0.2,
      0.1,0.0,0.0,0.0 },

    // C2EC — Carrier Stack 2
    { 0.0f, 0.95f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.3,0.1,
      0.0,0.0,0.0,0.0 },

    // M1EC — maximaler FM-Index für aggressiven Synth-Attack
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,0.9,0.7,0.4,0.2,
      0.05,0.0,0.0,0.0 },

    // M2EC
    { 0.0f, 0.93f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.3,0.1,
      0.0,0.0,0.0,0.0 },

    // ATE: Modulatoren sofort, Carrier leichter Swell (~8ms)
    {2000.0f, 1800.0f, 28000.0f, 25000.0f},

    2.5f,                           // DTE1Scaling
    {1, 1, 15, 12},                 // DTE: Carrier konstant, Modulatoren schnell decay (~0.8s)
    {90, 80, 75, 75},               // RTE
    {0, 0, 0, 0},                   // IL
    {252, 252, 155, 150},           // SL: Carrier voll, Modulatoren niedriger = mehr Brightness-Decay
    {1, 1},                         // FMmode: PI/2 auf beiden Stacks
    "Synth Brass III",
    // Double-Stack: breiter Jupiter-/JX-Synthbrass → größte Brass-Verstimmung.
    {9.0f, -9.0f, 11.0f, -10.0f}, // DS_Detune (cents)
    1.0f, 1.10f,                  // DS_MixBase : DS_MixLayer
    {0.80f, 0.80f, 1.0f, 1.0f},   // DS_ATScale
    {1.0f, 1.0f, 1.05f, 1.05f}    // DS_DTScale
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
    1.0f,
    {0, 0, 0, 0},               // DTE: KEIN Decay (Orgel hält Pegel)
    {200, 200, 200, 200},       // RTE
    {0, 0, 0, 0},               // IL
    {255, 255, 255, 255},       // SL: max (unbenutzt bei DTE=0, aber konsistent)
    {0, 0},                     // FMmode
    "Electronic Org I",
    // Double-Stack: Rotary-/Leslie-Chorus durch reine Verstimmung
    // (Orgel hält Pegel, daher keine Envelope-Skalierung).
    {8.0f, -8.0f, 7.0f, -7.0f},  // DS_Detune (cents)
    1.0f, 1.0f,                  // DS_MixBase : DS_MixLayer
    {1.0f, 1.0f, 1.0f, 1.0f},    // DS_ATScale
    {1.0f, 1.0f, 1.0f, 1.0f}     // DS_DTScale
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
    // Gibt dem Klang Präsenz und „Drawbar"-Charakter
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
    1.0f,
    {0, 0, 0, 0},               // DTE: KEIN Decay (Orgel hält Pegel)
    {200, 200, 200, 200},       // RTE
    {0, 0, 0, 0},               // IL
    {255, 255, 255, 255},       // SL: max (unbenutzt bei DTE=0, aber konsistent)
    {0, 0},                     // FMmode
    "Electronic OrgII",
    // Double-Stack: reichere Orgel → etwas breiterer Rotary-Chorus.
    {9.0f, -9.0f, 8.0f, -8.0f},  // DS_Detune (cents)
    1.0f, 1.0f,                  // DS_MixBase : DS_MixLayer
    {1.0f, 1.0f, 1.0f, 1.0f},    // DS_ATScale
    {1.0f, 1.0f, 1.0f, 1.0f}     // DS_DTScale
};


// --- (16) F-5  Pipe Organ ---
// Prinzipal 8' + Oktave 4'. Beide Stacks modulieren die Carrier bei gleicher
// Frequenz (Ratio 1:1) → FM-Index gibt Pfeifencharakter ohne Dissonanz.
// Sofort-Attack (Taste→Ton ohne Verzögerung), volles Sustain, kurzes Release.
const PatchConsts gs1_PipeOrgan = {
    // ---------------------------------------------------------
    // 1) Ratios (C1, C2, M1, M2)
    // ---------------------------------------------------------
    // Stack 1: M1=1× moduliert C1=1×  →  8' Prinzipal
    // Stack 2: M2=2× moduliert C2=2×  →  4' Oktave
    // Gleiche Frequenz Modulator/Carrier = pfeifenartiger FM-Charakter
    {1.0f, 2.0f, 1.0f, 2.0f},

    // ---------------------------------------------------------
    // 2) Detune
    // ---------------------------------------------------------
    // Orgelpfeifen sind präzise gestimmt — minimal Schwebung (Winddruckschwankung)
    {0, 0, 1, -1},

    // ---------------------------------------------------------
    // 3) C1EC — 8' Prinzipal Carrier
    // ---------------------------------------------------------
    // Voll und gleichmäßig über die Tastatur — keine Keyboard-Skalierung
    { 0.0f, 1.0f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 4) C2EC — 4' Oktave Carrier
    // ---------------------------------------------------------
    // Etwas leiser als 8' — klassisches Register-Mischungsverhältnis
    { 0.0f, 0.85f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1 },

    // ---------------------------------------------------------
    // 5) M1EC — Modulator für 8'-Stack (FM-Index)
    // ---------------------------------------------------------
    // Moderater Index (~0.35) → Principal-Charakter: leicht obertonreich,
    // nicht zu flötenartig (→ 0.15) und nicht zu rauschend (→ 0.7).
    // Im Diskant leicht abfallend — hohe Pfeifen klingen reiner.
    { 0.0f, 0.25f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.8,0.6,0.4,0.2,
      0.1,0.05,0.0,0.0 },

    // ---------------------------------------------------------
    // 6) M2EC — Modulator für 4'-Stack (FM-Index)
    // ---------------------------------------------------------
    { 0.0f, 0.30f,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,0.9,0.7,0.5,0.3,0.1,
      0.0,0.0,0.0,0.0 },

    // ---------------------------------------------------------
    // 7) Envelope Attack Times
    // ---------------------------------------------------------
    // Modulatoren (M1/M2) zuerst auf volle Amplitude → FM-Index steht bereit.
    // Carrier (C1/C2) etwas langsamer → verhindert Sample-Diskontinuität (Pop).
    {5000.0f, 5000.0f, 100000.0f, 100000.0f},

    // ---------------------------------------------------------
    // 8) DTE1Scaling & Envelopes
    // ---------------------------------------------------------
    1.0f,
    {500, 500, 500, 500},       // DTE: schnell auf Sustain-Level
    {800, 800, 800, 800},       // RTE: ~25ms Release (Orgel stoppt schnell)
    {0, 0, 0, 0},               // IL
    {240, 240, 240, 240},       // SL: stabiles Sustain bei vollem Pegel
    {0, 0},                     // FMmode: NORM/NORM
    "Pipe Organ",
    // Double-Stack: zweites Register leicht verstimmt → lebendige Pfeifen-
    // Schwebung (Winddruck), aber präziser als die E-Orgeln.
    {7.0f, -7.0f, 6.0f, -6.0f},  // DS_Detune (cents)
    1.0f, 1.0f,                  // DS_MixBase : DS_MixLayer
    {1.0f, 1.0f, 1.0f, 1.0f},    // DS_ATScale
    {1.0f, 1.0f, 1.0f, 1.0f}     // DS_DTScale
};

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
