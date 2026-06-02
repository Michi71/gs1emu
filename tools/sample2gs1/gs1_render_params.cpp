// gs1_render_params — rendert ein .gs1-Textpreset über libgs1emu.
// Nutzung: gs1_render_params <preset.gs1> <dur_s> <note1> [note2 ...]
// Schreibt rohe float32-Samples (mono, ENGINE_SR) aller Noten nacheinander
// nach stdout (binär). Wird vom CMA-ES-Optimierer pro Iteration aufgerufen.
#include "gs1emu.h"
#include "gs1_presets.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>

static std::map<std::string, std::vector<float>> read_preset(const char* path) {
  std::map<std::string, std::vector<float>> m;
  std::ifstream f(path);
  std::string line;
  while (std::getline(f, line)) {
    std::istringstream ss(line);
    std::string key; ss >> key;
    if (key.empty()) continue;
    std::vector<float> vals; float v;
    while (ss >> v) vals.push_back(v);
    m[key] = vals;
  }
  return m;
}

template <class T>
static void fill(const std::map<std::string, std::vector<float>>& m,
                 const char* key, T* dst, int n) {
  auto it = m.find(key);
  if (it == m.end()) return;
  for (int i = 0; i < n && i < (int)it->second.size(); ++i)
    dst[i] = (T)it->second[i];
}
static float scalar(const std::map<std::string, std::vector<float>>& m,
                    const char* key, float def) {
  auto it = m.find(key);
  return (it != m.end() && !it->second.empty()) ? it->second[0] : def;
}

int main(int argc, char** argv) {
  if (argc < 4) { fprintf(stderr, "usage: %s preset.gs1 dur_s note...\n", argv[0]); return 1; }
  auto m = read_preset(argv[1]);
  double dur = atof(argv[2]);

  PatchConsts p{};
  fill(m, "Ratio", p.Ratio, 4);   fill(m, "Detune", p.Detune, 4);
  fill(m, "C1EC", p.C1EC, 46);     fill(m, "C2EC", p.C2EC, 46);
  fill(m, "M1EC", p.M1EC, 46);     fill(m, "M2EC", p.M2EC, 46);
  fill(m, "ATE", p.ATE, 4);        p.DTE1Scaling = scalar(m, "DTE1Scaling", 2.0f);
  fill(m, "DTE", p.DTE, 4);        fill(m, "RTE", p.RTE, 4);
  fill(m, "IL", p.IL, 4);          fill(m, "SL", p.SL, 4);
  fill(m, "FMmode", p.FMmode, 2);
  fill(m, "DS_Detune", p.DS_Detune, 4);
  p.DS_MixBase = scalar(m, "DS_MixBase", 1.0f);
  p.DS_MixLayer = scalar(m, "DS_MixLayer", 1.0f);
  fill(m, "DS_ATScale", p.DS_ATScale, 4); fill(m, "DS_DTScale", p.DS_DTScale, 4);
  fill(m, "DS_Ratio", p.DS_Ratio, 4);     fill(m, "DS_FMmode", p.DS_FMmode, 2);
  fill(m, "DS_LevelDb", p.DS_LevelDb, 4);
  p.DS_ModKbdDb = scalar(m, "DS_ModKbdDb", 0.0f);
  p.DS_DTKbdTrack = scalar(m, "DS_DTKbdTrack", 1.0f);
  p.DS_MixMode = (int)scalar(m, "DS_MixMode", 0.0f);
  fill(m, "BaseLevelDb", p.BaseLevelDb, 4);
  p.OutLevelDb = scalar(m, "OutLevelDb", 0.0f);
  p.DTEKbdScale = scalar(m, "DTEKbdScale", 3.0f);
  const char* nm = "ParamGen";
  for (int i = 0; i <= PATCH_NAME_MAX; ++i) p.Name[i] = (i < (int)strlen(nm)) ? nm[i] : '\0';

  bool ds = scalar(m, "doublestack", 1.0f) > 0.5f;

  const int SR = 34687, blk = 256;
  CGS1Emu emu; emu.Initialize();
  emu.patches[0] = &p;
  emu.setCurrentProgram(0);
  emu.setDoubleStacksOn(ds);

  for (int ai = 3; ai < argc; ai++) {
    int note = atoi(argv[ai]);
    std::vector<float> bl(blk), br(blk);
    uint8_t on[3] = {0x90, (uint8_t)note, 110}; emu.processMidi(on, 3);
    int total = int(SR * dur), pos = 0;
    while (pos < total) {
      int mm = std::min(blk, total - pos);
      emu.processBlock(bl.data(), br.data(), mm);
      fwrite(bl.data(), sizeof(float), mm, stdout);
      pos += mm;
    }
    uint8_t off[3] = {0x80, (uint8_t)note, 0}; emu.processMidi(off, 3);
    // Stimme ausklingen lassen / Reset zwischen Noten
    for (int k = 0; k < 200; k++) emu.processBlock(bl.data(), br.data(), blk);
  }
  return 0;
}
