#pragma once

#include "delayline.h"
#include "gs1_presets.h"
#include "gs1_presets_extended.h"  // Extended Preset Pack (20 Presets, EP/GP/VB/MB)
#include "gs1_steinway_opt.h"      // sample2gs1: CMA-ES-optimiertes Steinway-Preset
#include "gs1_wurli_opt.h"         // sample2gs1: CMA-ES-optimiertes Wurlitzer-Preset
#include "gs1_rhodes_opt.h"        // sample2gs1: CMA-ES-optimiertes Rhodes-Preset
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cmath>

#ifndef MAXVOICES
#define MAXVOICES 16
#endif

// Gesamtzahl der Programme: 16 Factory-Presets + Extended Preset Pack.
static constexpr int GS1_NUM_PROGRAMS = 16 + GS1_EXTENDED_COUNT + 3; // +3 = Steinway/Wurli/Rhodes Opt (sample2gs1)

struct GS1BiquadFilter {
    float b0, b1, b2;
    float a1, a2;
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
    
    void setLowpass(float cutoff_hz, float resonance, float sample_rate) {
        float omega = 2.0f * M_PI * cutoff_hz / sample_rate;
        float sin_omega = std::sin(omega);
        float cos_omega = std::cos(omega);
        float Q = resonance;
        float alpha = sin_omega / (2.0f * Q);

        float a0 = 1.0f + alpha;
        b0 = ((1.0f - cos_omega) / 2.0f) / a0;
        b1 = (1.0f - cos_omega) / a0;
        b2 = ((1.0f - cos_omega) / 2.0f) / a0;
        a1 = (-2.0f * cos_omega) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    // Low shelf: ±dBgain at freq_hz  (RBJ Audio EQ Cookbook, S=1)
    void setLowShelf(float freq_hz, float dBgain, float sample_rate) {
        float A  = std::pow(10.0f, dBgain / 40.0f);
        float w0 = 2.0f * M_PI * freq_hz / sample_rate;
        float cosw = std::cos(w0);
        float sinw = std::sin(w0);
        float alpha = sinw / 1.41421356f; // sin(w0)/sqrt(2), shelf slope S=1
        float sqA  = std::sqrt(A);

        float a0 =  (A+1) + (A-1)*cosw + 2.0f*sqA*alpha;
        b0 =  A * ((A+1) - (A-1)*cosw + 2.0f*sqA*alpha) / a0;
        b1 =  2.0f*A * ((A-1) - (A+1)*cosw)             / a0;
        b2 =  A * ((A+1) - (A-1)*cosw - 2.0f*sqA*alpha) / a0;
        a1 = -2.0f * ((A-1) + (A+1)*cosw)               / a0;
        a2 =         ((A+1) + (A-1)*cosw - 2.0f*sqA*alpha) / a0;
    }

    // Peaking EQ bell: ±dBgain at freq_hz, Q controls bandwidth
    void setPeaking(float freq_hz, float Q, float dBgain, float sample_rate) {
        float A  = std::pow(10.0f, dBgain / 40.0f);
        float w0 = 2.0f * M_PI * freq_hz / sample_rate;
        float cosw = std::cos(w0);
        float alpha = std::sin(w0) / (2.0f * Q);

        float a0 = 1.0f + alpha / A;
        b0 =  (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cosw)      / a0;
        b2 =  (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cosw)      / a0;
        a2 =  (1.0f - alpha / A) / a0;
    }

    // High shelf: ±dBgain at freq_hz  (RBJ Audio EQ Cookbook, S=1)
    void setHighShelf(float freq_hz, float dBgain, float sample_rate) {
        float A  = std::pow(10.0f, dBgain / 40.0f);
        float w0 = 2.0f * M_PI * freq_hz / sample_rate;
        float cosw = std::cos(w0);
        float sinw = std::sin(w0);
        float alpha = sinw / 1.41421356f;
        float sqA  = std::sqrt(A);

        float a0 =  (A+1) - (A-1)*cosw + 2.0f*sqA*alpha;
        b0 =  A * ((A+1) + (A-1)*cosw + 2.0f*sqA*alpha) / a0;
        b1 = -2.0f*A * ((A-1) + (A+1)*cosw)             / a0;
        b2 =  A * ((A+1) + (A-1)*cosw - 2.0f*sqA*alpha) / a0;
        a1 =  2.0f * ((A-1) - (A+1)*cosw)               / a0;
        a2 =         ((A+1) - (A-1)*cosw - 2.0f*sqA*alpha) / a0;
    }
    
    float process(float input) {
        float output = b0 * input + b1 * x1 + b2 * x2
                     - a1 * y1 - a2 * y2;

        // Denormal-Schutz: klingt das Signal in die Stille aus, können die
        // Zustände (y1/y2) denormal werden → massive CPU-Spikes auf x86/ARM,
        // was die Echtzeit-Garantie im Audio-Callback verletzt. Sehr kleine
        // Beträge hart auf 0 ziehen (klangneutral, weit unter -200 dBFS).
        if (std::fabs(output) < 1.0e-18f) output = 0.0f;

        x2 = x1; x1 = input;
        y2 = y1; y1 = output;

        return output;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
    }
};

enum MidiType { NoteOn, NoteOff, SustainPedalOn, SustainPedalOff };

// Detune-Modi wie beim originalen GS1-Schalter:
//   RANDOM1/2  — zufälliger Offset pro Note-On (organisch, lebendig)
//   STATIC1/2  — fester Offset pro Stimme     (konsistent, breiter Chor)
//   OFF        — kein Detune
enum class DetuneMode { RANDOM2, RANDOM1, OFF, STATIC1, STATIC2 };

struct MidiMessage {
    MidiType type;
    int noteNumber;
    int velocity;
    int samplePosition;
};

struct VoiceState {
  float NOTE = 0;
  int GATE = 0, GATEOLD = 0, GATENEW = 0;
  int Mode = 0;                             // 0=Norm 1=Perc (ENVELOPE MODE)
  float AT[4] = {2000, 2000, 4400, 4400};   //(Original envelope setting)
  float DT[4] = {2, 2, 1, 1};
  float RT[4] = {100, 100, 100, 100};
  int STATE[4] = {0, 0, 0, 0};
  int32_t EA[4] = {0, 0, 0, 0};             // 16bit
  int32_t EAx[4] = {0, 0, 0, 0};            // 16bit
  int32_t EAo[4] = {0, 0, 0, 0};            // oldaccu
  int32_t RS[4] = {0, 0, 0, 0};
  int32_t RSx[4] = {0, 0, 0, 0};
  int PAI[4]{}, PAE[4]{};
  int PAI2[4]{}, PAE2[4]{};
  int CW[4] = {0, 0, 0, 0};                 // C1,C2,M1,M2
  int CW2[4] = {0, 0, 0, 0};
  int AMP[4] = {255, 0, 255, 255};          // C1,C2,M1,M2
  int CH1 = 0, CH2 = 0;
  int CH1b = 0, CH2b = 0;
  int M1 = 0, M2 = 0;
  int M1b = 0, M2b = 0;
  int M1old1 = 0, M1old2 = 0;
  int M2old1 = 0, M2old2 = 0;
  int M1bold1 = 0, M1bold2 = 0;
  int M2bold1 = 0, M2bold2 = 0;
  int EG0 = 0, EG1 = 0, EG2 = 0, EG3 = 0;
  float rnd = 0;
  float KNOTE = 0;
  float Velocity = 0;

