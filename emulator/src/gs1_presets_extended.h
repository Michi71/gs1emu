#pragma once
// ============================================================
// GS1 Extended Preset Pack (20 Presets)
// ------------------------------------------------------------
// Erweitert die 16 Factory-Presets um ein durchdachtes 20er-Pack, das
// die Double-Stack-Engine voll ausnutzt: Layer A = GS1-typischer warmer
// Hauptklang, Layer B = leicht verstimmter Schimmer (BLEND/Chorus).
//
// Designziele:
//   * harmonisch saubere Ratios (1/1/2/4, 1/1/3/4, 1/1/2/3) → keine
//     DX7-typischen Inharmonik-Artefakte
//   * Layer B nur leicht im Detune verschoben → Chorus-Breite ohne Schwebung
//   * GS1-typische Dumpfheit/Wärme durch die geerbten EC-Kurven der jeweils
//     passenden Klangfamilie (sanfte Höhen-Dämpfung über die Tastatur):
//       EP1–EP8 → gs1_ElectricPianoI
//       GP1–GP4 → gs1_AcousticPianoI
//       VB1–VB4 → gs1_Vibraphone
//       MB1–MB4 → gs1_Vibraphone  (Holz-Pluck nutzt Vibraphon-Grundwerte)
//
// Alle Presets werden über gs1MakeEpDual(...) gebaut (siehe
// gs1_presets_ep_test.h) und sind damit 1:1 in dieselbe Struktur
// übernehmbar. Layout im Standalone:
//
//   (1)–(16)  Factory-Presets
//   (17)–(24) Electric Pianos   EP1–EP8
//   (25)–(28) Grand Pianos      GP1–GP4
//   (29)–(32) Vibraphone/Glocken VB1–VB4
//   (33)–(36) Marimba/Pluck      MB1–MB4
//
// gs1MakeEpDual-Signatur (native Ratios/Hüllkurven werden 1:1 von base geerbt):
//   gs1MakeEpDual(base, name, attackScale, bAttackScale, bDetune, mixLayer)
//     attackScale  : Faktor auf native Attack-RATE (Layer A) — <1 weicher, >1 härter
//     bAttackScale : dito für Layer B (Schimmer)
//     bDetune      : Chorus-Verstimmung Layer B in Cent
//     mixLayer     : Mischpegel Layer B (BLEND)
// ============================================================

#include "gs1_presets_ep_test.h"   // gs1MakeEpDual() + EP1–EP4 (17)–(20)

//                        base,             name,           aAtt  bAtt  bDet  bright  mix
// ------------------------------------------------------------
// Kategorie 1 — Electric Pianos (Fortsetzung: EP5–EP8)
// Basis: gs1_ElectricPianoI  (native Ratio 1/1/1/1 + Detune → Rhodes-EP)
// ------------------------------------------------------------

// (21) EP Warm Pad — sehr weicher Anschlag, sehr dunkel/flächig
inline const PatchConsts gs1_EP5_WarmPad = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Warm Pad",     0.45f, 0.55f,  8.0f, -11.0f, 0.65f, 4.0f);

// (22) EP Bellish — heller, klarer Anschlag (Glocken-Anmutung)
inline const PatchConsts gs1_EP6_Bellish = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Bellish",      0.90f, 1.10f, 14.0f,  +4.0f, 0.75f, 4.0f);

// (23) EP Chorus Wide — sehr breiter Chorus, mittlere Helligkeit
inline const PatchConsts gs1_EP7_ChorusWide = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Chorus Wide",  0.80f, 0.95f, 20.0f,  -5.0f, 0.85f, 4.0f);

// (24) EP Vintage GS1 — dumpf, warm, langsamer Anschlag
inline const PatchConsts gs1_EP8_VintageGS1 = gs1MakeEpDual(
    gs1_ElectricPianoI, "EP Vintage GS1",  0.50f, 0.60f,  6.0f,  -9.0f, 0.55f, 4.0f);

// ------------------------------------------------------------
// Kategorie 2 — Grand Pianos (GP1–GP4)
// Basis: gs1_AcousticPianoI  (native Ratio 1/2/1/4 → Klavierkörper + Hammer)
// ------------------------------------------------------------

// (25) GP Soft Grand — weicher Hammer, dunkleres Timbre
inline const PatchConsts gs1_GP1_SoftGrand = gs1MakeEpDual(
    gs1_AcousticPianoI, "GP Soft Grand",   0.55f, 0.70f,  8.0f, -7.0f, 0.65f);

// (26) GP Clear Grand — klarer, definierter Anschlag
inline const PatchConsts gs1_GP2_ClearGrand = gs1MakeEpDual(
    gs1_AcousticPianoI, "GP Clear Grand",  0.80f, 0.95f, 12.0f, -3.0f, 0.75f);

// (27) GP Hammer Grand — harter, heller Hammer-Anschlag
inline const PatchConsts gs1_GP3_HammerGrand = gs1MakeEpDual(
    gs1_AcousticPianoI, "GP Hammer Grand", 1.20f, 1.40f, 16.0f, +3.0f, 0.85f);

// (28) GP GS1 Deluxe — voll, ausgewogen, breit
inline const PatchConsts gs1_GP4_GS1Deluxe = gs1MakeEpDual(
    gs1_AcousticPianoI, "GP GS1 Deluxe",   0.95f, 1.10f, 14.0f, -2.0f, 0.80f);

