// render_note — rendert den generierten Preset über libgs1emu zu WAVs.
// Nutzung: render_note <midi1> [midi2 ...]
// Schreibt out/gs1_<midi>.wav (2 s, Note gehalten), wie die Quell-Samples.
#include "gs1emu.h"
#include "gs1_Steinway_gen.h"     // definiert gs1_SteinwayGen
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>

static void wav_mono(const std::string& p, std::vector<float>& L, int sr) {
  int n = (int)L.size();
  std::vector<int16_t> pc(n);
  for (int i = 0; i < n; i++) {
    float v = L[i]; v = v > 1 ? 1.f : (v < -1 ? -1.f : v);
    pc[i] = (int16_t)(v * 32767);
  }
  FILE* f = fopen(p.c_str(), "wb");
  int db = n*2, ck = 36+db, brate = sr*2, s1 = 16; short ba = 2, bps = 16, fm = 1, ch = 1;
  fwrite("RIFF",1,4,f); fwrite(&ck,4,1,f); fwrite("WAVE",1,4,f); fwrite("fmt ",1,4,f);
  fwrite(&s1,4,1,f); fwrite(&fm,2,1,f); fwrite(&ch,2,1,f); fwrite(&sr,4,1,f);
  fwrite(&brate,4,1,f); fwrite(&ba,2,1,f); fwrite(&bps,2,1,f); fwrite("data",1,4,f);
  fwrite(&db,4,1,f); fwrite(pc.data(),2,pc.size(),f); fclose(f);
}

int main(int argc, char** argv) {
  const int SR = 34687, blk = 256;
  CGS1Emu emu; emu.Initialize();
  emu.patches[0] = &gs1_SteinwayGen;     // generierten Preset einhängen
  emu.setCurrentProgram(0);
  emu.setDoubleStacksOn(true);           // mit Hammer-Layer (ADD)

  for (int ai = 1; ai < argc; ai++) {
    int note = atoi(argv[ai]);
    std::vector<float> L, bl(blk), br(blk);
    uint8_t on[3] = {0x90, (uint8_t)note, 110}; emu.processMidi(on, 3);
    int total = int(SR*2.0), pos = 0;     // 2 s gehalten (wie die Samples)
    while (pos < total) {
      int m = std::min(blk, total-pos); emu.processBlock(bl.data(), br.data(), m);
      for (int i = 0; i < m; i++) L.push_back(bl[i]); pos += m;
    }
    uint8_t off[3] = {0x80, (uint8_t)note, 0}; emu.processMidi(off, 3);
    char path[128]; snprintf(path, sizeof(path), "tools/sample2gs1/out/gs1_%03d.wav", note);
    wav_mono(path, L, SR);
    printf("gerendert: %s\n", path);
  }
  return 0;
}
