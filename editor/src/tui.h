#pragma once

#include "audio_engine.h"
#include "curve_generator.h"
#include "sysex.h"
#include <ncurses.h>
#include <string>

// ============================================================
// GS1 Patch Editor — Text User Interface (ncurses)
// ============================================================
//
// Layout (80×24 Terminal):
//
//   Zeile 0:    Titelleiste + Patch-Name
//   Zeile 1:    Separator
//   Zeile 2-5:  Operator-Parameter (Ratio, Detune, ATE, DTE)
//   Zeile 6:    Separator
//   Zeile 7-8:  Globale Parameter (DTE1Scaling)
//   Zeile 9:    Separator
//   Zeile 10-19: EC-Kurve ASCII-Darstellung (10 Zeilen hoch)
//   Zeile 20:   Kurven-Parameter (Breakpoints)
//   Zeile 21:   Separator
//   Zeile 22:   Statuszeile (Hilfe/MIDI-Info)
//   Zeile 23:   Kommandozeile
//
// Navigation:
//   Tab/Shift-Tab: Zwischen Sektionen wechseln
//   Pfeiltasten:   Parameter auswählen
//   +/-:           Wert ändern (fein)
//   PgUp/PgDn:     Wert ändern (grob)
//   Space:         Test-Note an/aus
//   S:             Speichern (.syx)
//   L:             Laden (.syx)
//   P:             Kurven-Preset wählen
//   N:             Nächstes Factory Preset
//   1-4:           Operator C1/C2/M1/M2 für Kurvenanzeige
//   q:             Beenden
// ============================================================

enum class Section {
    Operators,     // Ratio, Detune, ATE, DTE pro Operator
    Globals,       // DTE1Scaling
    Curve,         // EC-Kurve Breakpoints
    COUNT
};

enum class OperatorParam {
    Ratio,
    Detune,
    ATE,
    DTE,
    COUNT
};

class CTui {
public:
    CTui(CAudioEngine& audio);
    ~CTui();

    bool init();
    void run();     // Hauptschleife
    void shutdown();

private:
    CAudioEngine& m_audio;

    // Editierbarer Patch
    PatchFile m_patch;
    CCurveGenerator m_curves[4]; // C1, C2, M1, M2

    // UI-State
    Section m_section = Section::Operators;
    int m_selectedOp = 0;        // 0=C1, 1=C2, 2=M1, 3=M2
    int m_selectedParam = 0;     // innerhalb der Sektion
    int m_curveViewOp = 0;       // Welcher Operator in der Kurve angezeigt wird
    int m_selectedBreakpoint = 0;
    bool m_testNoteOn = false;
    int  m_testNote = 60;
    bool m_running = true;
    std::string m_statusMsg;
    bool m_modified = false;

    // Zeichenfunktionen
    void draw();
    void drawTitle();
    void drawOperators();
    void drawGlobals();
    void drawCurve();
    void drawStatus();

    // Eingabeverarbeitung
    void handleInput(int ch);
    void handleOperatorInput(int ch);
    void handleGlobalInput(int ch);
    void handleCurveInput(int ch);

    // Patch ↔ Engine synchronisieren
    void pushPatchToEngine();
    void loadFactoryPatch(int index);
    void rebuildCurvesFromPatch();
    void rebuildPatchFromCurves();

    // Datei-Operationen
    void savePatch();
    void loadPatch();

    // Kurven-Preset Dialog
    void curvePresetDialog();

    // Hilfsfunktionen
    void setStatus(const std::string& msg);
    static const char* opName(int op);
};
