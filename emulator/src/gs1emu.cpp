#include "gs1emu.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// Constants
static int SampleRate = 34687; // 34687Hz Samplerate (IMPORTANT!)
// Note! Sinewave phase resolution has to be 10bit for quantization error to
// give right sound!

static int maxDelaySamples = 512;

static double PI = 3.1415927;
static double HALF_PI = 1.5707964;
constexpr float TWO_PI = 6.2831853f;

// Tables
static int logsinTable[256];
static int expTable[256];
static int expTable2[4096];

// Ziel: LFO1 ca. 0.3 Hz (Slow), LFO2 ca. 3.1 Hz (Fast)
const float lfo1Inc = (0.313f * 65536.0f) / SampleRate; 
const float lfo2Inc = (3.121f * 65536.0f) / SampleRate;

static double map(double x, double in_min, double in_max, double out_min,
                  double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static int lookupSin(int val) {
  bool signsin = (bool)(val & 512);
  bool mirrorsin = (bool)(val & 256);
  val &= 255;
  int result = logsinTable[mirrorsin ? val ^ 255 : val ^ 0];
  if (signsin) {
    result |= 0x8000;
  }
  return result;
}

static int lookupExp(int val) {
  bool signexp = (bool)(val & 0x8000);
  int t = (expTable[(val & 255) ^ 255] | (32768)) << 1;
  int result = t >> ((val & 0x7F00) >> 8);
  if (signexp) {
    result = -result - 1;
  }
  return result >> 4;
}

static int renderStackPair(const int pae[4], const int amp[4], const int fmMode[2],
                           int& m1, int& m2,
                           int& m1old1, int& m1old2,
                           int& m2old1, int& m2old2,
                           int& ch1, int& ch2) {
  if (fmMode[0] == 0) {
    if (amp[2] <= 4094) {
      m1 = ((lookupExp(lookupSin(pae[2]) + amp[2]) + 8192) >> 2) & 1023;
    } else {
      m1 = 4095;
    }
    if (amp[0] <= 4094) {
      ch1 = lookupExp(lookupSin(pae[0] + m1) + amp[0]);
    } else {
      ch1 = 0;
    }
  }
  if (fmMode[0] == 1) {
    if (amp[2] <= 4094) {
      m1 = (lookupExp(lookupSin(pae[2] + (((m1old1 + m1old2) / 2) >> 3)) + amp[2]) + 8192) >> 4;
    } else {
      m1 = 4095;
    }
    m1old2 = m1old1;
    m1old1 = m1;
    if (amp[0] <= 4094) {
      ch1 = lookupExp(lookupSin(pae[0] + m1) + amp[0]);
    } else {
      ch1 = 0;
    }
  }
  if (fmMode[0] == 2) {
    if (amp[2] <= 4094) {
      m1 = (lookupExp(lookupSin(pae[2] + (((m1old1 + m1old2) / 2) >> 2)) + amp[2]) + 8192) >> 4;
    } else {
      m1 = 4095;
    }
    m1old2 = m1old1;
    m1old1 = m1;
    if (amp[0] <= 4094) {
      ch1 = lookupExp(lookupSin(pae[0] + m1) + amp[0]);
    } else {
      ch1 = 0;
    }
  }
  if (fmMode[0] == 3) {
    if (amp[2] <= 4094) {
      m1 = ((lookupExp(lookupSin(pae[2] + m2) + amp[2]) + 8192) >> 2) & 1023;
    } else {
      m1 = 4095;
    }
    if (amp[0] <= 4094) {
      ch1 = lookupExp(lookupSin(pae[0] + m1) + amp[0]);
    } else {
      ch1 = 0;
    }
  }

  if (fmMode[1] == 0) {
    if (amp[3] <= 4094) {
      m2 = ((lookupExp(lookupSin(pae[3]) + amp[3]) + 8192) >> 2) & 1023;
    } else {
      m2 = 4095;
    }
    if (amp[1] <= 4094) {
      ch2 = lookupExp(lookupSin(pae[1] + m2) + amp[1]);
    } else {
      ch2 = 0;
    }
  }
  if (fmMode[1] == 1) {
    if (amp[3] <= 4094) {
      m2 = (lookupExp(lookupSin(pae[3] + (((m2old1 + m2old2) / 2) >> 3)) + amp[3]) + 8192) >> 4;
    } else {
      m2 = 4095;
    }
    m2old2 = m2old1;
    m2old1 = m2;
    if (amp[1] <= 4094) {
      ch2 = lookupExp(lookupSin(pae[1] + m2) + amp[1]);
    } else {
      ch2 = 0;
    }
  }
  if (fmMode[1] == 2) {
    if (amp[3] <= 4094) {
      m2 = (lookupExp(lookupSin(pae[3] + (((m2old1 + m2old2) / 2) >> 2)) + amp[3]) + 8192) >> 4;
    } else {
      m2 = 4095;
    }
    m2old2 = m2old1;
    m2old1 = m2;
    if (amp[1] <= 4094) {
      ch2 = lookupExp(lookupSin(pae[1] + m2) + amp[1]);
    } else {
      ch2 = 0;
    }
  }
  if (fmMode[1] == 3) {
    if (amp[3] <= 4094) {
      m2 = ((lookupExp(lookupSin(pae[3] + m1) + amp[3]) + 8192) >> 2) & 1023;
    } else {
      m2 = 4095;
    }
    if (amp[1] <= 4094) {
      ch2 = lookupExp(lookupSin(pae[1] + m2) + amp[1]);
    } else {
      ch2 = 0;
    }
  }

  return ch1 + ch2;
}

// Envelope-Zustandsmaschine für EINEN Operator. Arbeitet auf den übergebenen
// Arrays, damit Stack 1 und Stack 2 dieselbe Logik mit eigenem State nutzen.
static void stepEnvelope(int e, int gate, int gateOld, int mode,
                         int32_t* EA, int32_t* EAx, int32_t* EAo,
                         int32_t* RS, int32_t* RSx, int* STATE,
                         const float* AT, const float* DT, const float* RT,
                         const int* IL, const int* SL) {
  if (mode != 0) return;
  if (gateOld == 0 && gate == 1) {
    STATE[e] = 1;
    EA[e] = IL[e] << 12;
  }
  if (gate == 1 && STATE[e] == 1 && EA[e] < 0xFFFFF) {
    EA[e] = EA[e] + AT[e];
    EAx[e] = EA[e];
    if (EA[e] > 0xFFFFF) EA[e] = 0xFFFFF;
  }
  if (gate == 1 && STATE[e] == 1 && EA[e] >= 0xFFFFF) {
    STATE[e] = 2;
  }
  if (gate == 1 && STATE[e] == 2 && EA[e] > SL[e] << 12) {
    EA[e] = EA[e] - DT[e];
    if (EA[e] < SL[e] << 12) EA[e] = SL[e] << 12;
    EAx[e] = int(map(expTable2[int(map(EA[e], SL[e] << 12, 0xFFFFF, 0, 4095))],
                     0, 4095, SL[e] << 12, 0xFFFFF));
  }
  if (gate == 0 && gateOld == 1) {
    RS[e] = EA[e];
    RSx[e] = EAx[e];
  }
  if (gate == 0 && EA[e] > 0) {
    EA[e] = EA[e] - RT[e];
    STATE[e] = 0;
    if (EA[e] <= 0) EA[e] = 0;
    if (RS[e] > 0) {
      EAx[e] = int(map(expTable2[int(map(EA[e], 0, floor(RS[e]), 0, 4095))],
                       0, 4095, 0, RSx[e]));
    } else {
      EAx[e] = 0;
    }
  }
  if (gate == 0 && EAo[e] < (int(EA[e]) & 0xFFFFF)) {
    EA[e] = 0;
  }
  EAo[e] = EA[e];
}

// Operator-Amplitude aus log-domain Envelope (EAx), Keyboard-Scaling (eg) und
// Velocity. Geclamped auf 4095 (entspricht "stumm" in renderStackPair).
static inline int calcAmp(int32_t eax, int eg, int velocity) {
  int a = (((((int)eax) >> 8) ^ 4095) & 4095) + eg + (velocity << 3);
  return a >= 4095 ? 4095 : a;
}

int CGS1Emu::findVoice()
{
    // 1. Stille Stimme (vollständig inaktiv)
    for (int v = 0; v < MAXVOICES; ++v) {
        VoiceState& vs = voiceStates[v];
        if (!vs.noteOn && vs.EA[0] == 0 && vs.EA[1] == 0 && vs.EA[2] == 0 && vs.EA[3] == 0)
            return v;
    }
    // 2. Leiseste Stimme im Release (GATE == 0)
    int bestRelease = -1;
    int32_t lowestEnergy = INT32_MAX;
    for (int v = 0; v < MAXVOICES; ++v) {
        VoiceState& vs = voiceStates[v];
        if (!vs.noteOn && !vs.sustaining) {
            int32_t energy = vs.EA[0] + vs.EA[1] + vs.EA[2] + vs.EA[3];
            if (energy < lowestEnergy) {
                lowestEnergy = energy;
                bestRelease = v;
            }
        }
    }
    if (bestRelease >= 0)
        return bestRelease;
    // 3. Älteste aktive Stimme stehlen
    int oldest = 0;
    for (int v = 1; v < MAXVOICES; ++v) {
        if (voiceStates[v].noteAge < voiceStates[oldest].noteAge)
            oldest = v;
    }
    return oldest;
}

// --- Detune ---
void CGS1Emu::setDetuneMode(DetuneMode mode) { detuneMode = mode; }
DetuneMode CGS1Emu::getDetuneMode() const    { return detuneMode; }

void CGS1Emu::setDoubleStacksOn(bool on)     { doubleStacksOn = on; }
bool CGS1Emu::getDoubleStacksOn() const      { return doubleStacksOn; }

// --- Tremolo ---
void  CGS1Emu::setTremoloOn(bool on)           { tremoloOn = on; }
bool  CGS1Emu::getTremoloOn() const            { return tremoloOn; }
void  CGS1Emu::setTremoloSpeed(float hz)       { tremoloSpeed = hz; tremoloInc = hz * TWO_PI / (float)SampleRate; }
float CGS1Emu::getTremoloSpeed() const         { return tremoloSpeed; }
void  CGS1Emu::setTremoloDepth(float depth)    { tremoloDepth = depth; }
float CGS1Emu::getTremoloDepth() const         { return tremoloDepth; }

// --- Vibrato ---
void  CGS1Emu::setVibratoOn(bool on)           { vibratoOn = on; }
bool  CGS1Emu::getVibratoOn() const            { return vibratoOn; }
void  CGS1Emu::setVibratoSpeed(float hz)       { vibratoSpeed = hz; vibratoInc = hz * TWO_PI / (float)SampleRate; }
float CGS1Emu::getVibratoSpeed() const         { return vibratoSpeed; }
void  CGS1Emu::setVibratoDepth(float depth)    { vibratoDepth = depth; }
float CGS1Emu::getVibratoDepth() const         { return vibratoDepth; }

// --- 3-Band EQ ---
void  CGS1Emu::setEqBass(float dB) {
    eqBassGain = std::max(-12.0f, std::min(12.0f, dB));
    _eqBass.setLowShelf(100.0f, eqBassGain, (float)SampleRate);
}
float CGS1Emu::getEqBass() const   { return eqBassGain; }

void  CGS1Emu::setEqMid(float dB) {
    eqMidGain = std::max(-12.0f, std::min(12.0f, dB));
    _eqMid.setPeaking(600.0f, 1.0f, eqMidGain, (float)SampleRate);
}
float CGS1Emu::getEqMid() const    { return eqMidGain; }

void  CGS1Emu::setEqTreble(float dB) {
    eqTrebleGain = std::max(-12.0f, std::min(12.0f, dB));
    _eqTreble.setHighShelf(6000.0f, eqTrebleGain, (float)SampleRate);
}
float CGS1Emu::getEqTreble() const { return eqTrebleGain; }

void  CGS1Emu::setMasterVolume(float v) {
    masterVolume = v < 0.0f ? 0.0f : (v > 2.0f ? 2.0f : v);
}
float CGS1Emu::getMasterVolume() const { return masterVolume; }

int CGS1Emu::getNumPrograms() { return GS1_NUM_PROGRAMS; }

int CGS1Emu::getCurrentProgram() { return currentPatch; }

const char* CGS1Emu::getProgramName(int index) const {
  if (index < 0 || index >= GS1_NUM_PROGRAMS) return "";
  return patches[index]->Name;
}

void CGS1Emu::setCurrentProgram(int index) {
  if (index < 0) index = 0;
  else if (index >= GS1_NUM_PROGRAMS) index = GS1_NUM_PROGRAMS - 1;
  currentPatch = index;
}

void CGS1Emu::setEnsembleOn(bool ensonoff) { ensembleOn = ensonoff; }
bool CGS1Emu::getEnsembleOn() { return ensembleOn; }

void CGS1Emu::noteOn(VoiceState &voiceState, int voiceIndex, float KNOTE,
                     float Velocity) {
  const PatchConsts& patch = *patches[currentPatch];

  voiceState.noteOn = true;
  // Patch-Pointer pro Stimme festhalten: ein Programmwechsel während noch
  // klingender Noten darf deren Mix-/Layer-Parameter nicht rückwirkend ändern.
  voiceState.srcPatch = &patch;
  // Beim (Wieder-)Anschlagen Sustain-Status zurücksetzen — sonst erbt eine
  // gestohlene, noch sustainende Stimme das Flag und hängt beim Loslassen.
  voiceState.sustaining = false;
  voiceState.midiNote = KNOTE;

  voiceState.KNOTE = KNOTE;
  voiceState.Velocity = Velocity;
  for (int i = 0; i < 4; i++) {
    voiceState.STATE[i] = 0;
    voiceState.EA[i] = 0;  // 16bit
    voiceState.EAx[i] = 0; // 16bit
    voiceState.EAo[i] = 0; // oldaccu
    voiceState.RS[i] = 0;
    voiceState.RSx[i] = 0;
    voiceState.STATE2[i] = 0;
    voiceState.EA2[i] = 0;
    voiceState.EAx2[i] = 0;
    voiceState.EAo2[i] = 0;
    voiceState.RS2[i] = 0;
    voiceState.RSx2[i] = 0;
    voiceState.GATE = 0;
    voiceState.GATEOLD = 0;
    voiceState.Mode = 0;
  }
  for (int i = 0; i < 4; i++) {
    voiceState.PAI[i] = 0;
    voiceState.PAE[i] = 0;
    voiceState.PAI2[i] = 0;
    voiceState.PAE2[i] = 0;
  }
  voiceState.KNOTE = voiceState.KNOTE - 1;
  voiceState.NOTE = 27.50 * pow(2, (voiceState.KNOTE / (12))); // FROM A1
  switch (detuneMode) {
    case DetuneMode::RANDOM1:
      voiceState.rnd = ((float)(rand() % 201) - 100) * 0.0003f; // ±3 Cent
      break;
    case DetuneMode::RANDOM2:
      voiceState.rnd = ((float)(rand() % 201) - 100) * 0.0008f; // ±8 Cent
      break;
    case DetuneMode::STATIC1:
      voiceState.rnd = staticOffsets1[voiceIndex % MAXVOICES];
      break;
    case DetuneMode::STATIC2:
      voiceState.rnd = staticOffsets2[voiceIndex % MAXVOICES];
      break;
    case DetuneMode::OFF:
    default:
      voiceState.rnd = 0.0f;
      break;
  }

  for (int i = 0; i < 4; i++) {
    voiceState.DTE[i] = patch.DTE[i];
    voiceState.RTE[i] = patch.RTE[i];
    // IL/SL auf [0,255] klemmen: stepEnvelope rechnet mit (SL<<12) als Grenze
    // im Bereich bis 0xFFFFF. 256<<12 würde 0xFFFFF erreichen/überschreiten und
    // die Envelope-Map degenerieren lassen. 255<<12 bleibt sicher darunter.
    int il = patch.IL[i]; il = il < 0 ? 0 : (il > 255 ? 255 : il);
    int sl = patch.SL[i]; sl = sl < 0 ? 0 : (sl > 255 ? 255 : sl);
    voiceState.IL[i]  = il;
    voiceState.SL[i]  = sl;
  }
  
  voiceState.FMmode[0] = patch.FMmode[0];
  voiceState.FMmode[1] = patch.FMmode[1];
  // Stack-2-FM-Topologie: DS_FMmode[i] < 0 → erbt von Stack 1, sonst überschreiben.
  voiceState.FMmode2[0] = patch.DS_FMmode[0] < 0 ? patch.FMmode[0] : patch.DS_FMmode[0];
  voiceState.FMmode2[1] = patch.DS_FMmode[1] < 0 ? patch.FMmode[1] : patch.DS_FMmode[1];
  // Stack-2-Pegel-Offset: dB → Attenuation-Domain (~42.67 Units/dB).
  // Negatives dB (leiser) ergibt positiven Attenuation-Offset.
  for (int i = 0; i < 4; i++)
    voiceState.dsLevelOff[i] = int(-patch.DS_LevelDb[i] * 42.6666667f);
  // Basis-Stack-Pegel-Offset (BaseLevelDb): wirkt immer, auch ohne Double-Stack.
  // Modulatoren (idx 2,3) → FM-Index/Helligkeit; Carrier (idx 0,1) → Lautstärke.
  for (int i = 0; i < 4; i++)
    voiceState.baseLevelOff[i] = int(-patch.BaseLevelDb[i] * 42.6666667f);
  // Preset-Ausgangspegel (OutLevelDb) → linearer Gain (powf nur hier im NoteOn).
  voiceState.outGain = powf(10.0f, patch.OutLevelDb / 20.0f);
  // Keyboard-Rolloff der Layer-Modulatoren (M1=idx2, M2=idx3): oberhalb C2
  // (KNOTE 15) den FM-Index pro Oktave um DS_ModKbdDb dB zurücknehmen → der
  // Anschlag wird zu hohen Tönen hin weniger hell/metallisch.
  {
    float octAboveC2 = (voiceState.KNOTE - 15) / 12.0f;
    if (octAboveC2 > 0.0f && patch.DS_ModKbdDb > 0.0f) {
      int kbdAtten = int(patch.DS_ModKbdDb * octAboveC2 * 42.6666667f);
      voiceState.dsLevelOff[2] += kbdAtten;
      voiceState.dsLevelOff[3] += kbdAtten;
    }
  }

  // Calculate phase accumulator control words.
  voiceState.CW[0] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[0] * 0.01)) /
                                             (12)) *
                                  patch.Ratio[0])));
  voiceState.CW[1] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[1] * 0.01)) /
                                             (12)) *
                                  patch.Ratio[1])));
  voiceState.CW[2] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[2] * 0.01)) /
                                             (12)) *
                                  patch.Ratio[2])));
  voiceState.CW[3] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[3] * 0.01)) /
                                             (12)) *
                                  patch.Ratio[3])));

  // Stack-2-Verstimmung: patch-spezifisch in Cent (DS_Detune), → Semitöne.
  float stack2DetuneSemitones[4] = {
      patch.DS_Detune[0] * 0.01f, patch.DS_Detune[1] * 0.01f,
      patch.DS_Detune[2] * 0.01f, patch.DS_Detune[3] * 0.01f};
  // Effektive Operator-Ratios für Stack 2:
  //   DS_Ratio[i] > 0  → eigene Ratio (echter Layer, andere Klangfarbe)
  //   sonst            → erbt Ratio[] von Stack 1 (Chorus-Verhalten, default)
  float r2[4];
  for (int i = 0; i < 4; i++)
    r2[i] = patch.DS_Ratio[i] > 0.0f ? patch.DS_Ratio[i] : float(patch.Ratio[i]);
  voiceState.CW2[0] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[0] * 0.01) + stack2DetuneSemitones[0]) /
                                             (12)) *
                                  r2[0])));
  voiceState.CW2[1] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[1] * 0.01) + stack2DetuneSemitones[1]) /
                                             (12)) *
                                  r2[1])));
  voiceState.CW2[2] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[2] * 0.01) + stack2DetuneSemitones[2]) /
                                             (12)) *
                                  r2[2])));
  voiceState.CW2[3] = int(
      pow(2, 28) / (SampleRate / (27.50 *
                                  pow(2, (voiceState.KNOTE + voiceState.rnd +
                                          (patch.Detune[3] * 0.01) + stack2DetuneSemitones[3]) /
                                             (12)) *
                                  r2[3])));

  // in perc mode recalc envelope times depending on note.
  voiceState.AT[0] = patch.ATE[0] * map((voiceState.KNOTE + 1), 1, 88, 1, 4);
  voiceState.AT[1] = patch.ATE[1] * map((voiceState.KNOTE + 1), 1, 88, 1, 4);
  voiceState.AT[2] = patch.ATE[2] * map((voiceState.KNOTE + 1), 1, 88, 1, 4);
  voiceState.AT[3] = patch.ATE[3] * map((voiceState.KNOTE + 1), 1, 88, 1, 4);

  voiceState.DT[0] = voiceState.DTE[0] * map((voiceState.KNOTE + 1), 1, 88, 0.5, 3);
  voiceState.DT[1] =
      voiceState.DTE[1] * map((voiceState.KNOTE + 1), 1, 88, 0.5, patch.DTE1Scaling);
  voiceState.DT[2] = voiceState.DTE[2] * map((voiceState.KNOTE + 1), 1, 88, 0.5, 3);
  voiceState.DT[3] = voiceState.DTE[3] * map((voiceState.KNOTE + 1), 1, 88, 0.5, 3);
  voiceState.RT[0] = voiceState.RTE[0] * map((voiceState.KNOTE + 1), 1, 88, 1, 2);
  voiceState.RT[1] = voiceState.RTE[1] * map((voiceState.KNOTE + 1), 1, 88, 1, 2);
  voiceState.RT[2] = voiceState.RTE[2] * map((voiceState.KNOTE + 1), 1, 88, 1, 2);
  voiceState.RT[3] = voiceState.RTE[3] * map((voiceState.KNOTE + 1), 1, 88, 1, 2);

  // Stack-2-Hüllkurve: gleiche Form, aber patch-spezifisch skaliert →
  // beide Schichten bewegen sich nicht im Gleichschritt (lebendiger Layer).
  // Decay-Zeit: eigene Tastaturverfolgung (DS_DTKbdTrack). Der Keyboard-Faktor
  // wird hier rekonstruiert und mit dem Exponenten abgeflacht, damit der
  // Layer-Anschlag über die ganze Tastatur gleich lang bleibt (track<1).
  const double kbdMul[4] = {
      map((voiceState.KNOTE + 1), 1, 88, 0.5, 3),
      map((voiceState.KNOTE + 1), 1, 88, 0.5, patch.DTE1Scaling),
      map((voiceState.KNOTE + 1), 1, 88, 0.5, 3),
      map((voiceState.KNOTE + 1), 1, 88, 0.5, 3)};
  for (int i = 0; i < 4; i++) {
    voiceState.AT2[i] = voiceState.AT[i] * patch.DS_ATScale[i];
    // track==1 → pow(kbdMul,1)=kbdMul → DT2 == DT*scale (identisch zu vorher).
    float trackedMul = powf((float)kbdMul[i], patch.DS_DTKbdTrack);
    voiceState.DT2[i] = voiceState.DTE[i] * trackedMul * patch.DS_DTScale[i];
    voiceState.RT2[i] = voiceState.RT[i];
  }

  // calc operator volume scaler depending on note index in scaler array.
  // Keyboard-Index in die 46-Punkt-EC-Tabellen. KNOTE kann durch MIDI-Noten
  // außerhalb der 88-Tasten-Range (oder krumme Eingaben) aus dem gültigen
  // Bereich [0,45] laufen → hart klemmen, sonst Out-of-Bounds-Lesezugriff.
  int ecIdx = int(floor(voiceState.KNOTE / 2) + 2);
  if (ecIdx < 0) ecIdx = 0;
  else if (ecIdx > 45) ecIdx = 45;
  voiceState.EG2 =
      floor(map(pow(map(1, 0, 1, patch.M1EC[0],
                        map(patch.M1EC[ecIdx], 0,
                            1, patch.M1EC[0], patch.M1EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG1 =
      floor(map(pow(map(1, 0, 1, patch.C2EC[0],
                        map(patch.C2EC[ecIdx], 0,
                            1, patch.C2EC[0], patch.C2EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG3 =
      floor(map(pow(map(1, 0, 1, patch.M2EC[0],
                        map(patch.M2EC[ecIdx], 0,
                            1, patch.M2EC[0], patch.M2EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG0 =
      floor(map(pow(map(1, 0, 1, patch.C1EC[0],
                        map(patch.C1EC[ecIdx], 0,
                            1, patch.C1EC[0], patch.C1EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);

  voiceState.CH1 = 0;
  voiceState.CH2 = 0;
  voiceState.CH1b = 0;
  voiceState.CH2b = 0;
  voiceState.M1 = 0;
  voiceState.M2 = 0;
  voiceState.M1b = 0;
  voiceState.M2b = 0;
  voiceState.M1old1 = 0;
  voiceState.M1old2 = 0;
  voiceState.M2old1 = 0;
  voiceState.M2old2 = 0;
  voiceState.M1bold1 = 0;
  voiceState.M1bold2 = 0;
  voiceState.M2bold1 = 0;
  voiceState.M2bold2 = 0;

  voiceState.GATENEW = 1;
}

int CGS1Emu::fmGenSample(VoiceState &voiceState) {
  const int gate = voiceState.GATE;
  const int gateOld = voiceState.GATEOLD;
  for (int e = 0; e < 4; e++) {
    stepEnvelope(e, gate, gateOld, voiceState.Mode,
                 voiceState.EA, voiceState.EAx, voiceState.EAo,
                 voiceState.RS, voiceState.RSx, voiceState.STATE,
                 voiceState.AT, voiceState.DT, voiceState.RT,
                 voiceState.IL, voiceState.SL);
    if (doubleStacksOn) {
      // Stack 2 nutzt eigene Envelope-States, aber dasselbe Gate und
      // dieselben Keyboard-/Velocity-Skalierungen (EG*, Velocity).
      stepEnvelope(e, gate, gateOld, voiceState.Mode,
                   voiceState.EA2, voiceState.EAx2, voiceState.EAo2,
                   voiceState.RS2, voiceState.RSx2, voiceState.STATE2,
                   voiceState.AT2, voiceState.DT2, voiceState.RT2,
                   voiceState.IL, voiceState.SL);
    }
  }
  voiceState.GATEOLD = voiceState.GATE;
  voiceState.GATE = voiceState.GATENEW;

  // Operator volume is calculated from scaled volume and velocity.
  const int vel = int(voiceState.Velocity);
  voiceState.AMP[2] = calcAmp(voiceState.EAx[2], int(voiceState.EG2), vel);
  voiceState.AMP[1] = calcAmp(voiceState.EAx[1], int(voiceState.EG1), vel);
  voiceState.AMP[3] = calcAmp(voiceState.EAx[3], int(voiceState.EG3), vel);
  voiceState.AMP[0] = calcAmp(voiceState.EAx[0], int(voiceState.EG0), vel);

  // Basis-Stack-Pegel-Offset (BaseLevelDb) anwenden, in [0,4095] clampen.
  // Wirkt auf Modulatoren als Helligkeit/FM-Index, auf Carrier als Lautstärke.
  for (int i = 0; i < 4; i++) {
    int a = voiceState.AMP[i] + voiceState.baseLevelOff[i];
    voiceState.AMP[i] = a < 0 ? 0 : (a > 4095 ? 4095 : a);
  }

  if (doubleStacksOn) {
    // Stack 2 teilt sich Keyboard-Scaling (EG*) und Velocity, hat aber durch
    // EAx2 eine eigene Hüllkurve.
    voiceState.AMP2[2] = calcAmp(voiceState.EAx2[2], int(voiceState.EG2), vel);
    voiceState.AMP2[1] = calcAmp(voiceState.EAx2[1], int(voiceState.EG1), vel);
    voiceState.AMP2[3] = calcAmp(voiceState.EAx2[3], int(voiceState.EG3), vel);
    voiceState.AMP2[0] = calcAmp(voiceState.EAx2[0], int(voiceState.EG0), vel);
    // Pegel-Offset pro Operator (DS_LevelDb) anwenden, in [0,4095] clampen.
    for (int i = 0; i < 4; i++) {
      int a = voiceState.AMP2[i] + voiceState.dsLevelOff[i];
      voiceState.AMP2[i] = a < 0 ? 0 : (a > 4095 ? 4095 : a);
    }
  }

  // Tremolo: nur auf Carrier (AMP[0], AMP[1]) anwenden.
  // Modulatoren bleiben unverändert → FM-Index (Klangfarbe) bleibt stabil.
  // tremoloAtten wird einmal pro Sample in processBlock berechnet.
  if (tremoloAtten > 0) {
      voiceState.AMP[0] = std::min(voiceState.AMP[0] + tremoloAtten, 4094);
      voiceState.AMP[1] = std::min(voiceState.AMP[1] + tremoloAtten, 4094);
      if (doubleStacksOn) {
        voiceState.AMP2[0] = std::min(voiceState.AMP2[0] + tremoloAtten, 4094);
        voiceState.AMP2[1] = std::min(voiceState.AMP2[1] + tremoloAtten, 4094);
      }
  }

  // Update all Phase accumulators..(28bit)
  // Vibrato: vibratoFraction wird einmal pro Sample in processBlock berechnet.
  // Alle 4 Operatoren gleichmäßig modulieren → Intervallverhältnisse bleiben stabil.
  for (int n = 0; n < 4; n++) {
    voiceState.PAI[n] = voiceState.PAI[n] & 0xFFFFFFF;
    voiceState.PAE[n] = voiceState.PAI[n] >> 18;
    voiceState.PAI[n] += (int)(voiceState.CW[n] * (1.0f + vibratoFraction));
    if (doubleStacksOn) {
      voiceState.PAI2[n] = voiceState.PAI2[n] & 0xFFFFFFF;
      voiceState.PAE2[n] = voiceState.PAI2[n] >> 18;
      voiceState.PAI2[n] += (int)(voiceState.CW2[n] * (1.0f + vibratoFraction));
    }
  }

  int baseMix = renderStackPair(voiceState.PAE, voiceState.AMP, voiceState.FMmode,
                                 voiceState.M1, voiceState.M2,
                                 voiceState.M1old1, voiceState.M1old2,
                                 voiceState.M2old1, voiceState.M2old2,
                                 voiceState.CH1, voiceState.CH2);

  if (!doubleStacksOn) {
    return int(baseMix * voiceState.outGain);
  }

  int layerMix = renderStackPair(voiceState.PAE2, voiceState.AMP2, voiceState.FMmode2,
                                 voiceState.M1b, voiceState.M2b,
                                 voiceState.M1bold1, voiceState.M1bold2,
                                 voiceState.M2bold1, voiceState.M2bold2,
                                 voiceState.CH1b, voiceState.CH2b);

  // Mix-/Layer-Parameter aus dem Patch, mit dem DIESE Stimme angeschlagen wurde
  // (nicht currentPatch) — sonst würde ein Programmwechsel klingende Stimmen
  // rückwirkend umstimmen. Fallback auf currentPatch, falls (theoretisch) leer.
  const PatchConsts& patch =
      voiceState.srcPatch ? *voiceState.srcPatch : *patches[currentPatch];
  const float wB = patch.DS_MixBase;
  const float wL = patch.DS_MixLayer;
  if (patch.DS_MixMode == 1) {
    // ADD: Grundklang bleibt auf vollem Pegel, Layer wird oben drauf addiert.
    // (Globales Headroom in processBlock = ~6× Vollaussteuerung pro Stimme.)
    return int((baseMix * wB + layerMix * wL) * voiceState.outGain);
  }
  // BLEND (default): Mittelwert beider Stacks → Chorus ohne Übersteuerung.
  return int((baseMix * wB + layerMix * wL) / (wB + wL) * voiceState.outGain);
}

CGS1Emu::CGS1Emu()
    : currentPatch(0),
      sampleRate(SampleRate),
      delayA(),
      delayB(),
      delayC()
{
    for (int i = 0; i < 256; ++i) {
      logsinTable[i] =
          (round(-(log(sin(ceil(i + 0.5) * PI / 256 / 2)) / log(2)) * 256.0));
      expTable[i] = (round((pow(2, i / 256.0) - 1) * 32768));
    }
    // expTable2 bleibt Identity: EA ist bereits ein Log-Domain-Wert.
    // lookupExp macht die exp. Umwandlung am Ende des Signalpfads —
    // eine zusätzliche Log-Kurve hier würde die Decay-Phase künstlich verlängern.
    for (int i = 0; i < 4096; i++) {
      expTable2[i] = i;
    }

    _filter.setLowpass(8000.0f, 0.707f, SampleRate);

    // 3-Band EQ initialisieren (0 dB = flat)
    _eqBass.setLowShelf(100.0f,   0.0f, (float)SampleRate);
    _eqMid.setPeaking  (600.0f,  1.0f, 0.0f, (float)SampleRate);
    _eqTreble.setHighShelf(6000.0f, 0.0f, (float)SampleRate);

    tremoloInc  = tremoloSpeed  * TWO_PI / (float)SampleRate;
    vibratoInc  = vibratoSpeed  * TWO_PI / (float)SampleRate;

    // GS1 Factory Presets (1)–(16)
    for (int i = 0; i < 16; ++i)
        patches[i] = gs1FactoryPresets[i];
    // Extended Preset Pack (17)–(36) hinten anhängen → im Standalone durchscrollbar.
    for (int i = 0; i < GS1_EXTENDED_COUNT; ++i)
        patches[16 + i] = gs1ExtendedPresets[i];

    currentPatch = 0;
}

void CGS1Emu::Initialize()
{
    // Alle Stimmen in den Grundzustand (stumm, kein Gate, kein Sustain) —
    // sonst laufen bei einem Re-Initialize noch klingende Noten weiter.
    // Kein Audio-Callback-Kontext → Zuweisung hier unkritisch.
    for (int v = 0; v < MAXVOICES; ++v)
        voiceStates[v] = VoiceState();
    voiceCounter = 0;

    // Filter-/EQ-Zustände leeren (Denormal-/Klick-frei neu starten).
    _filter.reset();
    _eqBass.reset();
    _eqMid.reset();
    _eqTreble.reset();

    delayA.reset();
    delayB.reset();
    delayC.reset();
}

void CGS1Emu::processMidi(uint8_t* data, int size)
{
    int i = 0;
    while (i < size)
    {
        uint8_t status = data[i];

        if ((status & 0xF0) == 0x90 && i + 2 < size)
        {
            uint8_t note = data[i + 1];
            uint8_t velocity = data[i + 2];
            if (velocity > 0)
            {
                int v = findVoice();
                voiceStates[v].noteAge = ++voiceCounter;
                noteOn(voiceStates[v], v, note - 20, 127 - velocity);
            }
            else
            {
                for (int v = 0; v < MAXVOICES; ++v)
                {
                    if (voiceStates[v].midiNote == note - 20)
                    {
                        if (!voiceStates[v].sustaining)
                            voiceStates[v].GATENEW = 0;
                        voiceStates[v].noteOn = false;
                    }
                }
            }
            i += 3;
        }
        else if ((status & 0xF0) == 0x80 && i + 2 < size)
        {
            uint8_t note = data[i + 1];
            for (int v = 0; v < MAXVOICES; ++v)
            {
                if (voiceStates[v].midiNote == note - 20)
                {
                    if (!voiceStates[v].sustaining)
                        voiceStates[v].GATENEW = 0;
                    voiceStates[v].noteOn = false;
                }
            }
            i += 3;
        }
        else if ((status & 0xF0) == 0xB0 && i + 2 < size)
        {
            if (data[i + 1] == 64)
            {
                uint8_t value = data[i + 2];
                bool sustainOn = (value >= 64);
                for (int v = 0; v < MAXVOICES; ++v)
                {
                    if (sustainOn)
                    {
                        // Pedal gedrückt: aktuell gehaltene Tasten markieren.
                        if (voiceStates[v].noteOn)
                            voiceStates[v].sustaining = true;
                    }
                    else
                    {
                        // Pedal losgelassen: ALLE Stimmen freigeben.
                        //  - Taste nicht mehr gehalten + sustaining → jetzt auslösen.
                        //  - Taste noch gehalten → nur Flag löschen, Ton bleibt;
                        //    der spätere Key-Up löst dann normal aus.
                        // (Vorher blieb sustaining bei gehaltenen Tasten stehen →
                        //  Note hing, wenn das Pedal vor der Taste losgelassen wurde.)
                        if (voiceStates[v].sustaining && !voiceStates[v].noteOn)
                            voiceStates[v].GATENEW = 0;
                        voiceStates[v].sustaining = false;
                    }
                }
            }
            i += 3;
        }
        else
        {
            ++i;
        }
    }
}

void CGS1Emu::processBlock(float* outputL, float* outputR, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // Tremolo LFO — einmal pro Sample (nicht pro Stimme!)
        if (tremoloOn) {
            tremoloPhase += tremoloInc;
            if (tremoloPhase >= TWO_PI) tremoloPhase -= TWO_PI;
            // Log-Domain: 512 Einheiten ≈ 12 dB Dämpfung bei Depth=1.0
            // Sinuskurve: 0 bei Phase=0 (laut), Maximum bei Phase=π (leise)
            tremoloAtten = (int)(tremoloDepth * 512.0f *
                                 (0.5f - 0.5f * std::cos(tremoloPhase)));
        } else {
            tremoloAtten = 0;
        }

        // Vibrato LFO — einmal pro Sample
        if (vibratoOn) {
            vibratoPhase += vibratoInc;
            if (vibratoPhase >= TWO_PI) vibratoPhase -= TWO_PI;
            // Lineare Näherung: fraction ≈ cents * ln(2)/1200
            vibratoFraction = vibratoDepth * kVibratoMaxCents *
                              kVibratoCentsToFraction * std::sin(vibratoPhase);
        } else {
            vibratoFraction = 0.0f;
        }

        int sumSample = 0;
        // Silence detection: skip voices with all EA == 0 and not active
        for (int v = 0; v < MAXVOICES; ++v) {
            VoiceState& vs = voiceStates[v];
            if (vs.EA[0] == 0 && vs.EA[1] == 0 && vs.EA[2] == 0 && vs.EA[3] == 0 && !vs.noteOn)
                continue;
            sumSample += fmGenSample(vs);
        }

        // Headroom: Divisor 4.5 statt 6 → ca. -2.5 dB mehr Reserve, damit
        // dichte Akkorde (v.a. Brass) + EQ/Ensemble nicht übersteuern.
        float sample = map(sumSample, -262144.0f / 4.5f, 262112.0f / 4.5f, -1.0f, 1.0f);
        sample *= masterVolume;   // globaler Lautstärkeregler (Master-Volume)
        sample = _filter.process(sample);

        // 3-Band EQ (Bass → Mid → Treble, seriell)
        sample = _eqBass.process(sample);
        sample = _eqMid.process(sample);
        sample = _eqTreble.process(sample);

        if (ensembleOn == false) {
            outputL[i] = sample;
            outputR[i] = sample;
        } else {
            delayA.pushSample(sample);
            delayB.pushSample(sample);
            delayC.pushSample(sample);

            lfo1Phase += lfo1Inc;
            lfo2Phase += lfo2Inc;

            // Phasen-Offsets für die 3 Delay-Lines (120 Grad Versatz)
            float phaseOffset2 = 21845.0f; // 1/3 von 65536
            float phaseOffset3 = 43690.0f; // 2/3 von 65536

            // Berechnung der Modulation für Line A, B und C
            auto getMod = [&](float phaseOffset) {
                float p1 = fmod(lfo1Phase + phaseOffset, 65536.0f) / 65536.0f;
                float p2 = fmod(lfo2Phase + phaseOffset, 65536.0f) / 65536.0f;
                // GS1 Mix: Viel Slow, wenig Fast Modulation
                return (std::sin(p1 * TWO_PI) * 0.85f + std::sin(p2 * TWO_PI) * 0.15f);
            };

            // Delay-Zeiten in Samples (GS1 nutzt ca. 5ms bis 10ms Basis-Delay)
            float baseDelay = 180.0f; // ca. 5.2ms bei 34.6kHz
            float modDepth = 60.0f;   // Modulationstiefe

            delayA.setDelay(baseDelay + getMod(0.0f) * modDepth);
            delayB.setDelay(baseDelay + getMod(phaseOffset2) * modDepth);
            delayC.setDelay(baseDelay + getMod(phaseOffset3) * modDepth);
            
            float sampA = delayA.popSample();
            float sampB = delayB.popSample();
            float sampC = delayC.popSample();
            outputL[i] = (sample * 0.5f) + (sampA * 0.5f) - (sampB * 0.3f);
            outputR[i] = (sample * 0.5f) + (sampC * 0.5f) - (sampB * 0.3f);
        }
    }
}