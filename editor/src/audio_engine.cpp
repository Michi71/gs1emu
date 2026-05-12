#include "audio_engine.h"
#include <cmath>
#include <cstdio>

static inline int16_t floatToInt16(float val) {
    return static_cast<int16_t>(
        std::fmax(-32767.0f, std::fmin(32767.0f, val * 32767.0f)));
}

CAudioEngine::CAudioEngine() {
    std::memset(m_bufferL, 0, sizeof(m_bufferL));
    std::memset(m_bufferR, 0, sizeof(m_bufferR));
}

CAudioEngine::~CAudioEngine() {
    shutdown();
}

bool CAudioEngine::init() {
    // SDL Audio init
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::fprintf(stderr, "SDL Audio init failed: %s\n", SDL_GetError());
        return false;
    }

    // GS1 Engine initialisieren
    m_gs1.sampleRate = EDITOR_SAMPLE_RATE;
    m_gs1.Initialize();

    // Audio-Device öffnen
    SDL_AudioSpec want = {};
    SDL_AudioSpec have = {};
    want.format = AUDIO_S16SYS;
    want.freq = EDITOR_SAMPLE_RATE;
    want.channels = 2;
    want.callback = audioCallback;
    want.userdata = this;
    want.samples = EDITOR_BUFFER_SIZE / 4;

    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (m_audioDevice == 0) {
        std::fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return false;
    }

    // MIDI (optional — kein Fehler wenn nicht verfügbar)
    initMidi();

    // Audio starten
    SDL_PauseAudioDevice(m_audioDevice, 0);

    return true;
}

void CAudioEngine::shutdown() {
    if (m_audioDevice != 0) {
        SDL_PauseAudioDevice(m_audioDevice, 1);
        SDL_CloseAudioDevice(m_audioDevice);
        m_audioDevice = 0;
    }
    if (m_midiIn) {
        Pm_Close(m_midiIn);
        m_midiIn = nullptr;
    }
    Pm_Terminate();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool CAudioEngine::initMidi() {
    Pm_Initialize();

    int numDevices = Pm_CountDevices();
    for (int i = 0; i < numDevices; i++) {
        const PmDeviceInfo* info = Pm_GetDeviceInfo(i);
        if (info && info->input) {
            m_midiDeviceId = i;
            break;
        }
    }

    if (m_midiDeviceId < 0) {
        std::fprintf(stderr, "Kein MIDI-Input gefunden.\n");
        return false;
    }

    PmError err = Pm_OpenInput(&m_midiIn, m_midiDeviceId, nullptr, 256, nullptr, nullptr);
    if (err != pmNoError) {
        std::fprintf(stderr, "MIDI open failed: %s\n", Pm_GetErrorText(err));
        m_midiIn = nullptr;
        return false;
    }

    const PmDeviceInfo* info = Pm_GetDeviceInfo(m_midiDeviceId);
    std::fprintf(stderr, "MIDI Input: %s\n", info ? info->name : "Unknown");

    return true;
}

void CAudioEngine::updatePatch(const PatchConsts& patch) {
    m_stagingPatch = patch;
    m_patchDirty.store(true, std::memory_order_release);
}

const PatchConsts* CAudioEngine::getCurrentPatch() const {
    return m_gs1.patches[m_currentProgram];
}

void CAudioEngine::setProgram(int index) {
    if (index >= 0 && index < m_gs1.getNumPrograms()) {
        m_gs1.setCurrentProgram(index);
        m_currentProgram = index;
    }
}

void CAudioEngine::playTestNote(int note, int velocity) {
    uint8_t midi[3] = {0x90, static_cast<uint8_t>(note),
                       static_cast<uint8_t>(velocity)};
    m_gs1.processMidi(midi, 3);
}

void CAudioEngine::stopTestNote(int note) {
    uint8_t midi[3] = {0x80, static_cast<uint8_t>(note), 0};
    m_gs1.processMidi(midi, 3);
}

void CAudioEngine::stopAllNotes() {
    for (int n = 0; n < 128; n++) {
        stopTestNote(n);
    }
}

void CAudioEngine::processMidiInput() {
    if (!m_midiIn) return;

    PmEvent events[64];
    int count = Pm_Read(m_midiIn, events, 64);
    for (int i = 0; i < count; i++) {
        uint8_t midi[3];
        midi[0] = Pm_MessageStatus(events[i].message);
        midi[1] = Pm_MessageData1(events[i].message);
        midi[2] = Pm_MessageData2(events[i].message);
        m_gs1.processMidi(midi, 3);
    }
}

// ============================================================
// Audio Callback (Echtzeit-Thread!)
// ============================================================

void CAudioEngine::audioCallback(void* userdata, uint8_t* stream, int len) {
    auto* engine = static_cast<CAudioEngine*>(userdata);
    int numSamples = len / 4; // 2ch × 16bit

    engine->renderAudio(reinterpret_cast<int16_t*>(stream), numSamples);
}

void CAudioEngine::renderAudio(int16_t* out, int numSamples) {
    // Patch-Update prüfen (lockfree)
    if (m_patchDirty.load(std::memory_order_acquire)) {
        // Staging-Patch als User-Patch an Position 0 der Engine setzen
        // (wir nutzen patches[0] als editierbaren Slot)
        static PatchConsts editPatch;
        editPatch = m_stagingPatch;
        m_gs1.patches[m_gs1.getCurrentProgram()] = &editPatch;
        m_patchDirty.store(false, std::memory_order_release);
    }

    m_gs1.processBlock(m_bufferL, m_bufferR, numSamples);

    for (int i = 0; i < numSamples; i++) {
        out[i * 2]     = floatToInt16(m_bufferL[i]);
        out[i * 2 + 1] = floatToInt16(m_bufferR[i]);
    }
}
