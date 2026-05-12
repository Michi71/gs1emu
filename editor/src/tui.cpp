#include "tui.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>

static const char* OP_NAMES[] = {"C1", "C2", "M1", "M2"};
static const char* PARAM_NAMES[] = {"Ratio", "Detune", "ATE", "DTE"};

// Farb-Paare
enum Colors {
    COL_NORMAL = 1,
    COL_HIGHLIGHT,
    COL_HEADER,
    COL_CURVE,
    COL_BREAKPOINT,
    COL_STATUS,
    COL_MODIFIED,
    COL_OP_C1,
    COL_OP_C2,
    COL_OP_M1,
    COL_OP_M2,
};

CTui::CTui(CAudioEngine& audio) : m_audio(audio) {
    std::memset(&m_patch, 0, sizeof(m_patch));
    std::snprintf(m_patch.name, PATCH_NAME_MAX, "Init");
}

CTui::~CTui() {
    shutdown();
}

bool CTui::init() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  // Non-blocking für MIDI-Polling
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(COL_NORMAL,     -1, -1);
        init_pair(COL_HIGHLIGHT,  COLOR_BLACK, COLOR_CYAN);
        init_pair(COL_HEADER,     COLOR_CYAN, -1);
        init_pair(COL_CURVE,      COLOR_GREEN, -1);
        init_pair(COL_BREAKPOINT, COLOR_YELLOW, -1);
        init_pair(COL_STATUS,     COLOR_WHITE, -1);
        init_pair(COL_MODIFIED,   COLOR_RED, -1);
        init_pair(COL_OP_C1,      COLOR_CYAN, -1);
        init_pair(COL_OP_C2,      COLOR_BLUE, -1);
        init_pair(COL_OP_M1,      COLOR_GREEN, -1);
        init_pair(COL_OP_M2,      COLOR_YELLOW, -1);
    }

    // Factory Preset 0 laden
    loadFactoryPatch(0);

    return true;
}

void CTui::shutdown() {
    endwin();
}

const char* CTui::opName(int op) {
    return (op >= 0 && op < 4) ? OP_NAMES[op] : "??";
}

// ============================================================
// Hauptschleife
// ============================================================

void CTui::run() {
    while (m_running) {
        // MIDI verarbeiten
        m_audio.processMidiInput();

        draw();

        int ch = getch();
        if (ch != ERR) {
            handleInput(ch);
        }

        // ~60fps refresh, MIDI-Polling
        napms(16);
    }
}

// ============================================================
// Zeichenfunktionen
// ============================================================

void CTui::draw() {
    erase();
    drawTitle();
    drawOperators();
    drawGlobals();
    drawCurve();
    drawStatus();
    refresh();
}

void CTui::drawTitle() {
    attron(COLOR_PAIR(COL_HEADER) | A_BOLD);
    mvprintw(0, 0, " GS1-EMU Patch Editor ");
    attroff(A_BOLD);

    if (m_modified) {
        attron(COLOR_PAIR(COL_MODIFIED));
        printw("[*] ");
        attroff(COLOR_PAIR(COL_MODIFIED));
    }

    attron(COLOR_PAIR(COL_NORMAL));
    printw("Patch: %-16s  Program: %d", m_patch.name, m_audio.getProgram());
    attroff(COLOR_PAIR(COL_HEADER));

    attron(COLOR_PAIR(COL_NORMAL));
    mvhline(1, 0, ACS_HLINE, 80);
}