  // Per-voice params (copied from globals at noteOn, will become patch-dependent)
  int DTE[4] = {2, 2, 1, 1};
  int RTE[4] = {100, 100, 100, 100};
  int IL[4] = {0, 0, 0, 0};
  int SL[4] = {0, 0, 0, 0};
  int FMmode[2] = {0, 0};

  // --- Double-Stack (Stack 2): eigener Envelope-/AMP-Pfad ---
  // Wird nur befüllt/verarbeitet wenn doubleStacksOn aktiv ist.
  float AT2[4] = {2000, 2000, 4400, 4400};
  float DT2[4] = {2, 2, 1, 1};
  float RT2[4] = {100, 100, 100, 100};
  int STATE2[4] = {0, 0, 0, 0};
  int32_t EA2[4]  = {0, 0, 0, 0};
  int32_t EAx2[4] = {0, 0, 0, 0};
  int32_t EAo2[4] = {0, 0, 0, 0};
  int32_t RS2[4]  = {0, 0, 0, 0};
  int32_t RSx2[4] = {0, 0, 0, 0};
  int AMP2[4] = {255, 0, 255, 255};
  int FMmode2[2] = {0, 0};        // eigene FM-Topologie für Stack 2 (geerbt o. überschrieben)
  int dsLevelOff[4] = {0, 0, 0, 0}; // Pegel-Offset (Attenuation-Domain) pro Operator für Stack 2
  int baseLevelOff[4] = {0, 0, 0, 0}; // Pegel-Offset (Attenuation-Domain) pro Operator für Stack 1 (BaseLevelDb)
  float outGain = 1.0f;               // linearer Ausgangs-Gain des Presets (OutLevelDb)

