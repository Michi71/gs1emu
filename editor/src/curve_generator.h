#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

// ============================================================
// Parametrischer EC-Kurven-Generator für den GS1 Patch Editor
// ============================================================
// Generiert die 46 Keyboard-Scaling-Werte aus wenigen Breakpoints.
//
// EC-Array Layout (aus gs1_presets.h):
//   [0]     = Basis-Level (Untergrenze des Keyboard-Scalings)
//   [1]     = Referenz-Level (Obergrenze)
//   [2..45] = 44 Keyboard-Zonen (je 2 Halbtöne, tiefste bis höchste Lage)
//
// Die Engine mappt die Zonenwerte [2..45] (Bereich 0.0..1.0)
// zwischen [0] (Untergrenze) und [1] (Obergrenze).
//
// Breakpoint-Modell:
//   Breakpoints beschreiben nur die 44 Zonen-Kurve ([2..45]).
//   Jeder Breakpoint hat (zone, level) — Zone 0..43, Level 0.0..1.0.
//   Zwischen Breakpoints wird Cosine-interpoliert für glatte Kurven.
//   Mindestens 2 Breakpoints (Start + Ende).
//   baseLevel und refLevel werden separat gesetzt.
// ============================================================

static constexpr int EC_SIZE      = 46;
static constexpr int EC_ZONES     = 44;   // Anzahl Keyboard-Zonen [2..45]
static constexpr int EC_ZONE_OFS  = 2;    // Offset: Zonen beginnen bei Index 2

struct Breakpoint {
    int   zone;    // 0..43 (relativ zu den 44 Keyboard-Zonen)
    float level;   // 0.0..1.0
};

class CCurveGenerator {
public:
    CCurveGenerator();

    // Basis-/Referenz-Level (EC[0] und EC[1])
    void  setBaseLevel(float level) { m_baseLevel = std::clamp(level, 0.0f, 1.0f); }
    void  setRefLevel(float level)  { m_refLevel  = std::clamp(level, 0.0f, 1.0f); }
    float getBaseLevel() const { return m_baseLevel; }
    float getRefLevel()  const { return m_refLevel; }

    // Breakpoint-Management (für die 44 Zonen-Kurve)
    void clearBreakpoints();
    void addBreakpoint(int zone, float level);
    void removeBreakpoint(int index);
    void setBreakpoint(int index, int zone, float level);

    int getBreakpointCount() const { return static_cast<int>(m_breakpoints.size()); }
    const Breakpoint& getBreakpoint(int index) const { return m_breakpoints[index]; }

    // Kurve generieren — schreibt 46 Werte in out[]
    // out[0] = baseLevel, out[1] = refLevel, out[2..45] = interpolierte Zonen
    void generate(float out[EC_SIZE]) const;

    // Nur die 44 Zonen generieren (für Anzeige)
    void generateZones(float out[EC_ZONES]) const;

    // Aus bestehender EC-Kurve Breakpoints + Levels ableiten (Fit)
    void fitFromCurve(const float curve[EC_SIZE], float tolerance = 0.02f);

    // Preset-Kurvenformen (setzen nur die 44 Zonen-Breakpoints)
    void presetFlat(float level = 1.0f);
    void presetBell(float peakLevel = 1.0f, int peakZone = 15,
                    float endLevel = 0.05f);
    void presetDecay(float startLevel = 1.0f, float endLevel = 0.01f);
    void presetRise(float startLevel = 0.2f, float peakLevel = 1.0f,
                    int riseEnd = 5);

private:
    float m_baseLevel = 0.0f;   // EC[0]
    float m_refLevel  = 1.0f;   // EC[1]
    std::vector<Breakpoint> m_breakpoints;

    void sortBreakpoints();

    // Cosine-Interpolation zwischen zwei Werten
    static float cosineInterp(float a, float b, float t) {
        float mu = (1.0f - std::cos(t * 3.14159265f)) * 0.5f;
        return a * (1.0f - mu) + b * mu;
    }
};
