#include "sysex.h"
#include <cstring>
#include <fstream>

// ============================================================
// 7-bit Encoding für MIDI-sichere Übertragung
// ============================================================

void CSysEx::encodeFloat(float val, uint8_t out[5]) {
    uint32_t bits;
    std::memcpy(&bits, &val, 4);
    out[0] = (bits >> 28) & 0x0F;
    out[1] = (bits >> 21) & 0x7F;
    out[2] = (bits >> 14) & 0x7F;
    out[3] = (bits >>  7) & 0x7F;
    out[4] = (bits      ) & 0x7F;
}

float CSysEx::decodeFloat(const uint8_t in[5]) {
    uint32_t bits = (static_cast<uint32_t>(in[0] & 0x0F) << 28)
                  | (static_cast<uint32_t>(in[1] & 0x7F) << 21)
                  | (static_cast<uint32_t>(in[2] & 0x7F) << 14)
                  | (static_cast<uint32_t>(in[3] & 0x7F) <<  7)
                  | (static_cast<uint32_t>(in[4] & 0x7F));
    float val;
    std::memcpy(&val, &bits, 4);
    return val;
}

void CSysEx::encodeInt(int val, uint8_t out[5]) {
    uint32_t bits;
    std::memcpy(&bits, &val, 4);
    out[0] = (bits >> 28) & 0x0F;
    out[1] = (bits >> 21) & 0x7F;
    out[2] = (bits >> 14) & 0x7F;
    out[3] = (bits >>  7) & 0x7F;
    out[4] = (bits      ) & 0x7F;
}

int CSysEx::decodeInt(const uint8_t in[5]) {
    uint32_t bits = (static_cast<uint32_t>(in[0] & 0x0F) << 28)
                  | (static_cast<uint32_t>(in[1] & 0x7F) << 21)
                  | (static_cast<uint32_t>(in[2] & 0x7F) << 14)
                  | (static_cast<uint32_t>(in[3] & 0x7F) <<  7)
                  | (static_cast<uint32_t>(in[4] & 0x7F));
    int val;
    std::memcpy(&val, &bits, 4);
    return val;
}

uint8_t CSysEx::calcChecksum(const uint8_t* data, size_t len) {
    uint8_t cs = 0;
    for (size_t i = 0; i < len; i++) {
        cs ^= data[i];
    }
    return cs & 0x7F;
}

// ============================================================
// Hilfsmakros für encode/decode Schleifen
// ============================================================

