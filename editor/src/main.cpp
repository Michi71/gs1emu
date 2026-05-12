#include "tui.h"
#include "audio_engine.h"
#include <cstdio>

// ============================================================
// GS1-EMU Patch Editor
// ============================================================
// Textbasierter Patch-Editor für den Yamaha GS1 Emulator.
// Nutzt ncurses für die TUI, SDL2 für Audio, PortMidi für MIDI.
// ============================================================

int main(int /*argc*/, char* /*argv*/[]) {
    std::fprintf(stderr, "GS1-EMU Patch Editor v0.1\n");

    // Audio-Engine starten
    CAudioEngine audio;
    if (!audio.init()) {
        std::fprintf(stderr, "Audio-Engine konnte nicht initialisiert werden.\n");
        return 1;
    }

    // TUI starten
    CTui tui(audio);
    if (!tui.init()) {
        std::fprintf(stderr, "TUI konnte nicht initialisiert werden.\n");
        audio.shutdown();
        return 1;
    }

    // Hauptschleife
    tui.run();

    // Aufräumen
    tui.shutdown();
    audio.shutdown();

    std::fprintf(stderr, "Editor beendet.\n");
    return 0;
}