void CTui::drawOperators() {
    int startRow = 2;

    // Header
    attron(COLOR_PAIR(COL_HEADER));
    mvprintw(startRow, 0, "         ");
    for (int op = 0; op < 4; op++) {
        int col = (op == 0) ? COL_OP_C1 : (op == 1) ? COL_OP_C2
                : (op == 2) ? COL_OP_M1 : COL_OP_M2;
        attron(COLOR_PAIR(col) | A_BOLD);
        printw("  %-8s", OP_NAMES[op]);
        attroff(A_BOLD);
    }
    attroff(COLOR_PAIR(COL_HEADER));

    // Parameter-Zeilen
    for (int p = 0; p < static_cast<int>(OperatorParam::COUNT); p++) {
        int row = startRow + 1 + p;

        bool isSelectedRow = (m_section == Section::Operators && m_selectedParam == p);

        attron(COLOR_PAIR(COL_NORMAL));
        mvprintw(row, 0, " %-7s ", PARAM_NAMES[p]);

        for (int op = 0; op < 4; op++) {
            bool isSelected = isSelectedRow && (m_selectedOp == op);

            if (isSelected) attron(COLOR_PAIR(COL_HIGHLIGHT));
            else attron(COLOR_PAIR(COL_NORMAL));

            float val = 0;
            switch (static_cast<OperatorParam>(p)) {
                case OperatorParam::Ratio:
                    val = m_patch.data.Ratio[op];
                    printw("  %6.2f  ", val);
                    break;
                case OperatorParam::Detune:
                    printw("  %+4d ct ", m_patch.data.Detune[op]);
                    break;
                case OperatorParam::ATE:
                    val = m_patch.data.ATE[op];
                    printw("  %6.0f  ", val);
                    break;
                case OperatorParam::DTE:
                    val = m_patch.data.DTE[op];
                    printw("  %6.2f  ", val);
                    break;
                default: break;
            }

            if (isSelected) attroff(COLOR_PAIR(COL_HIGHLIGHT));
        }
    }

    mvhline(startRow + 1 + static_cast<int>(OperatorParam::COUNT), 0, ACS_HLINE, 80);
}

void CTui::drawGlobals() {
    int row = 7;
    bool isSelected = (m_section == Section::Globals);

    attron(COLOR_PAIR(COL_HEADER));
    mvprintw(row, 0, " Globals");
    attroff(COLOR_PAIR(COL_HEADER));

    if (isSelected) attron(COLOR_PAIR(COL_HIGHLIGHT));
    else attron(COLOR_PAIR(COL_NORMAL));

    mvprintw(row + 1, 1, "DTE1Scaling: %6.2f", m_patch.data.DTE1Scaling);

    if (isSelected) attroff(COLOR_PAIR(COL_HIGHLIGHT));

    mvhline(row + 2, 0, ACS_HLINE, 80);
}

void CTui::drawCurve() {
    int startRow = 10;
    int height = 8;
    int width = EC_ZONES;  // 44 Spalten für die Keyboard-Zonen

    // Zonen-Kurve generieren (nur die 44 editierbaren Werte)
    float zones[EC_ZONES];
    m_curves[m_curveViewOp].generateZones(zones);

    // Header mit Operator-Auswahl + Base/Ref-Level
    int opCol = (m_curveViewOp == 0) ? COL_OP_C1 : (m_curveViewOp == 1) ? COL_OP_C2
              : (m_curveViewOp == 2) ? COL_OP_M1 : COL_OP_M2;
    attron(COLOR_PAIR(opCol) | A_BOLD);
    mvprintw(startRow - 1, 0, " EC-Kurve: %s ", OP_NAMES[m_curveViewOp]);
    attroff(A_BOLD);

    attron(COLOR_PAIR(COL_NORMAL));
    printw(" Base=%.2f Ref=%.2f  BP: %d  [1-4] Op [b/r] Base/Ref",
           m_curves[m_curveViewOp].getBaseLevel(),
           m_curves[m_curveViewOp].getRefLevel(),
           m_curves[m_curveViewOp].getBreakpointCount());

    // ASCII-Bargraph zeichnen (44 Zonen)
    for (int row = 0; row < height; row++) {
        float threshold = 1.0f - (static_cast<float>(row) / static_cast<float>(height - 1));

        // Y-Achse Label
        attron(COLOR_PAIR(COL_NORMAL));
        mvprintw(startRow + row, 0, "%3.0f%% ", threshold * 100.0f);

        for (int z = 0; z < width; z++) {
            // Ist hier ein Breakpoint?
            bool isBP = false;
            bool isBPSelected = false;
            if (m_section == Section::Curve) {
                for (int b = 0; b < m_curves[m_curveViewOp].getBreakpointCount(); b++) {
                    if (m_curves[m_curveViewOp].getBreakpoint(b).zone == z) {
                        isBP = true;
                        isBPSelected = (b == m_selectedBreakpoint);
                        break;
                    }
                }
            }

            char ch;
            if (zones[z] >= threshold) {
                ch = '|';
            } else if (zones[z] >= threshold - (1.0f / height)) {
                ch = '.';
            } else {
                ch = ' ';
            }

            if (isBPSelected) {
                attron(COLOR_PAIR(COL_HIGHLIGHT));
            } else if (isBP) {
                attron(COLOR_PAIR(COL_BREAKPOINT));
            } else {
                attron(COLOR_PAIR(COL_CURVE));
            }

            mvaddch(startRow + row, 6 + z, ch);

            if (isBPSelected) attroff(COLOR_PAIR(COL_HIGHLIGHT));
            else if (isBP) attroff(COLOR_PAIR(COL_BREAKPOINT));
        }
    }

    // X-Achse (44 Zonen, je 2 Halbtöne)
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(startRow + height, 6, "%-44s", "0   5   10   15   20   25   30   35   40 43");

    // Breakpoint-Details
    if (m_section == Section::Curve &&
        m_selectedBreakpoint < m_curves[m_curveViewOp].getBreakpointCount()) {
        const auto& bp = m_curves[m_curveViewOp].getBreakpoint(m_selectedBreakpoint);
        attron(COLOR_PAIR(COL_BREAKPOINT));
        mvprintw(startRow + height + 1, 0,
                 " BP %d/%d: Zone=%2d Level=%.2f  [+/-] Level  [</>] Zone  [a]dd [d]el",
                 m_selectedBreakpoint + 1,
                 m_curves[m_curveViewOp].getBreakpointCount(),
                 bp.zone, bp.level);
        attroff(COLOR_PAIR(COL_BREAKPOINT));
    }
}

