#pragma once

#include "delayline.h"
#include "patchdata.h"
#include "gs1_presets.h"
#include <cstdint>
#include <cstring>
#include <cmath>

#ifndef MAXVOICES
#define MAXVOICES 32
#endif

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
    
    float process(float input) {
        float output = b0 * input + b1 * x1 + b2 * x2
                     - a1 * y1 - a2 * y2;
        
        x2 = x1; x1 = input;
        y2 = y1; y1 = output;
        
        return output;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
    }
};

enum MidiType { NoteOn, NoteOff, SustainPedalOn, SustainPedalOff };

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
  int CW[4] = {0, 0, 0, 0};                 // C1,C2,M1,M2
  int AMP[4] = {255, 0, 255, 255};          // C1,C2,M1,M2
  int CH1 = 0, CH2 = 0;
  int M1 = 0, M2 = 0;
  int M1old1 = 0, M1old2 = 0;
  int M2old1 = 0, M2old2 = 0;
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

  int midiNote = 0;
  bool noteOn = false;
  bool sustaining = false;
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

  VoiceState voiceStates[MAXVOICES];

  const PatchConsts* patches[16];       // GS1 Factory Presets (1)–(16)
  
  int lastVoice = 0;
  int chorusPos = 0;
  int currentPatch = 0;
  int sampleRate;

  void noteOn(VoiceState& voiceState, float KNOTE, float Velocity);
  int fmGenSample(VoiceState& voiceState);

private:

  // Filter 
  GS1BiquadFilter _filter;

  DelayLine delayA;
  DelayLine delayB;
  DelayLine delayC;
};