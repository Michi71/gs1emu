#pragma once

#include "gs1_presets.h"
#include <cstdint>
#include <vector>
#include <string>

// ============================================================
// GS1-EMU SysEx Format
// ============================================================
//
// Struktur:
//   F0              — SysEx Start
//   7D              — Non-Commercial Manufacturer ID
//   01              — GS1-EMU Device ID
//   01              — Patch Dump Command
//   <name_len>      — Patch-Name Länge (max 16)
//   <name_bytes>    — Patch-Name (ASCII, 7-bit safe)
//   <patch_data>    — Serialisierte PatchConsts (7-bit encoded)
//   <checksum>      — XOR aller Datenbytes
//   F7              — SysEx End
//
// 7-bit Encoding: Jedes Float/Int wird als 4-Byte IEEE754/int32
// übertragen, aufgesplittet in 5x 7-bit Nibbles (MIDI-safe).
// ============================================================

static constexpr uint8_t SYSEX_START     = 0xF0;
static constexpr uint8_t SYSEX_END       = 0xF7;
static constexpr uint8_t SYSEX_MANUF_ID  = 0x7D;  // Non-Commercial
static constexpr uint8_t SYSEX_DEVICE_ID = 0x01;   // GS1-EMU
static constexpr uint8_t SYSEX_CMD_PATCH = 0x01;   // Patch Dump

class CSysEx {
public:
    // Patch → SysEx-Bytes
    static std::vector<uint8_t> encodePatch(const PatchConsts& patch);

    // SysEx-Bytes → Patch (gibt false zurück bei ungültigen Daten)
    static bool decodePatch(const std::vector<uint8_t>& sysex, PatchConsts& outPatch);

    // Datei-I/O
    static bool saveToFile(const std::string& path, const PatchConsts& patch);
    static bool loadFromFile(const std::string& path, PatchConsts& outPatch);

    // Float/Int ↔ 5 × 7-bit Nibbles (public für Hilfsfunktionen)
    static void encodeFloat(float val, uint8_t out[5]);
    static float decodeFloat(const uint8_t in[5]);
    static void encodeInt(int val, uint8_t out[5]);
    static int decodeInt(const uint8_t in[5]);

private:
    static uint8_t calcChecksum(const uint8_t* data, size_t len);
};
