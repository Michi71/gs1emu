#include "sysex.h"
#include <cstring>
#include <fstream>

// ============================================================
// 7-bit Encoding für MIDI-sichere Übertragung
// ============================================================

void CSysEx::encodeFloat(float val, uint8_t out[5]) {
    uint32_t bits;
    std::memcpy(&bits, &val, 4);
    out[0] = (bits >> 28) & 0x0F;         // 4 bit
    out[1] = (bits >> 21) & 0x7F;         // 7 bit
    out[2] = (bits >> 14) & 0x7F;         // 7 bit
    out[3] = (bits >>  7) & 0x7F;         // 7 bit
    out[4] = (bits      ) & 0x7F;         // 7 bit
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
// Encode PatchConsts → SysEx
// ============================================================

std::vector<uint8_t> CSysEx::encodePatch(const PatchFile& patch) {
    std::vector<uint8_t> sysex;
    sysex.push_back(SYSEX_START);
    sysex.push_back(SYSEX_MANUF_ID);
    sysex.push_back(SYSEX_DEVICE_ID);
    sysex.push_back(SYSEX_CMD_PATCH);

    // Patch-Name
    int nameLen = static_cast<int>(std::strlen(patch.name));
    if (nameLen > PATCH_NAME_MAX) nameLen = PATCH_NAME_MAX;
    sysex.push_back(static_cast<uint8_t>(nameLen));
    for (int i = 0; i < nameLen; i++) {
        sysex.push_back(static_cast<uint8_t>(patch.name[i] & 0x7F));
    }

    // Beginn der Datenbytes (für Checksum)
    size_t dataStart = sysex.size();

    uint8_t buf[5];

    // Ratios (4 floats)
    for (int i = 0; i < 4; i++) {
        encodeFloat(patch.data.Ratio[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }

    // Detune (4 ints)
    for (int i = 0; i < 4; i++) {
        encodeInt(patch.data.Detune[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }

    // EC Arrays (4 × 46 floats)
    for (int i = 0; i < 46; i++) {
        encodeFloat(patch.data.C1EC[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }
    for (int i = 0; i < 46; i++) {
        encodeFloat(patch.data.C2EC[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }
    for (int i = 0; i < 46; i++) {
        encodeFloat(patch.data.M1EC[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }
    for (int i = 0; i < 46; i++) {
        encodeFloat(patch.data.M2EC[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }

    // ATE (4 floats)
    for (int i = 0; i < 4; i++) {
        encodeFloat(patch.data.ATE[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }

    // DTE1Scaling (1 float)
    encodeFloat(patch.data.DTE1Scaling, buf);
    sysex.insert(sysex.end(), buf, buf + 5);

    // DTE (4 floats)
    for (int i = 0; i < 4; i++) {
        encodeFloat(patch.data.DTE[i], buf);
        sysex.insert(sysex.end(), buf, buf + 5);
    }

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

bool CSysEx::decodePatch(const std::vector<uint8_t>& sysex, PatchFile& outPatch) {
    // Minimale Validierung
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
    std::memset(outPatch.name, 0, sizeof(outPatch.name));
    for (int i = 0; i < nameLen; i++) {
        if (pos >= sysex.size()) return false;
        outPatch.name[i] = static_cast<char>(sysex[pos++]);
    }

    size_t dataStart = pos;

    // Ratios
    for (int i = 0; i < 4; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.Ratio[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }

    // Detune
    for (int i = 0; i < 4; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.Detune[i] = decodeInt(&sysex[pos]);
        pos += 5;
    }

    // EC Arrays
    for (int i = 0; i < 46; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.C1EC[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }
    for (int i = 0; i < 46; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.C2EC[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }
    for (int i = 0; i < 46; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.M1EC[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }
    for (int i = 0; i < 46; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.M2EC[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }

    // ATE
    for (int i = 0; i < 4; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.ATE[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }

    // DTE1Scaling
    if (pos + 5 > sysex.size()) return false;
    outPatch.data.DTE1Scaling = decodeFloat(&sysex[pos]);
    pos += 5;

    // DTE
    for (int i = 0; i < 4; i++) {
        if (pos + 5 > sysex.size()) return false;
        outPatch.data.DTE[i] = decodeFloat(&sysex[pos]);
        pos += 5;
    }

    // Checksum verifizieren
    if (pos + 2 > sysex.size()) return false;
    uint8_t expectedCs = calcChecksum(sysex.data() + dataStart, pos - dataStart);
    uint8_t actualCs = sysex[pos];
    if (expectedCs != actualCs) return false;

    return true;
}

// ============================================================
// File I/O
// ============================================================

bool CSysEx::saveToFile(const std::string& path, const PatchFile& patch) {
    auto sysex = encodePatch(patch);
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(sysex.data()),
               static_cast<std::streamsize>(sysex.size()));
    return file.good();
}

bool CSysEx::loadFromFile(const std::string& path, PatchFile& outPatch) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> sysex(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(sysex.data()),
              static_cast<std::streamsize>(size));
    return decodePatch(sysex, outPatch);
}
