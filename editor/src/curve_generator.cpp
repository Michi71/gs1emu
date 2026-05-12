#include "curve_generator.h"

CCurveGenerator::CCurveGenerator() {
    // Default: flache Kurve bei 1.0, Basis 0.0, Ref 1.0
    m_baseLevel = 0.0f;
    m_refLevel  = 1.0f;
    presetFlat(1.0f);
}

void CCurveGenerator::clearBreakpoints() {
    m_breakpoints.clear();
}

void CCurveGenerator::addBreakpoint(int zone, float level) {
    zone  = std::clamp(zone, 0, EC_ZONES - 1);
    level = std::clamp(level, 0.0f, 1.0f);
    m_breakpoints.push_back({zone, level});
    sortBreakpoints();
}

void CCurveGenerator::removeBreakpoint(int index) {
    if (index >= 0 && index < static_cast<int>(m_breakpoints.size())) {
        m_breakpoints.erase(m_breakpoints.begin() + index);
    }
}

void CCurveGenerator::setBreakpoint(int index, int zone, float level) {
    if (index >= 0 && index < static_cast<int>(m_breakpoints.size())) {
        m_breakpoints[index].zone  = std::clamp(zone, 0, EC_ZONES - 1);
        m_breakpoints[index].level = std::clamp(level, 0.0f, 1.0f);
        sortBreakpoints();
    }
}

void CCurveGenerator::sortBreakpoints() {
    std::sort(m_breakpoints.begin(), m_breakpoints.end(),
              [](const Breakpoint& a, const Breakpoint& b) {
                  return a.zone < b.zone;
              });
}

// ============================================================
// Generiert die 44 Zonen-Werte per Breakpoint-Interpolation
// ============================================================

void CCurveGenerator::generateZones(float out[EC_ZONES]) const {
    if (m_breakpoints.empty()) {
        for (int i = 0; i < EC_ZONES; i++) out[i] = 0.0f;
        return;
    }

    if (m_breakpoints.size() == 1) {
        for (int i = 0; i < EC_ZONES; i++) out[i] = m_breakpoints[0].level;
        return;
    }

    int bpCount = static_cast<int>(m_breakpoints.size());

    for (int z = 0; z < EC_ZONES; z++) {
        // Vor dem ersten Breakpoint: konstant
        if (z <= m_breakpoints[0].zone) {
            out[z] = m_breakpoints[0].level;
            continue;
        }
        // Nach dem letzten Breakpoint: konstant
        if (z >= m_breakpoints[bpCount - 1].zone) {
            out[z] = m_breakpoints[bpCount - 1].level;
            continue;
        }

        // Segment finden und interpolieren
        for (int i = 0; i < bpCount - 1; i++) {
            if (z >= m_breakpoints[i].zone && z <= m_breakpoints[i + 1].zone) {
                int span = m_breakpoints[i + 1].zone - m_breakpoints[i].zone;
                if (span == 0) {
                    out[z] = m_breakpoints[i + 1].level;
                } else {
                    float t = static_cast<float>(z - m_breakpoints[i].zone)
                            / static_cast<float>(span);
                    out[z] = cosineInterp(m_breakpoints[i].level,
                                          m_breakpoints[i + 1].level, t);
                }
                break;
            }
        }
    }
}

// ============================================================
// Generiert das vollständige 46-Werte EC-Array
// ============================================================

void CCurveGenerator::generate(float out[EC_SIZE]) const {
    out[0] = m_baseLevel;
    out[1] = m_refLevel;
    generateZones(out + EC_ZONE_OFS);  // Schreibt [2..45]
}

// ============================================================
// Fit: Bestehende EC-Kurve → Breakpoints + Levels
// ============================================================

void CCurveGenerator::fitFromCurve(const float curve[EC_SIZE], float tolerance) {
    // Basis- und Referenz-Level übernehmen
    m_baseLevel = curve[0];
    m_refLevel  = curve[1];

    // Nur die 44 Zonen fitten
    const float* zones = curve + EC_ZONE_OFS;

    m_breakpoints.clear();

    // Immer Start und Ende der Zonen
    m_breakpoints.push_back({0, zones[0]});
    m_breakpoints.push_back({EC_ZONES - 1, zones[EC_ZONES - 1]});

    // Iterativ Breakpoints einfügen (Douglas-Peucker-ähnlich)
    bool changed = true;
    while (changed) {
        changed = false;

        float current[EC_ZONES];
        generateZones(current);

        float maxErr = 0.0f;
        int   maxIdx = -1;

        for (int z = 0; z < EC_ZONES; z++) {
            float err = std::fabs(current[z] - zones[z]);
            if (err > maxErr) {
                maxErr = err;
                maxIdx = z;
            }
        }

        if (maxErr > tolerance && maxIdx >= 0) {
            bool exists = false;
            for (const auto& bp : m_breakpoints) {
                if (bp.zone == maxIdx) { exists = true; break; }
            }
            if (!exists) {
                addBreakpoint(maxIdx, zones[maxIdx]);
                changed = true;
            }
        }
    }
}

// ============================================================
// Preset-Kurvenformen (nur die 44 Zonen)
// ============================================================

void CCurveGenerator::presetFlat(float level) {
    m_breakpoints.clear();
    addBreakpoint(0, level);
    addBreakpoint(EC_ZONES - 1, level);
}

void CCurveGenerator::presetBell(float peakLevel, int peakZone, float endLevel) {
    m_breakpoints.clear();
    addBreakpoint(0, endLevel);            // Tiefe Lage
    addBreakpoint(peakZone, peakLevel);    // Peak
    addBreakpoint(EC_ZONES - 1, endLevel); // Hohe Lage
}

void CCurveGenerator::presetDecay(float startLevel, float endLevel) {
    m_breakpoints.clear();
    addBreakpoint(0, startLevel);
    addBreakpoint(EC_ZONES - 1, endLevel);
}

void CCurveGenerator::presetRise(float startLevel, float peakLevel, int riseEnd) {
    m_breakpoints.clear();
    addBreakpoint(0, startLevel);
    addBreakpoint(riseEnd, peakLevel);
    addBreakpoint(EC_ZONES - 1, peakLevel);
}