// ------------------------------------------------------------
// Kategorie 3 — Vibraphone & Glocken (VB1–VB4)
// Basis: gs1_Vibraphone  (native Ratio 1/1/8/8 → metallisches Schimmern).
// WICHTIG: 8×-Modulatoren sind alias-anfällig — Helligkeit auf <=0 dB deckeln
// und Detune moderat halten, sonst entstehen Verzerrungen/Alias-Artefakte.
// ------------------------------------------------------------

// (29) VB Soft Vibes — weicher Mallet, runder Metallklang
inline const PatchConsts gs1_VB1_SoftVibes = gs1MakeEpDual(
    gs1_Vibraphone, "VB Soft Vibes",       0.55f, 0.70f,  5.0f, -10.0f, 0.75f);

// (30) VB Clear Vibes — klarer Metall-Attack
inline const PatchConsts gs1_VB2_ClearVibes = gs1MakeEpDual(
    gs1_Vibraphone, "VB Clear Vibes",      0.80f, 0.95f,  7.0f,  -6.0f, 0.85f);

// (31) VB Hard Vibes — harter, metallischer Ping (ohne Alias-Verzerrung)
inline const PatchConsts gs1_VB3_HardVibes = gs1MakeEpDual(
    gs1_Vibraphone, "VB Hard Vibes",       1.20f, 1.40f,  9.0f,  -2.0f, 0.90f);

// (32) VB GS1 Deluxe — voll, metallisch, breit
inline const PatchConsts gs1_VB4_GS1Deluxe = gs1MakeEpDual(
    gs1_Vibraphone, "VB GS1 Deluxe",       0.95f, 1.10f,  7.0f,  -4.0f, 0.80f);

// ------------------------------------------------------------
// Kategorie 4 — Marimba / Pluck (MB1–MB4)
// Basis: gs1_ClavichordII  (native Ratio 1/1/3/4 → holziger Pluck).
// Modulatoren STARK gedämpft (wenig Obertöne = holzig) UND decayScale ~8 →
// kurzer, perkussiver Holz-Anschlag mit schnell fallender Kurve (kein EP-Sustain).
//                              base,            name,            aAtt  bAtt  bDet  bright   mix   out   decay
// ------------------------------------------------------------

// (33) MB Soft Marimba — weicher, sehr holziger, kurzer Anschlag
inline const PatchConsts gs1_MB1_SoftMarimba = gs1MakeEpDual(
    gs1_ClavichordII, "MB Soft Marimba",   0.55f, 0.70f,  8.0f, -13.0f, 0.70f, 0.0f, 7.0f);

// (34) MB Clear Marimba — klarer, kurzer Holz-Pluck
inline const PatchConsts gs1_MB2_ClearMarimba = gs1MakeEpDual(
    gs1_ClavichordII, "MB Clear Marimba",  0.80f, 0.95f, 12.0f, -10.0f, 0.80f, 0.0f, 8.0f);

// (35) MB Hard Marimba — harter, sehr kurzer Holz-Schlag
inline const PatchConsts gs1_MB3_HardMarimba = gs1MakeEpDual(
    gs1_ClavichordII, "MB Hard Marimba",   1.20f, 1.40f, 16.0f,  -6.0f, 0.85f, 0.0f, 10.0f);

// (36) MB GS1 Deluxe — voll, holzig, kurz, breit
inline const PatchConsts gs1_MB4_GS1Deluxe = gs1MakeEpDual(
    gs1_ClavichordII, "MB GS1 Deluxe",     0.95f, 1.10f, 14.0f,  -9.0f, 0.75f, 0.0f, 8.0f);

// ============================================================
// Lookup-Array des Extended-Packs (20 Presets, (17)–(36)).
// Wird in CGS1Emu hinter die 16 Factory-Presets gehängt.
// ============================================================
static constexpr int GS1_EXTENDED_COUNT = 20;

static const PatchConsts* const gs1ExtendedPresets[GS1_EXTENDED_COUNT] = {
    // Electric Pianos (17)–(24)
    &gs1_EP1_SoftDigital,   // (17)
    &gs1_EP2_ClearDigital,  // (18)
    &gs1_EP3_HardDigital,   // (19)
    &gs1_EP4_GS1Deluxe,     // (20)
    &gs1_EP5_WarmPad,       // (21)
    &gs1_EP6_Bellish,       // (22)
    &gs1_EP7_ChorusWide,    // (23)
    &gs1_EP8_VintageGS1,    // (24)
    // Grand Pianos (25)–(28)
    &gs1_GP1_SoftGrand,     // (25)
    &gs1_GP2_ClearGrand,    // (26)
    &gs1_GP3_HammerGrand,   // (27)
    &gs1_GP4_GS1Deluxe,     // (28)
    // Vibraphone/Glocken (29)–(32)
    &gs1_VB1_SoftVibes,     // (29)
    &gs1_VB2_ClearVibes,    // (30)
    &gs1_VB3_HardVibes,     // (31)
    &gs1_VB4_GS1Deluxe,     // (32)
    // Marimba/Pluck (33)–(36)
    &gs1_MB1_SoftMarimba,   // (33)
    &gs1_MB2_ClearMarimba,  // (34)
    &gs1_MB3_HardMarimba,   // (35)
    &gs1_MB4_GS1Deluxe      // (36)
};
