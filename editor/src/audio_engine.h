#pragma once

#include "gs1emu.h"
#include "sysex.h"
#include <SDL.h>
#include <portmidi.h>
#include <atomic>
#include <cstring>

// ============================================================
// Audio-Engine für den Patch Editor
// ============================================================
// Kapselt SDL2-Audio, PortMidi und die CGS1Emu-Instanz.
// Der Editor schreibt Patch-Daten lockfree in einen Staging-Buffer,
// der Audio-Callback übernimmt sie bei Bedarf.
// ============================================================

#define EDITOR_SAMPLE_RATE 34687
#define EDITOR_BUFFER_SIZE 512

class CAudioEngine {
public:
    CAudioEngine();
    ~CAudioEngine();

    bool init();
    void shutdown();

    // Patch in die Engine laden (thread-safe via atomic flag)
    void updatePatch(const PatchConsts& patch);

    // Aktuellen Patch lesen (für Initialisierung)
    const PatchConsts* getCurrentPatch() const;

    // MIDI: Test-Note abspielen
    void playTestNote(int note = 60, int velocity = 100);
    void stopTestNote(int note = 60);
    void stopAllNotes();

    // Patch-Programm wechseln (0..15 für Factory Presets)
    void setProgram(int index);
    int  getProgram() const { return m_currentProgram; }

    // Zugriff auf die Engine (für Preset-Liste etc.)
    CGS1Emu& getEngine() { return m_gs1; }

    // MIDI-Input verarbeiten (wird von Main-Loop aufgerufen)
    void processMidiInput();

private:
    CGS1Emu m_gs1;

    // SDL Audio
    SDL_AudioDeviceID m_audioDevice = 0;

    // PortMidi
    PmStream* m_midiIn = nullptr;
    int m_midiDeviceId = -1;

    int m_currentProgram = 0;

    // Lockfreier Patch-Update
    PatchConsts m_stagingPatch;
    std::atomic<bool> m_patchDirty{false};

    // Audio-Buffers
    float m_bufferL[EDITOR_BUFFER_SIZE];
    float m_bufferR[EDITOR_BUFFER_SIZE];

    // Audio-Callback
    static void audioCallback(void* userdata, uint8_t* stream, int len);
    void renderAudio(int16_t* out, int numSamples);

    // MIDI initialisieren
    bool initMidi();
};