void CTui::drawStatus() {
    int row = 22;
    mvhline(row - 1, 0, ACS_HLINE, 80);

    attron(COLOR_PAIR(COL_STATUS));
    mvprintw(row, 0, " %s", m_statusMsg.c_str());
    attroff(COLOR_PAIR(COL_STATUS));

    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(row + 1, 0,
             " [Tab] Sektion  [Space] Note  [S]ave  [L]oad  [P]reset  [N]ext  [q] Quit");
}

// ============================================================
// Eingabeverarbeitung
// ============================================================

void CTui::handleInput(int ch) {
    // Globale Tasten
    switch (ch) {
        case 'q':
        case 'Q':
            m_running = false;
            return;

        case '\t':  // Tab: nächste Sektion
            m_section = static_cast<Section>(
                (static_cast<int>(m_section) + 1) % static_cast<int>(Section::COUNT));
            return;

        case KEY_BTAB: // Shift-Tab: vorherige Sektion
            m_section = static_cast<Section>(
                (static_cast<int>(m_section) - 1 + static_cast<int>(Section::COUNT))
                % static_cast<int>(Section::COUNT));
            return;

        case ' ':  // Space: Test-Note
            if (m_testNoteOn) {
                m_audio.stopTestNote(m_testNote);
                m_testNoteOn = false;
                setStatus("Note Off");
            } else {
                m_audio.playTestNote(m_testNote, 100);
                m_testNoteOn = true;
                setStatus("Note On: " + std::to_string(m_testNote));
            }
            return;

        case '1': m_curveViewOp = 0; setStatus("Kurve: C1"); return;
        case '2': m_curveViewOp = 1; setStatus("Kurve: C2"); return;
        case '3': m_curveViewOp = 2; setStatus("Kurve: M1"); return;
        case '4': m_curveViewOp = 3; setStatus("Kurve: M2"); return;

        case 's': case 'S': savePatch(); return;
        case 'l': case 'L': loadPatch(); return;
        case 'p': case 'P': curvePresetDialog(); return;

        // Test-Note Tonhöhe ändern
        case '[': if (m_testNote > 21)  { m_testNote--; setStatus("Testnote: " + std::to_string(m_testNote)); } return;
        case ']': if (m_testNote < 108) { m_testNote++; setStatus("Testnote: " + std::to_string(m_testNote)); } return;

        // Factory Preset wechseln
        case 'n': case 'N':  // Next Factory Preset
        {
            int prog = (m_audio.getProgram() + 1) % m_audio.getEngine().getNumPrograms();
            loadFactoryPatch(prog);
            setStatus("Factory Preset " + std::to_string(prog));
            return;
        }
    }

    // Sektionsspezifisch
    switch (m_section) {
        case Section::Operators: handleOperatorInput(ch); break;
        case Section::Globals:   handleGlobalInput(ch); break;
        case Section::Curve:     handleCurveInput(ch); break;
        default: break;
    }
}

