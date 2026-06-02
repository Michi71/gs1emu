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
//
// Threading-Modell:
//   - Haupt-Thread (TUI): schreibt userPatch (m_userPatch) und ruft setProgram,
//     playTestNote etc.
//   - SDL-Audio-Thread: ruft renderAudio() → CGS1Emu::processBlock().
//   - MIDI-Thread (PMScheduler): ruft processMidiInput() → engine.processMidi().
//
// Cross-Thread-Daten, die zwischen Haupt- und Audio-Thread laufen:
//   - userPatch + m_patchDirty: 800-Byte PatchStruct. Wir tauschen nur dann,
//     wenn Audio-Thread "idle" ist (im Audio-Callback selbst). Schreibvorgang
//     ist single-writer (Haupt-Thread) → single-reader (Audio-Thread).
//   - m_currentProgram: atomic<int>.
//
// Cross-Thread MIDI → Engine: processMidiInput läuft unter SDL_LockAudioDevice,
// damit processMidi() (das voiceStates[] schreibt) nicht mit dem Audio-Thread
// kollidiert, der dieselben Felder liest. Ohne Lock wären Init-Felder-Torn-
// writes und Bus-Errors möglich (vgl. test/standalone.cpp:179).
// ============================================================

#define EDITOR_SAMPLE_RATE 34687
#define EDITOR_BUFFER_SIZE 512

class CAudioEngine {
public:
    CAudioEngine();
    ~CAudioEngine();

    bool init();
    void shutdown();

    // Patch in die Engine laden. Überschreibt den User-Patch (Slot 'userProgram',
    // default = aktuelles Programm) und macht ihn im Audio-Callback sichtbar.
    void updatePatch(const PatchConsts& patch);

    // Aktuellen Patch lesen (für Initialisierung)
    const PatchConsts* getCurrentPatch() const;

    // MIDI: Test-Note abspielen. Ruft intern SDL_LockAudioDevice.
    void playTestNote(int note = 60, int velocity = 100);
    void stopTestNote(int note = 60);
    void stopAllNotes();

    // Programm-Slot wechseln (0..N-1). Schreibvorgang ist atomic; der
    // Audio-Thread sieht entweder den alten oder den neuen Slot, nie eine
    // halb-aktualisierte Mischung.
    void setProgram(int index);
    int  getProgram() const { return m_currentProgram.load(std::memory_order_acquire); }

    // Zugriff auf die Engine (für Preset-Liste etc.)
    CGS1Emu& getEngine() { return m_gs1; }

    // MIDI-Input verarbeiten (wird von Main-Loop aufgerufen)
    void processMidiInput();

    // Index des vom User editierbaren Patch-Slots. Aktuell fest auf
    // GS1_NUM_PROGRAMS-1 (nach den Factory- + Extended-Presets).
    int userProgram() const { return m_userProgram; }

private:
    CGS1Emu m_gs1;

    // SDL Audio
    SDL_AudioDeviceID m_audioDevice = 0;

    // PortMidi
    PmStream* m_midiIn = nullptr;
    int m_midiDeviceId = -1;

    // Aktives Programm und editierbarer User-Slot
    std::atomic<int> m_currentProgram{0};
    int m_userProgram = 0;          // wird im init() auf den letzten freien Slot gesetzt

    // Edit-Patch (im Audio-Thread via m_patchDirty sichtbar gemacht).
    // Wird per Copy-by-Value gehalten, damit keine Dangling-Pointer entstehen,
    // wenn die TUI ihren lokalen Patch zerstört.
    PatchConsts m_userPatch;
    // Audio-Thread-lokale Kopie, auf die m_gs1.patches[userProgram] zeigt.
    // Wird im Audio-Callback ausschließlich unter SDL_LockAudioDevice-Effekt
    // aktualisiert; dazwischen ist sie stabil. m_userPatch im Haupt-Thread
    // kann gefahrlos erneut überschrieben werden.
    PatchConsts m_audioUserPatch;
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