  int midiNote = 0;
  bool noteOn = false;
  bool sustaining = false;
  uint32_t noteAge = 0;

  // Patch, mit dem diese Stimme angeschlagen wurde. Bei noteOn gesetzt, damit
  // ein späterer Programmwechsel die klingende Stimme nicht rückwirkend ändert.
  const PatchConsts* srcPatch = nullptr;
};

class CGS1Emu {
public:
  CGS1Emu();

  void Initialize();
  void processBlock(float* outputL, float* outputR, int numSamples);
  void processMidi(uint8_t* data, int size);

  void initPatch(int index);

  int getNumPrograms();
  int getCurrentProgram();
  void setCurrentProgram(int index);
  const char* getProgramName(int index) const;

  void setEnsembleOn(bool ensonoff);
  bool getEnsembleOn();

  void setDetuneMode(DetuneMode mode);
  DetuneMode getDetuneMode() const;

  void setDoubleStacksOn(bool on);
  bool getDoubleStacksOn() const;

  // Tremolo (Speed 1–6 Hz, Depth 0.0–1.0, On/Off)
  void setTremoloOn(bool on);
  bool getTremoloOn() const;
  void setTremoloSpeed(float hz);    // 1.0 – 6.0 Hz
  float getTremoloSpeed() const;
  void setTremoloDepth(float depth); // 0.0 – 1.0
  float getTremoloDepth() const;

  // Vibrato (Speed 4–10 Hz, Depth 0.0–1.0, On/Off)
  void setVibratoOn(bool on);
  bool getVibratoOn() const;
  void setVibratoSpeed(float hz);    // 4.0 – 10.0 Hz
  float getVibratoSpeed() const;
  void setVibratoDepth(float depth); // 0.0 – 1.0
  float getVibratoDepth() const;

  // 3-Band EQ (±12 dB each band)
  void setEqBass(float dB);          // Low shelf  @ 100 Hz, ±12 dB
  float getEqBass() const;
  void setEqMid(float dB);           // Peaking    @ 600 Hz, ±12 dB
  float getEqMid() const;
  void setEqTreble(float dB);        // High shelf @ 6 kHz,  ±12 dB
  float getEqTreble() const;

  // Master-Volume (globaler Ausgangsregler, wie am Original-GS1).
  // 0.0 = stumm, 1.0 = Einheitsverstärkung (default), bis 2.0 = +6 dB.
  void setMasterVolume(float v);     // 0.0 – 2.0
  float getMasterVolume() const;

  VoiceState voiceStates[MAXVOICES];

  const PatchConsts* patches[GS1_NUM_PROGRAMS]; // Factory (1)–(16) + Extended (17)–(36) + Steinway Opt (37)

  int currentPatch = 0;