void CTui::handleOperatorInput(int ch) {
    float step = 0.01f;
    float bigStep = 0.1f;

    switch (ch) {
        case KEY_LEFT:
            m_selectedOp = std::max(0, m_selectedOp - 1);
            break;
        case KEY_RIGHT:
            m_selectedOp = std::min(3, m_selectedOp + 1);
            break;
        case KEY_UP:
            m_selectedParam = std::max(0, m_selectedParam - 1);
            break;
        case KEY_DOWN:
            m_selectedParam = std::min(static_cast<int>(OperatorParam::COUNT) - 1,
                                       m_selectedParam + 1);
            break;

        case '+':
        case '=':
        case KEY_PPAGE:
        {
            float delta = (ch == KEY_PPAGE) ? bigStep : step;
            switch (static_cast<OperatorParam>(m_selectedParam)) {
                case OperatorParam::Ratio:
                    m_patch.data.Ratio[m_selectedOp] = std::max(0.0f,
                        m_patch.data.Ratio[m_selectedOp] + delta);
                    break;
                case OperatorParam::Detune:
                    m_patch.data.Detune[m_selectedOp] += (ch == KEY_PPAGE) ? 5 : 1;
                    break;
                case OperatorParam::ATE:
                    m_patch.data.ATE[m_selectedOp] += (ch == KEY_PPAGE) ? 500.0f : 100.0f;
                    break;
                case OperatorParam::DTE:
                    m_patch.data.DTE[m_selectedOp] += delta;
                    break;
                default: break;
            }
            m_modified = true;
            pushPatchToEngine();
            break;
        }

        case '-':
        case '_':
        case KEY_NPAGE:
        {
            float delta = (ch == KEY_NPAGE) ? bigStep : step;
            switch (static_cast<OperatorParam>(m_selectedParam)) {
                case OperatorParam::Ratio:
                    m_patch.data.Ratio[m_selectedOp] = std::max(0.0f,
                        m_patch.data.Ratio[m_selectedOp] - delta);
                    break;
                case OperatorParam::Detune:
                    m_patch.data.Detune[m_selectedOp] -= (ch == KEY_NPAGE) ? 5 : 1;
                    break;
                case OperatorParam::ATE:
                    m_patch.data.ATE[m_selectedOp] = std::max(0.0f,
                        m_patch.data.ATE[m_selectedOp] - ((ch == KEY_NPAGE) ? 500.0f : 100.0f));
                    break;
                case OperatorParam::DTE:
                    m_patch.data.DTE[m_selectedOp] = std::max(0.0f,
                        m_patch.data.DTE[m_selectedOp] - delta);
                    break;
                default: break;
            }
            m_modified = true;
            pushPatchToEngine();
            break;
        }
    }
}

void CTui::handleGlobalInput(int ch) {
    switch (ch) {
        case '+': case '=':
            m_patch.data.DTE1Scaling += 0.1f;
            m_modified = true;
            pushPatchToEngine();
            break;
        case '-': case '_':
            m_patch.data.DTE1Scaling = std::max(0.0f, m_patch.data.DTE1Scaling - 0.1f);
            m_modified = true;
            pushPatchToEngine();
            break;
        case KEY_PPAGE:
            m_patch.data.DTE1Scaling += 1.0f;
            m_modified = true;
            pushPatchToEngine();
            break;
        case KEY_NPAGE:
            m_patch.data.DTE1Scaling = std::max(0.0f, m_patch.data.DTE1Scaling - 1.0f);
            m_modified = true;
            pushPatchToEngine();
            break;
    }
}

