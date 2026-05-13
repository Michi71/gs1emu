#include "gs1emu.h"
#include <cmath>

// Constants
static int SampleRate = 34687; // 34687Hz Samplerate (IMPORTANT!)
// Note! Sinewave phase resolution has to be 10bit for quantization error to
// give right sound!

static int maxDelaySamples = 512;

static double PI = 3.1415927;
static double HALF_PI = 1.5707964;
constexpr float TWO_PI = 6.2831853f;

// Tables
static int Sin[4096];
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

int CGS1Emu::getNumPrograms() { return 16; }

int CGS1Emu::getCurrentProgram() { return currentPatch; }

void CGS1Emu::setCurrentProgram(int index) {
  currentPatch = index;
}

void CGS1Emu::setEnsembleOn(bool ensonoff) { ensembleOn = ensonoff; }
bool CGS1Emu::getEnsembleOn() { return ensembleOn; }

void CGS1Emu::noteOn(VoiceState &voiceState, float KNOTE,
                                        float Velocity) {
  const PatchConsts& patch = *patches[currentPatch];

  voiceState.noteOn = true;
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
    voiceState.GATE = 0;
    voiceState.GATEOLD = 0;
    voiceState.Mode = 0;
  }
  for (int i = 0; i < 4; i++) {
    voiceState.PAI[i] = 0;
    voiceState.PAE[i] = 0;
  }
  voiceState.KNOTE = voiceState.KNOTE - 1;
  voiceState.NOTE = 27.50 * pow(2, (voiceState.KNOTE / (12))); // FROM A1
  voiceState.rnd = 0;

  for (int i = 0; i < 4; i++) {
    voiceState.DTE[i] = patch.DTE[i];
    voiceState.RTE[i] = patch.RTE[i];
    voiceState.IL[i]  = patch.IL[i];
    voiceState.SL[i]  = patch.SL[i];
  }
  
  voiceState.FMmode[0] = patch.FMmode[0];
  voiceState.FMmode[1] = patch.FMmode[1];

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

  // calc operator volume scaler depending on note index in scaler array.
  voiceState.EG2 =
      floor(map(pow(map(1, 0, 1, patch.M1EC[0],
                        map(patch.M1EC[int(floor(voiceState.KNOTE / 2) + 2)], 0,
                            1, patch.M1EC[0], patch.M1EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG1 =
      floor(map(pow(map(1, 0, 1, patch.C2EC[0],
                        map(patch.C2EC[int(floor(voiceState.KNOTE / 2) + 2)], 0,
                            1, patch.C2EC[0], patch.C2EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG3 =
      floor(map(pow(map(1, 0, 1, patch.M2EC[0],
                        map(patch.M2EC[int(floor(voiceState.KNOTE / 2) + 2)], 0,
                            1, patch.M2EC[0], patch.M2EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);
  voiceState.EG0 =
      floor(map(pow(map(1, 0, 1, patch.C1EC[0],
                        map(patch.C1EC[int(floor(voiceState.KNOTE / 2) + 2)], 0,
                            1, patch.C1EC[0], patch.C1EC[1])),
                    0.1),
                0, 1, 1, 0) *
            4095);

  voiceState.CH1 = 0;
  voiceState.CH2 = 0;
  voiceState.M1 = 0;
  voiceState.M2 = 0;
  voiceState.M1old1 = 0;
  voiceState.M1old2 = 0;
  voiceState.M2old1 = 0;
  voiceState.M2old2 = 0;

  voiceState.GATENEW = 1;
}

int CGS1Emu::fmGenSample(VoiceState &voiceState) {
  for (int e = 0; e < 4; e++) {
    if (voiceState.Mode == 0) {
      if (voiceState.GATEOLD == 0 && voiceState.GATE == 1) {
        voiceState.STATE[e] = 1;
        voiceState.EA[e] = voiceState.IL[e] << 12;
      }
      if (voiceState.GATE == 1 && voiceState.STATE[e] == 1 &&
          voiceState.EA[e] < 0xFFFFF) {
        voiceState.EA[e] = voiceState.EA[e] + voiceState.AT[e];
        voiceState.EAx[e] = voiceState.EA[e];
        if (voiceState.EA[e] > 0xFFFFF) {
          voiceState.EA[e] = 0xFFFFF;
        }
      }
      if (voiceState.GATE == 1 && voiceState.STATE[e] == 1 &&
          voiceState.EA[e] >= 0xFFFFF) {
        voiceState.STATE[e] = 2;
      }
      if (voiceState.GATE == 1 && voiceState.STATE[e] == 2 &&
          voiceState.EA[e] > voiceState.SL[e] << 12) {
        voiceState.EA[e] = voiceState.EA[e] - voiceState.DT[e];
        if (voiceState.EA[e] < voiceState.SL[e] << 12) {
          voiceState.EA[e] = voiceState.SL[e] << 12;
        }
        voiceState.EAx[e] =
            int(map(expTable2[int(
                        map(voiceState.EA[e], voiceState.SL[e] << 12, 0xFFFFF, 0, 4095))],
                    0, 4095, voiceState.SL[e] << 12, 0xFFFFF));
      }
      if (voiceState.GATE == 0 && voiceState.GATEOLD == 1) {
        voiceState.RS[e] = voiceState.EA[e];
        voiceState.RSx[e] = voiceState.EAx[e];
        voiceState.RT[e] = voiceState.RT[e];
      }
      if (voiceState.GATE == 0 && voiceState.EA[e] > 0) {
        voiceState.EA[e] = voiceState.EA[e] - voiceState.RT[e];
        voiceState.STATE[e] = 0;
        if (voiceState.EA[e] <= 0) {
          voiceState.EA[e] = 0;
        }
        // Guard: RS[e]==0 wenn Note losgelassen wird bevor Attack startet
        // → Division durch Null in map() vermeiden.
        if (voiceState.RS[e] > 0) {
          voiceState.EAx[e] =
              int(map(expTable2[int(map(voiceState.EA[e], 0,
                                        floor(voiceState.RS[e]), 0, 4095))],
                      0, 4095, 0, voiceState.RSx[e]));
        } else {
          voiceState.EAx[e] = 0;
        }
      }
      if (voiceState.GATE == 0 &&
          voiceState.EAo[e] < (int(voiceState.EA[e]) & 0xFFFFF)) {
        voiceState.EA[e] = 0;
      }
      voiceState.EAo[e] = voiceState.EA[e];
    }
  }
  voiceState.GATEOLD = voiceState.GATE;
  voiceState.GATE = voiceState.GATENEW;

  // Operator volume is calculated from scaled volume and velocity.
  voiceState.AMP[2] = ((((((int)floor(voiceState.EAx[2]) >> 8) ^ 4095) & 4095) +
                        int(voiceState.EG2))
                       << 0) +
                      (int(voiceState.Velocity) << 3);
  voiceState.AMP[1] = ((((((int)floor(voiceState.EAx[1]) >> 8) ^ 4095) & 4095) +
                        int(voiceState.EG1))
                       << 0) +
                      (int(voiceState.Velocity) << 3);
  voiceState.AMP[3] = ((((((int)floor(voiceState.EAx[3]) >> 8) ^ 4095) & 4095) +
                        int(voiceState.EG3))
                       << 0) +
                      (int(voiceState.Velocity) << 3);
  voiceState.AMP[0] = ((((((int)floor(voiceState.EAx[0]) >> 8) ^ 4095) & 4095) +
                        int(voiceState.EG0))
                       << 0) +
                      (int(voiceState.Velocity) << 3);
  for (int check = 0; check < 4;
       check++) { // Make sure amplitude does not go out of range.
    if (voiceState.AMP[check] >= 4095) {
      voiceState.AMP[check] = 4095;
    }
  }

  /*// 1. LFO für Tremolo (ca. 1.0Hz bis 7.0Hz)
  // In deiner Klasse als Member: float tremoloPhase;
  tremoloPhase += tremoloInc; // tremoloInc = (freq * 65536) / 34687
  if (tremoloPhase >= 65536.0f) tremoloPhase -= 65536.0f;

  // 2. Tremolo-Wert berechnen (0.0 bis 1.0)
  float tremoloMod = 1.0f - (tremoloDepth * (0.5f + 0.5f * sin(tremoloPhase * TWO_PI_OVER_65536)));

  // 3. Anwendung auf die Carrier-Amplituden
  // Wichtig: Nur auf die Carrier (AMP[0] und AMP[1]), 
  // damit die Klangfarbe (Modulationsindex) stabil bleibt!
  int finalAmp0 = (int)(voiceState.AMP[0] * tremoloMod);
  int finalAmp1 = (int)(voiceState.AMP[1] * tremoloMod);
  */

  // In deiner Routing-Logik nutzt du dann finalAmp statt voiceState.AMP

  // Update all Phase accumulators..(28bit)
  for (int n = 0; n < 4; n++) {
    voiceState.PAI[n] = voiceState.PAI[n] & 0xFFFFFFF;
    // Shift down externally used accu value.
    voiceState.PAE[n] = voiceState.PAI[n] >> 18;
    voiceState.PAI[n] = voiceState.PAI[n] + voiceState.CW[n];
  }

  // Following section is routing and operator stack config. 4 modes: norm,
  // pi/2: half intensity modulator self feedback, pi full mod self feedback and
  // cross from other stack. CHANNEL1
  if (voiceState.FMmode[0] == 0) { // NORM
    if (voiceState.AMP[2] <= 4094) {
      voiceState.M1 =
          ((lookupExp(lookupSin(voiceState.PAE[2]) + voiceState.AMP[2]) +
            8192) >>
           2) &
          1023;
    } else {
      voiceState.M1 = 4095;
    }
    if (voiceState.AMP[0] <= 4094) {
      voiceState.CH1 =
          ((lookupExp(lookupSin(voiceState.PAE[0] + voiceState.M1) +
                      voiceState.AMP[0]))); // 14bit output signed+-
    } else {
      voiceState.CH1 = 0;
    }
  }
  if (voiceState.FMmode[0] == 1) { // PI/2
    if (voiceState.AMP[2] <= 4094) {
      voiceState.M1 =
          (lookupExp(
               lookupSin(voiceState.PAE[2] +
                         (((voiceState.M1old1 + voiceState.M1old2) / 2) >> 3)) +
               voiceState.AMP[2]) +
           8192) >>
          4;
    } else {
      voiceState.M1 = 4095;
    }
    voiceState.M1old2 = voiceState.M1old1;
    voiceState.M1old1 = voiceState.M1;
    if (voiceState.AMP[0] <= 4094) {
      voiceState.CH1 =
          ((lookupExp(lookupSin(voiceState.PAE[0] + voiceState.M1) +
                      voiceState.AMP[0]))); // 14bit output signed+-
    } else {
      voiceState.CH1 = 0;
    }
  }
  if (voiceState.FMmode[0] == 2) { // PI
    if (voiceState.AMP[2] <= 4094) {
      voiceState.M1 =
          (lookupExp(
               lookupSin(voiceState.PAE[2] +
                         (((voiceState.M1old1 + voiceState.M1old2) / 2) >> 2)) +
               voiceState.AMP[2]) +
           8192) >>
          4;
    } else {
      voiceState.M1 = 4095;
    }
    voiceState.M1old2 = voiceState.M1old1;
    voiceState.M1old1 = voiceState.M1;
    if (voiceState.AMP[0] <= 4094) {
      voiceState.CH1 =
          ((lookupExp(lookupSin(voiceState.PAE[0] + voiceState.M1) +
                      voiceState.AMP[0]))); // 14bit output signed+-
    } else {
      voiceState.CH1 = 0;
    }
  }
  if (voiceState.FMmode[0] == 3) { // CROSS
    if (voiceState.AMP[2] <= 4094) {
      voiceState.M1 = ((lookupExp(lookupSin(voiceState.PAE[2] + voiceState.M2) +
                                  voiceState.AMP[2]) +
                        8192) >>
                       2) &
                      1023; // USE OPPOSITE MOD!
    } else {
      voiceState.M1 = 4095;
    }
    if (voiceState.AMP[0] <= 4094) {
      voiceState.CH1 = ((lookupExp(
          lookupSin(voiceState.PAE[0] + voiceState.M1) + voiceState.AMP[0])));
    } else {
      voiceState.CH1 = 0;
    }
  }
  // CHANNEL2
  if (voiceState.FMmode[1] == 0) { // NORM
    if (voiceState.AMP[3] <= 4094) {
      voiceState.M2 =
          ((lookupExp(lookupSin(voiceState.PAE[3]) + voiceState.AMP[3]) +
            8192) >>
           2) &
          1023;
    } else {
      voiceState.M2 = 4095;
    }
    if (voiceState.AMP[1] <= 4094) {
      voiceState.CH2 =
          ((lookupExp(lookupSin(voiceState.PAE[1] + voiceState.M2) +
                      voiceState.AMP[1]))); // 14bit output signed+-
    } else {
      voiceState.CH2 = 0;
    }
  }
  if (voiceState.FMmode[1] == 1) { // PI/2
    // Fix: Guard muss AMP[3] (M2) prüfen, nicht AMP[1] (C2)
    if (voiceState.AMP[3] <= 4094) {
      voiceState.M2 =
          (lookupExp(
               lookupSin(voiceState.PAE[3] +
                         (((voiceState.M2old1 + voiceState.M2old2) / 2) >> 3)) +
               voiceState.AMP[3]) +
           8192) >>
          4;
    } else {
      voiceState.M2 = 4095;
    }
    voiceState.M2old2 = voiceState.M2old1;
    voiceState.M2old1 = voiceState.M2;
    if (voiceState.AMP[1] <= 4094) {
      voiceState.CH2 =
          // Fix: C2 muss von M2 moduliert werden, nicht M1
          ((lookupExp(lookupSin(voiceState.PAE[1] + voiceState.M2) +
                      voiceState.AMP[1]))); // 14bit output signed+-
    } else {
      voiceState.CH2 = 0;
    }
  }
  if (voiceState.FMmode[1] == 2) { // PI
    if (voiceState.AMP[3] <= 4094) {
      voiceState.M2 =
          (lookupExp(
               lookupSin(voiceState.PAE[3] +
                         (((voiceState.M2old1 + voiceState.M2old2) / 2) >> 2)) +
               voiceState.AMP[3]) +
           8192) >>
          4;
    } else {
      voiceState.M2 = 4095;
    }
    voiceState.M2old2 = voiceState.M2old1;
    voiceState.M2old1 = voiceState.M2;
    if (voiceState.AMP[1] <= 4094) {
      voiceState.CH2 =
          // Fix: C2 muss von M2 moduliert werden, nicht M1
          ((lookupExp(lookupSin(voiceState.PAE[1] + voiceState.M2) +
                      voiceState.AMP[1]))); // 14bit output signed+-
    } else {
      voiceState.CH2 = 0;
    }
  }
  if (voiceState.FMmode[1] == 3) { // CROSSMOD
    if (voiceState.AMP[3] <= 4094) {
      voiceState.M2 = ((lookupExp(lookupSin(voiceState.PAE[3] + voiceState.M1) +
                                  voiceState.AMP[3]) +
                        8192) >>
                       2) &
                      1023; // USE OPPOSITE MOD!
    } else {
      voiceState.M2 = 4095;
    }
    if (voiceState.AMP[1] <= 4094) {
      voiceState.CH2 = ((lookupExp(
          lookupSin(voiceState.PAE[1] + voiceState.M2) + voiceState.AMP[1])));
    } else {
      voiceState.CH2 = 0;
    }
  }
  return voiceState.CH1 +
         voiceState.CH2; // Mix two stacks output and send to wave array
}

CGS1Emu::CGS1Emu()
    : lastVoice(0),
      currentPatch(0),
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
    for (int i = 0; i < 4096; i++) {
      Sin[i] = int(map(sin(map(i, 0, 4095, 0, HALF_PI)), 0, 1, 0, 5782));
    }
    // expTable2 bleibt Identity: EA ist bereits ein Log-Domain-Wert.
    // lookupExp macht die exp. Umwandlung am Ende des Signalpfads —
    // eine zusätzliche Log-Kurve hier würde die Decay-Phase künstlich verlängern.
    for (int i = 0; i < 4096; i++) {
      expTable2[i] = i;
    }

    _filter.setLowpass(8000.0f, 0.707f, SampleRate); 

    // GS1 Factory Presets (1)–(16)
    for (int i = 0; i < 16; ++i)
        patches[i] = gs1FactoryPresets[i];

    currentPatch = 0;
}

void CGS1Emu::Initialize()
{
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
                noteOn(voiceStates[lastVoice], note - 24, 127 - velocity);
                lastVoice = (lastVoice + 1) % MAXVOICES;
            }
            else
            {
                for (int v = 0; v < MAXVOICES; ++v)
                {
                    if (voiceStates[v].midiNote == note - 24)
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
                if (voiceStates[v].midiNote == note - 24)
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
                        voiceStates[v].sustaining = true;
                    else if (!voiceStates[v].noteOn)
                    {
                        voiceStates[v].GATENEW = 0;
                        voiceStates[v].sustaining = false;
                        voiceStates[v].noteOn = false;
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
        int sumSample = 0;
        // Silence detection: skip voices with all EA == 0 and not active
        for (int v = 0; v < MAXVOICES; ++v) {
            VoiceState& vs = voiceStates[v];
            if (vs.EA[0] == 0 && vs.EA[1] == 0 && vs.EA[2] == 0 && vs.EA[3] == 0 && !vs.noteOn)
                continue;
            sumSample += fmGenSample(vs);
        }

        float sample = map(sumSample, -262144.0f / 6, 262112.0f / 6, -1.0f, 1.0f);
        sample = _filter.process(sample);

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