  void noteOn(VoiceState& voiceState, int voiceIndex, float KNOTE, float Velocity);
  int fmGenSample(VoiceState& voiceState);
  int findVoice();

private:

  // Anti-aliasing lowpass
  GS1BiquadFilter _filter;

  // 3-Band EQ
  GS1BiquadFilter _eqBass;
  GS1BiquadFilter _eqMid;
  GS1BiquadFilter _eqTreble;
  // Gain-Werte werden vom UI-Thread geschrieben, vom Audio-Thread gelesen
  // → atomic mit relaxed load (single-writer / single-reader, kurze Latenz).
  std::atomic<float> eqBassGain{0.0f};
  std::atomic<float> eqMidGain{0.0f};
  std::atomic<float> eqTrebleGain{0.0f};

  // Ensemble On/Off
  std::atomic<bool> ensembleOn{false};
  uint32_t voiceCounter = 0;

  // Tremolo LFO
  std::atomic<bool>  tremoloOn{false};
  float tremoloSpeed = 3.0f;   // Hz
  float tremoloDepth = 0.5f;   // 0.0 – 1.0
  // tremoloInc wird vom UI geändert (setTremoloSpeed) und im Audio-Thread
  // sample-genau gelesen → atomic.
  std::atomic<float> tremoloInc{0.0f};
  float tremoloPhase = 0.0f;   // aktuelle LFO-Phase (0 – 2π), nur Audio-Thread
  int   tremoloAtten = 0;      // aktueller Dämpfungswert (Log-Domain), nur Audio-Thread

  // Vibrato LFO
  std::atomic<bool>  vibratoOn{false};
  float vibratoSpeed = 6.0f;  // Hz
  float vibratoDepth = 0.3f;  // 0.0 – 1.0
  std::atomic<float> vibratoInc{0.0f};
  float vibratoPhase = 0.0f;  // aktuelle LFO-Phase, nur Audio-Thread
  float vibratoFraction = 0.0f;  // aktueller Frequenz-Faktor, nur Audio-Thread
  // Maximale Pitch-Deviation bei Depth=1.0: ±30 Cent
  // Berechnung: cents * ln(2)/1200 → lineare Frequenz-Näherung
  static constexpr float kVibratoMaxCents = 30.0f;
  static constexpr float kVibratoCentsToFraction = 0.000578f; // ln(2)/1200

  // Master-Volume (globaler Ausgangs-Gain). atomic: wird vom Main-/UI-Thread
  // geschrieben, im Audio-Thread (processBlock) gelesen.
  std::atomic<float> masterVolume{1.0f};

  // Detune
  std::atomic<DetuneMode> detuneMode{DetuneMode::OFF};
  std::atomic<bool> doubleStacksOn{false};
  // Feste Cent-Offsets für STATIC-Modi: 16 Stimmen symmetrisch um 0 verteilt.
  // Index = Voice-Nummer, Wert in Semitonen (0.01 = 1 Cent).
  // Werte in Semitönen: 0.01 = 1 Cent
  // STATIC1: ±5 Cent max  → leichte Schwebung (~1 Hz bei A4)
  // STATIC2: ±12 Cent max → deutlicher Chorus-Charakter (~3 Hz bei A4)
  static constexpr float staticOffsets1[MAXVOICES] = {
      0.000f, +0.030f, -0.030f, +0.050f, -0.050f, +0.015f, -0.015f, +0.040f,
     -0.040f, +0.022f, -0.022f, +0.048f, -0.048f, +0.010f, -0.010f, +0.035f
  };
  static constexpr float staticOffsets2[MAXVOICES] = {
      0.000f, +0.070f, -0.070f, +0.120f, -0.120f, +0.035f, -0.035f, +0.090f,
     -0.090f, +0.055f, -0.055f, +0.110f, -0.110f, +0.020f, -0.020f, +0.080f
  };
  float lfo1Phase = 0;
  float lfo2Phase = 0;

  DelayLine delayA;
  DelayLine delayB;
  DelayLine delayC;
};