void CTui::handleCurveInput(int ch) {
    auto& gen = m_curves[m_curveViewOp];
    int bpCount = gen.getBreakpointCount();

    switch (ch) {
        case KEY_LEFT:
            m_selectedBreakpoint = std::max(0, m_selectedBreakpoint - 1);
            break;
        case KEY_RIGHT:
            m_selectedBreakpoint = std::min(bpCount - 1, m_selectedBreakpoint + 1);
            break;

        case '+': case '=':  // Level erhöhen
            if (m_selectedBreakpoint < bpCount) {
                auto bp = gen.getBreakpoint(m_selectedBreakpoint);
                gen.setBreakpoint(m_selectedBreakpoint, bp.zone,
                                  std::min(1.0f, bp.level + 0.02f));
                rebuildPatchFromCurves();
            }
            break;

        case '-': case '_':  // Level verringern
            if (m_selectedBreakpoint < bpCount) {
                auto bp = gen.getBreakpoint(m_selectedBreakpoint);
                gen.setBreakpoint(m_selectedBreakpoint, bp.zone,
                                  std::max(0.0f, bp.level - 0.02f));
                rebuildPatchFromCurves();
            }
            break;

        case ',': case '<':  // Zone nach links
            if (m_selectedBreakpoint < bpCount) {
                auto bp = gen.getBreakpoint(m_selectedBreakpoint);
                gen.setBreakpoint(m_selectedBreakpoint,
                                  std::max(0, bp.zone - 1), bp.level);
                rebuildPatchFromCurves();
            }
            break;

        case '.': case '>':  // Zone nach rechts
            if (m_selectedBreakpoint < bpCount) {
                auto bp = gen.getBreakpoint(m_selectedBreakpoint);
                gen.setBreakpoint(m_selectedBreakpoint,
                                  std::min(EC_ZONES - 1, bp.zone + 1), bp.level);
                rebuildPatchFromCurves();
            }
            break;

        case 'b':  // Base-Level erhöhen
            gen.setBaseLevel(gen.getBaseLevel() + 0.02f);
            rebuildPatchFromCurves();
            break;
        case 'B':  // Base-Level verringern
            gen.setBaseLevel(gen.getBaseLevel() - 0.02f);
            rebuildPatchFromCurves();
            break;

        case 'r':  // Ref-Level erhöhen
            gen.setRefLevel(gen.getRefLevel() + 0.02f);
            rebuildPatchFromCurves();
            break;
        case 'R':  // Ref-Level verringern
            gen.setRefLevel(gen.getRefLevel() - 0.02f);
            rebuildPatchFromCurves();
            break;

        case 'a': case 'A':  // Breakpoint hinzufügen
        {
            int newZone = EC_ZONES / 2;
            if (m_selectedBreakpoint < bpCount - 1) {
                auto bp1 = gen.getBreakpoint(m_selectedBreakpoint);
                auto bp2 = gen.getBreakpoint(m_selectedBreakpoint + 1);
                newZone = (bp1.zone + bp2.zone) / 2;
            }
            gen.addBreakpoint(newZone, 0.5f);
            m_selectedBreakpoint = std::min(m_selectedBreakpoint + 1, gen.getBreakpointCount() - 1);
            rebuildPatchFromCurves();
            break;
        }

        case 'd': case 'D':  // Breakpoint entfernen (min 2)
            if (bpCount > 2) {
                gen.removeBreakpoint(m_selectedBreakpoint);
                m_selectedBreakpoint = std::min(m_selectedBreakpoint,
                                                gen.getBreakpointCount() - 1);
                rebuildPatchFromCurves();
            } else {
                setStatus("Mindestens 2 Breakpoints erforderlich!");
            }
            break;
    }
}

// ============================================================
// Patch-Synchronisation
// ============================================================

void CTui::pushPatchToEngine() {
    // EC-Kurven aus den Generatoren in den Patch schreiben
    m_curves[0].generate(m_patch.data.C1EC);
    m_curves[1].generate(m_patch.data.C2EC);
    m_curves[2].generate(m_patch.data.M1EC);
    m_curves[3].generate(m_patch.data.M2EC);

    m_audio.updatePatch(m_patch.data);
}

