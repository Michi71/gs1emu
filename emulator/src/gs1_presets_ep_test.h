#pragma once
// ============================================================
// EP-Test-Presets (Layer-Experimente)
// ------------------------------------------------------------
// Zusätzliche E-Piano-Varianten zum Erkunden des Double-Stack-Layers.
// Werden in CGS1Emu hinter den 16 Factory-Presets eingehängt, damit man
// im Standalone weiter durchscrollen kann (Programm 17..20).
//
// Konzept (entspricht der ursprünglichen DualPatch-Idee):
//   Layer A = Hauptklang  → Basis-Stack (Ratio/Detune/ATE/DTE1Scaling)
//   Layer B = Schimmer    → Stack 2 via DS_-Felder, BLEND/Chorus
//
// Alle vier nutzen die EC-Kurven von gs1_ElectricPianoI (per Kopie geerbt),
// und unterscheiden sich nur in Ratios, Detune, Attack-Zeiten und Pegel.
// Die DS_-Übersetzung von Layer B:
//   DS_Ratio      = Ratios von Layer B (leicht verstimmte Modulatoren)
//   DS_Detune     = (Detune_B − Detune_A) in Cent  (DS_Detune ist additiv)
//   DS_ATScale    = ATE_B / ATE_A                   (kürzerer Attack)
//   DS_DTKbdTrack = DTE1Scaling_B / DTE1Scaling_A   (flachere Decay-Verfolgung)
//   DS_MixLayer   = relative Lautstärke des Schimmer-Layers
//   DS_MixMode    = 0 (BLEND/Chorus)
// ============================================================

#include "gs1_presets.h"

// Baut eine EP-Variante: Layer A als Basis, Layer B als DS_-Layer (BLEND).
// EC-Kurven + DTE/RTE/IL/SL/FMmode werden von gs1_ElectricPianoI geerbt.
inline PatchConsts gs1MakeEpDual(
    const char* name,
    // Layer A (Hauptklang):
    float aR0, float aR1, float aR2, float aR3,
    int   aD0, int   aD1, int   aD2, int   aD3,
    float aT0, float aT1, float aT2, float aT3, float aDTE,
    // Layer B (Schimmer):
    float bR0, float bR1, float bR2, float bR3,
    int   bD0, int   bD1, int   bD2, int   bD3,
    float bT0, float bT1, float bT2, float bT3, float bDTE,
    float mixLayer)
{
  PatchConsts p = gs1_ElectricPianoI; // erbt EC-Kurven + Hüllkurven-Raten

  // --- Layer A → Basis-Stack ---
  p.Ratio[0] = aR0; p.Ratio[1] = aR1; p.Ratio[2] = aR2; p.Ratio[3] = aR3;
  p.Detune[0] = aD0; p.Detune[1] = aD1; p.Detune[2] = aD2; p.Detune[3] = aD3;
  p.ATE[0] = aT0; p.ATE[1] = aT1; p.ATE[2] = aT2; p.ATE[3] = aT3;
  p.DTE1Scaling = aDTE;

  // --- Layer B → Stack 2 (DS_, BLEND/Chorus) ---
  p.DS_Ratio[0] = bR0; p.DS_Ratio[1] = bR1; p.DS_Ratio[2] = bR2; p.DS_Ratio[3] = bR3;
  p.DS_Detune[0] = float(bD0 - aD0); p.DS_Detune[1] = float(bD1 - aD1);
  p.DS_Detune[2] = float(bD2 - aD2); p.DS_Detune[3] = float(bD3 - aD3);
  p.DS_ATScale[0] = bT0 / aT0; p.DS_ATScale[1] = bT1 / aT1;
  p.DS_ATScale[2] = bT2 / aT2; p.DS_ATScale[3] = bT3 / aT3;
  p.DS_DTScale[0] = 1.0f; p.DS_DTScale[1] = 1.0f;
  p.DS_DTScale[2] = 1.0f; p.DS_DTScale[3] = 1.0f;
  p.DS_FMmode[0] = -1; p.DS_FMmode[1] = -1;          // erbt NORM von Layer A
  p.DS_LevelDb[0] = 0.0f; p.DS_LevelDb[1] = 0.0f;
  p.DS_LevelDb[2] = 0.0f; p.DS_LevelDb[3] = 0.0f;
  p.DS_ModKbdDb = 0.0f;
  p.DS_DTKbdTrack = (aDTE > 0.0f) ? (bDTE / aDTE) : 1.0f;
  p.DS_MixBase = 1.0f;
  p.DS_MixLayer = mixLayer;
  p.DS_MixMode = 0;                                   // BLEND/Chorus

  // Name kopieren (max. PATCH_NAME_MAX Zeichen + Null)
  int i = 0;
  for (; name[i] != '\0' && i < PATCH_NAME_MAX; ++i) p.Name[i] = name[i];
  for (; i <= PATCH_NAME_MAX; ++i) p.Name[i] = '\0';

  return p;
}

// (17) EP Soft Digital  — weiche, warme Variante, dezenter Schimmer
inline const PatchConsts gs1_EP1_SoftDigital = gs1MakeEpDual(
    "EP Soft Digital",
    1.0f, 1.0f, 2.0f, 4.0f,  0, 4, 2, 3, 1200, 1000, 1800, 700, 1.2f,
    1.0f, 1.0f, 2.0f, 4.0f,  0, 8, 4, 5, 1100,  900, 1600, 600, 0.8f,
    0.80f);

// (18) EP Clear Digital — klarer, etwas heller, breiterer Detune
inline const PatchConsts gs1_EP2_ClearDigital = gs1MakeEpDual(
    "EP Clear Digital",
    1.0f, 1.0f, 2.0f, 4.0f,  0,  6, 3,  5, 1100, 900, 1700, 600, 1.3f,
    1.0f, 1.0f, 2.0f, 4.0f,  0, 12, 8, 10,  900, 700, 1500, 450, 0.9f,
    0.70f);

// (19) EP Hard Digital  — harter Anschlag, kurzer Attack-Spike-Layer
inline const PatchConsts gs1_EP3_HardDigital = gs1MakeEpDual(
    "EP Hard Digital",
    1.0f, 1.0f, 2.0f, 4.0f,  0,  8,  4,  7, 1000, 800, 1600, 500, 1.4f,
    1.0f, 1.0f, 2.0f, 4.0f,  0, 14, 10, 12,  700, 500, 1200, 300, 1.0f,
    0.80f);

// (20) EP GS1 Deluxe    — voller Hauptklang + ausgeprägter Charakter-Layer
inline const PatchConsts gs1_EP4_GS1Deluxe = gs1MakeEpDual(
    "EP GS1 Deluxe",
    1.0f, 1.0f, 2.0f, 4.0f,  0,  5,  3,  4, 1200, 900, 1800, 650, 1.3f,
    1.0f, 1.0f, 2.0f, 4.0f,  0, 14, 10, 12,  800, 600, 1500, 400, 0.85f,
    0.70f);

static constexpr int GS1_EP_TEST_COUNT = 4;

// Lookup-Array der EP-Test-Presets (werden hinter die Factory-Presets gehängt).
static const PatchConsts* const gs1EpTestPresets[GS1_EP_TEST_COUNT] = {
    &gs1_EP1_SoftDigital,   // (17)
    &gs1_EP2_ClearDigital,  // (18)
    &gs1_EP3_HardDigital,   // (19)
    &gs1_EP4_GS1Deluxe      // (20)
};
