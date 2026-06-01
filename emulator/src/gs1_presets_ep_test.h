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

// Baut eine Layer-Variante AUF BASIS eines Familien-Presets.
//
// WICHTIG (Lehre aus dem Re-Voicing): Die Klangfarbe eines GS1-Instruments
// steckt in seinen Operator-RATIOS und Hüllkurven, nicht in generischen
// Spec-Zahlen. Darum erbt dieser Builder die nativen Ratios/Detune/EC-Kurven/
// DTE/RTE/IL/SL/FMmode des Basis-Presets KOMPLETT (1:1) und differenziert die
// Varianten nur über drei musikalisch sinnvolle Achsen:
//
//   attackScale   Faktor auf die native Attack-RATE (ATE ist eine Rate pro
//                 Sample, NICHT eine Zeit!):  <1 = langsamer/weicher Anschlag,
//                 1.0 = nativ,  >1 = schneller/härter.
//   bAttackScale  dito für Layer B (Schimmer) — relativ zur nativen Rate.
//   bDetune       Chorus-Verstimmung von Layer B in Cent (symmetrisch gespreizt).
//   brightness    Pegel-Offset der MODULATOREN (M1,M2) in dB → FM-Index/Timbre.
//                 negativ = dunkler/weicher/"holziger", positiv = heller/Bite.
//                 DIES ist die Hauptachse, um Varianten EINER Familie hörbar zu
//                 trennen (sonst klingen sie im Sustain fast gleich, weil alle
//                 denselben Modulator-Peak erreichen).
//   mixLayer      Mischpegel von Layer B (BLEND/Chorus).
//   decayScale    Faktor auf die native Decay-RATE (DTE) — >1 = SCHNELLERER
//                 Decay (kürzerer, perkussiver Ausklang, z.B. Marimba), 1.0 =
//                 nativ (default). Wirkt auf Basis- und (via Skalierung) Layer.
//
// Layer B erbt die nativen Ratios (DS_Ratio<=0 → Stack-1-Ratio) → reiner,
// verstimmter Chorus auf demselben Klang. So bleibt der Instrumentencharakter
// erhalten und die Doppelschicht erzeugt Breite/Schimmer statt Fremdklang.
inline PatchConsts gs1MakeEpDual(
    const PatchConsts& base,
    const char* name,
    float attackScale,    // Layer A: Faktor auf native ATE-Rate
    float bAttackScale,   // Layer B: Faktor auf native ATE-Rate
    float bDetune,        // Layer B: Chorus-Verstimmung in Cent
    float brightness,     // Modulator-Pegel (M1,M2) in dB (Timbre/Helligkeit)
    float mixLayer,       // Layer-B-Mischpegel
    float outLevelDb = 0.0f, // Preset-Ausgangspegel in dB (Lautstärke-Angleich)
    float decayScale = 1.0f) // Faktor auf native Decay-Rate (DTE); >1 = schneller
{
  PatchConsts p = base; // erbt ALLES (Ratio, Detune, EC, ATE, DTE, RTE, IL, SL, FMmode)

  // --- Layer A: native Attack-RATE skalieren + Modulator-Helligkeit setzen ---
  for (int i = 0; i < 4; ++i) p.ATE[i] = base.ATE[i] * attackScale;
  p.BaseLevelDb[2] = brightness; p.BaseLevelDb[3] = brightness; // M1,M2 = Timbre
  p.OutLevelDb = outLevelDb;                                    // Ausgangspegel
  // --- Decay-Rate skalieren (kürzerer Ausklang bei decayScale > 1) ---
  if (decayScale != 1.0f)
    for (int i = 0; i < 4; ++i) {
      int d = int(base.DTE[i] * decayScale + 0.5f);
      p.DTE[i] = d < 1 ? 1 : d;
    }

  // --- Layer B → Stack 2: reiner Chorus auf geerbten Ratios ---
  for (int i = 0; i < 4; ++i) p.DS_Ratio[i] = 0.0f;  // <=0 → erbt native Ratio
  // Symmetrisch gespreizte Verstimmung → lebendiges Schweben gegen die Basis.
  p.DS_Detune[0] = +bDetune;        p.DS_Detune[1] = -bDetune;
  p.DS_Detune[2] = +bDetune * 1.2f; p.DS_Detune[3] = -bDetune * 1.2f;
  // AT2 = AT * DS_ATScale = (nativeATE*attackScale) * (bAttackScale/attackScale)
  //     = nativeATE * bAttackScale  → Layer-B-Attack relativ zur nativen Rate.
  float atRel = (attackScale != 0.0f) ? (bAttackScale / attackScale) : 1.0f;
  for (int i = 0; i < 4; ++i) { p.DS_ATScale[i] = atRel; p.DS_DTScale[i] = 1.0f; }
  p.DS_FMmode[0] = -1; p.DS_FMmode[1] = -1;          // erbt NORM von Layer A
  for (int i = 0; i < 4; ++i) p.DS_LevelDb[i] = 0.0f;
  p.DS_LevelDb[2] = brightness; p.DS_LevelDb[3] = brightness; // Chorus klanglich angleichen
  p.DS_ModKbdDb   = 0.0f;
  p.DS_DTKbdTrack = 1.0f;                            // folgt der Basis-Decay-Verfolgung
  p.DS_MixBase    = 1.0f;
  p.DS_MixLayer   = mixLayer;
  p.DS_MixMode    = 0;                               // BLEND/Chorus

  // Name kopieren (max. PATCH_NAME_MAX Zeichen + Null)
  int i = 0;
  for (; name[i] != '\0' && i < PATCH_NAME_MAX; ++i) p.Name[i] = name[i];
  for (; i <= PATCH_NAME_MAX; ++i) p.Name[i] = '\0';

  return p;
}

//                              base,             name,            aAtt  bAtt  bDet  bright  mix    out
// (17) EP Soft Digital  — weicher Anschlag, dunkles/warmes Timbre
inline const PatchConsts gs1_EP1_SoftDigital = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Soft Digital",   0.55f, 0.70f,  8.0f, -8.0f, 0.80f, 4.0f);

// (18) EP Clear Digital — klarer Anschlag, mittlere Helligkeit
inline const PatchConsts gs1_EP2_ClearDigital = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Clear Digital",  0.80f, 0.95f, 12.0f, -3.0f, 0.70f, 4.0f);

// (19) EP Hard Digital  — harter, schneller, heller Anschlag (viel Bite)
inline const PatchConsts gs1_EP3_HardDigital = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Hard Digital",   1.20f, 1.40f, 16.0f, +3.0f, 0.80f, 4.0f);

// (20) EP GS1 Deluxe    — voll, ausgewogen, breiter Chorus, leicht gedämpft
inline const PatchConsts gs1_EP4_GS1Deluxe = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP GS1 Deluxe",     0.95f, 1.10f, 14.0f, -2.0f, 0.70f, 4.0f);

static constexpr int GS1_EP_TEST_COUNT = 4;

// Lookup-Array der EP-Test-Presets (werden hinter die Factory-Presets gehängt).
static const PatchConsts* const gs1EpTestPresets[GS1_EP_TEST_COUNT] = {
    &gs1_EP1_SoftDigital,   // (17)
    &gs1_EP2_ClearDigital,  // (18)
    &gs1_EP3_HardDigital,   // (19)
    &gs1_EP4_GS1Deluxe      // (20)
};