void CTui::rebuildPatchFromCurves() {
    m_modified = true;
    pushPatchToEngine();
}

void CTui::loadFactoryPatch(int index) {
    m_audio.setProgram(index);
    const PatchConsts* p = m_audio.getCurrentPatch();
    if (p) {
        m_patch.data = *p;

        // Kurven aus den EC-Daten fitten
        m_curves[0].fitFromCurve(m_patch.data.C1EC);
        m_curves[1].fitFromCurve(m_patch.data.C2EC);
        m_curves[2].fitFromCurve(m_patch.data.M1EC);
        m_curves[3].fitFromCurve(m_patch.data.M2EC);

        m_selectedBreakpoint = 0;
        m_modified = false;

        setStatus("Factory Preset " + std::to_string(index) + " geladen");
    }
}

void CTui::rebuildCurvesFromPatch() {
    m_curves[0].fitFromCurve(m_patch.data.C1EC);
    m_curves[1].fitFromCurve(m_patch.data.C2EC);
    m_curves[2].fitFromCurve(m_patch.data.M1EC);
    m_curves[3].fitFromCurve(m_patch.data.M2EC);
    m_selectedBreakpoint = 0;
}

// ============================================================
// Datei-Operationen
// ============================================================

void CTui::savePatch() {
    // Kurven in Patch übernehmen
    pushPatchToEngine();

    // Dateiname zusammenbauen
    char filename[256];
    std::snprintf(filename, sizeof(filename), "%s.syx", m_patch.name);

    // Blocking-Modus für Texteingabe
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    mvprintw(23, 0, "Dateiname [%s]: ", filename);
    clrtoeol();
    refresh();

    char input[256] = {0};
    getnstr(input, sizeof(input) - 1);

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (std::strlen(input) > 0) {
        std::snprintf(filename, sizeof(filename), "%s", input);
    }

    if (CSysEx::saveToFile(filename, m_patch)) {
        setStatus(std::string("Gespeichert: ") + filename);
        m_modified = false;
    } else {
        setStatus("FEHLER beim Speichern!");
    }
}

void CTui::loadPatch() {
    // Blocking-Modus für Texteingabe
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    mvprintw(23, 0, "Dateiname laden: ");
    clrtoeol();
    refresh();

    char input[256] = {0};
    getnstr(input, sizeof(input) - 1);

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (std::strlen(input) == 0) {
        setStatus("Abgebrochen.");
        return;
    }

    PatchFile loaded;
    if (CSysEx::loadFromFile(input, loaded)) {
        m_patch = loaded;
        rebuildCurvesFromPatch();
        pushPatchToEngine();
        m_modified = false;
        setStatus(std::string("Geladen: ") + input);
    } else {
        setStatus("FEHLER beim Laden!");
    }
}

// ============================================================
// Kurven-Preset Dialog
// ============================================================

void CTui::curvePresetDialog() {
    mvprintw(23, 0, "Kurvenform [f]lat [b]ell [d]ecay [r]ise: ");
    clrtoeol();
    nodelay(stdscr, FALSE);

    int ch = getch();

    nodelay(stdscr, TRUE);

    auto& gen = m_curves[m_curveViewOp];

    switch (ch) {
        case 'f': case 'F':
            gen.presetFlat(1.0f);
            setStatus("Preset: Flat");
            break;
        case 'b': case 'B':
            gen.presetBell(1.0f, 15, 0.05f);
            setStatus("Preset: Bell");
            break;
        case 'd': case 'D':
            gen.presetDecay(1.0f, 0.01f);
            setStatus("Preset: Decay");
            break;
        case 'r': case 'R':
            gen.presetRise(0.2f, 1.0f, 5);
            setStatus("Preset: Rise");
            break;
        default:
            setStatus("Abgebrochen.");
            return;
    }

    m_selectedBreakpoint = 0;
    rebuildPatchFromCurves();
}

void CTui::setStatus(const std::string& msg) {
    m_statusMsg = msg;
}