static void encodeFloatArray(std::vector<uint8_t>& sysex,
                             const float* arr, int count,
                             void (*enc)(float, uint8_t[5])) {
    uint8_t buf[5];
    for (int i = 0; i < count; i++) {
        enc(arr[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }
}

static void encodeIntArray(std::vector<uint8_t>& sysex,
                           const int* arr, int count,
                           void (*enc)(int, uint8_t[5])) {
    uint8_t buf[5];
    for (int i = 0; i < count; i++) {
        enc(arr[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }
}

// ============================================================
// Encode PatchConsts → SysEx
// ============================================================

std::vector<uint8_t> CSysEx::encodePatch(const PatchConsts& patch) {
    std::vector<uint8_t> sysex;
    sysex.push_back(SYSEX_START);
    sysex.push_back(SYSEX_MANUF_ID);
    sysex.push_back(SYSEX_DEVICE_ID);
    sysex.push_back(SYSEX_CMD_PATCH);

    // Patch-Name aus PatchConsts::Name
    int nameLen = static_cast<int>(std::strlen(patch.Name));
    if (nameLen > PATCH_NAME_MAX) nameLen = PATCH_NAME_MAX;
    sysex.push_back(static_cast<uint8_t>(nameLen));
    for (int i = 0; i < nameLen; i++) {
        sysex.push_back(static_cast<uint8_t>(patch.Name[i] & 0x7F));
    }

    // Beginn der Datenbytes (für Checksum)
    size_t dataStart = sysex.size();

    // Ratios (4 floats)
    encodeFloatArray(sysex, patch.Ratio, 4, encodeFloat);

    // Detune (4 ints)
    encodeIntArray(sysex, patch.Detune, 4, encodeInt);

    // EC Arrays (4 × 46 floats)
    encodeFloatArray(sysex, patch.C1EC, 46, encodeFloat);
    encodeFloatArray(sysex, patch.C2EC, 46, encodeFloat);
    encodeFloatArray(sysex, patch.M1EC, 46, encodeFloat);
    encodeFloatArray(sysex, patch.M2EC, 46, encodeFloat);

    // ATE (4 floats)
    encodeFloatArray(sysex, patch.ATE, 4, encodeFloat);

    // DTE1Scaling (1 float)
    uint8_t buf[5];
    encodeFloat(patch.DTE1Scaling, buf);
    sysex.insert(sysex.end(), buf, buf + 5);

    // DTE (4 ints)
    encodeIntArray(sysex, patch.DTE, 4, encodeInt);

    // RTE (4 ints)
    encodeIntArray(sysex, patch.RTE, 4, encodeInt);

    // IL (4 ints)
    encodeIntArray(sysex, patch.IL, 4, encodeInt);

    // SL (4 ints)
    encodeIntArray(sysex, patch.SL, 4, encodeInt);

    // FMmode (2 ints)
    encodeIntArray(sysex, patch.FMmode, 2, encodeInt);

    // Checksum über Datenbytes
    uint8_t cs = calcChecksum(sysex.data() + dataStart,
                              sysex.size() - dataStart);
    sysex.push_back(cs);
    sysex.push_back(SYSEX_END);

    return sysex;
}

// ============================================================
// Decode SysEx → PatchConsts
// ============================================================

// Hilfsfunktion: Liest count Floats ab Position pos
static bool decodeFloats(const std::vector<uint8_t>& sysex, size_t& pos,
                         float* arr, int count) {
    for (int i = 0; i < count; i++) {
        if (pos + 5 > sysex.size()) return false;
        arr[i] = CSysEx::decodeFloat(&sysex[pos]);
        pos += 5;
    }
    return true;
}

static bool decodeInts(const std::vector<uint8_t>& sysex, size_t& pos,
                       int* arr, int count) {
    for (int i = 0; i < count; i++) {
        if (pos + 5 > sysex.size()) return false;
        arr[i] = CSysEx::decodeInt(&sysex[pos]);
        pos += 5;
    }
    return true;
}

bool CSysEx::decodePatch(const std::vector<uint8_t>& sysex, PatchConsts& out) {
    if (sysex.size() < 8) return false;
    if (sysex[0] != SYSEX_START) return false;
    if (sysex[1] != SYSEX_MANUF_ID) return false;
    if (sysex[2] != SYSEX_DEVICE_ID) return false;
    if (sysex[3] != SYSEX_CMD_PATCH) return false;
    if (sysex.back() != SYSEX_END) return false;

    size_t pos = 4;

    // Name
    int nameLen = sysex[pos++];
    if (nameLen > PATCH_NAME_MAX) return false;
    std::memset(out.Name, 0, sizeof(out.Name));
    for (int i = 0; i < nameLen; i++) {
        if (pos >= sysex.size()) return false;
        out.Name[i] = static_cast<char>(sysex[pos++]);
    }

    size_t dataStart = pos;

    if (!decodeFloats(sysex, pos, out.Ratio, 4)) return false;
    if (!decodeInts(sysex, pos, out.Detune, 4)) return false;
    if (!decodeFloats(sysex, pos, out.C1EC, 46)) return false;
    if (!decodeFloats(sysex, pos, out.C2EC, 46)) return false;
    if (!decodeFloats(sysex, pos, out.M1EC, 46)) return false;
    if (!decodeFloats(sysex, pos, out.M2EC, 46)) return false;
    if (!decodeFloats(sysex, pos, out.ATE, 4)) return false;

    // DTE1Scaling
    if (pos + 5 > sysex.size()) return false;
    out.DTE1Scaling = decodeFloat(&sysex[pos]);
    pos += 5;

    if (!decodeInts(sysex, pos, out.DTE, 4)) return false;
    if (!decodeInts(sysex, pos, out.RTE, 4)) return false;
    if (!decodeInts(sysex, pos, out.IL, 4)) return false;
    if (!decodeInts(sysex, pos, out.SL, 4)) return false;
    if (!decodeInts(sysex, pos, out.FMmode, 2)) return false;

    // Checksum
    if (pos + 2 > sysex.size()) return false;
    uint8_t expectedCs = calcChecksum(sysex.data() + dataStart, pos - dataStart);
    if (expectedCs != sysex[pos]) return false;

    return true;
}

// ============================================================
// File I/O
// ============================================================

bool CSysEx::saveToFile(const std::string& path, const PatchConsts& patch) {
    auto sysex = encodePatch(patch);
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(sysex.data()),
               static_cast<std::streamsize>(sysex.size()));
    return file.good();
}

bool CSysEx::loadFromFile(const std::string& path, PatchConsts& outPatch) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> sysex(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(sysex.data()),
              static_cast<std::streamsize>(size));
    return decodePatch(sysex, outPatch);
}
