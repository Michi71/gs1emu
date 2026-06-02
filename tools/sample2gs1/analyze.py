#!/usr/bin/env python3
# ============================================================
# sample2gs1 — Stufe 1: Heuristischer Sample-zu-GS1-Preset-Mapper
# ------------------------------------------------------------
# Liest eine Sample-Bibliothek (mono WAV, Dateiname NNN-VVV.wav mit
# NNN = MIDI-Note, VVV = Velocity), extrahiert pro Note Features
# (Grundton, Amplituden-Hüllkurve, Helligkeit, Partial-Struktur) und
# leitet daraus einen GS1-Double-Stack-Preset ab:
#
#   * C1EC  ← Lautstärke-Verlauf über die Tastatur
#   * M1EC/M2EC ← Helligkeits-/FM-Index-Verlauf (rich tief, rein hoch)
#   * ATE/DTE/DTE1Scaling ← Attack-/Decay-Zeiten
#   * Stack 2 (ADD) ← hellerer Anschlags-Transient (Hammer)
#
# Ausgabe: out/gs1_<name>_gen.h  (PatchConsts-Initializer, einfügefertig)
#          out/report.txt        (Feature-Tabelle + Fit-Begründung)
#
# Nur numpy nötig (FFT aus numpy.fft). Engine-Konvention:
#   KNOTE = MIDI - 21   (27.5 Hz = A0 = MIDI 21 = KNOTE 0)
#   ecIdx = floor(KNOTE/2) + 2   → Zonen 2..45 über die Tastatur
#   EC[zone] = (1 - D_dB*42.667/4095)^10   (D_dB = Dämpfung ggü. lautester Stelle)
# ============================================================
import numpy as np, wave, os, sys, glob, math

ENGINE_SR = 34687          # interne Sample-Rate des Emulators (für Zeit→Rate)
AMP_PER_DB = 4095.0 / 96.0 # ~42.667 Attenuation-Units pro dB
EA_FULL    = 0xFFFFF       # max Envelope-Level

# ---------- WAV laden ----------
def load_wav(path):
    w = wave.open(path, 'rb')
    sr = w.getframerate(); n = w.getnframes()
    raw = w.readframes(n); w.close()
    x = np.frombuffer(raw, dtype=np.int16).astype(np.float64) / 32768.0
    return x, sr

# ---------- Feature-Extraktion pro Note ----------
def amp_env(x, hop=256):
    m = (len(x)//hop)*hop
    e = np.sqrt((x[:m].reshape(-1, hop)**2).mean(axis=1) + 1e-12)
    return e

def spectral_centroid(seg, sr):
    win = seg * np.hanning(len(seg))
    sp = np.abs(np.fft.rfft(win)); fr = np.fft.rfftfreq(len(win), 1/sr)
    return float((fr*sp).sum() / (sp.sum() + 1e-12))

def harmonic_amps(x, f0, sr, nh=10, a=0.15, b=0.65):
    seg = x[int(a*sr):int(b*sr)]
    if len(seg) < 1024: return np.zeros(nh)
    win = seg * np.hanning(len(seg))
    sp = np.abs(np.fft.rfft(win)); fr = np.fft.rfftfreq(len(win), 1/sr)
    out = np.zeros(nh)
    for h in range(1, nh+1):
        t = f0*h
        if t > sr/2 - 50: break
        band = (fr > t - f0*0.3) & (fr < t + f0*0.3)
        if band.any(): out[h-1] = sp[band].max()
    return out

def analyze_note(path, midi):
    x, sr = load_wav(path)
    f0 = 440.0 * 2**((midi - 69)/12.0)        # erwartete Tonhöhe aus Dateiname
    env = amp_env(x)
    pk = env.max(); pki = int(env.argmax())
    peak_db = 20*math.log10(pk + 1e-9)
    atk_s = pki*256/sr
    # Decay-Slope (dB/s) im Bereich nach dem Peak (linearer Fit auf 20log10(env))
    tail = env[pki:]
    if len(tail) > 8:
        tdb = 20*np.log10(tail + 1e-9)
        tt = np.arange(len(tdb))*256/sr
        # nur bis -40 dB unter Peak fitten (danach Rauschboden)
        m = tdb > (tdb[0] - 40)
        if m.sum() > 4:
            slope = np.polyfit(tt[m], tdb[m], 1)[0]   # dB/s (negativ)
        else:
            slope = -10.0
    else:
        slope = -10.0
    cen_atk = spectral_centroid(x[:int(0.06*sr)], sr)
    cen_sus = spectral_centroid(x[int(0.8*sr):int(1.4*sr)], sr)
    har = harmonic_amps(x, f0, sr)
    h1 = har[0] + 1e-9
    richness_db = 10*math.log10((har[1:].sum() + 1e-9) / h1)  # Obertöne vs Grundton
    return dict(midi=midi, f0=f0, peak_db=peak_db, atk_s=atk_s,
                decay_dbps=slope, cen_atk=cen_atk, cen_sus=cen_sus,
                richness_db=richness_db, peak=pk)

# ---------- EC-Inversion: gewünschte Dämpfung (dB) → EC-Zonenwert ----------
def db_to_ec(att_db):
    att_db = max(0.0, att_db)
    gain01 = 1.0 - (att_db * AMP_PER_DB) / 4095.0
    gain01 = max(1e-4, min(1.0, gain01))
    return max(0.0, min(1.0, gain01**10))

def midi_to_eczone(midi):
    knote = midi - 21
    return int(math.floor(knote/2) + 2)        # 2..45

def build_ec_curve(notes, value_db_fn, floor_db=42.0):
    """Baut ein 46-Element-EC-Array. value_db_fn(note)→Dämpfung in dB (0=lauteste/hellste
    Stelle). Zonen 2..45 aus den Noten, 0/1 = Kurven-Endpunkte (0,1)."""
    ec = [0.0]*46
    ec[0] = 0.0; ec[1] = 1.0
    zone_db = {}
    for nf in notes:
        z = midi_to_eczone(nf['midi'])
        zone_db.setdefault(z, []).append(value_db_fn(nf))
    # fehlende Zonen per Nachbar-Interpolation füllen
    last = None
    for z in range(2, 46):
        if z in zone_db:
            d = float(np.mean(zone_db[z])); last = d
        elif last is not None:
            d = last
        else:
            d = 0.0
        d = min(d, floor_db)
        ec[z] = db_to_ec(d)
    return ec

def fmt_arr(a, perline=12):
    s = []
    for i in range(0, len(a), perline):
        chunk = ", ".join(f"{v:.4f}" for v in a[i:i+perline])
        s.append("      " + chunk)
    return "{\n" + ",\n".join(s) + " }"

# ---------- Hauptlauf ----------
def main():
    sample_dir = sys.argv[1] if len(sys.argv) > 1 else "samples/Steinway"
    name       = sys.argv[2] if len(sys.argv) > 2 else "Steinway"
    files = sorted(glob.glob(os.path.join(sample_dir, "*.wav")))
    notes = []
    for f in files:
        base = os.path.basename(f)
        try: midi = int(base.split('-')[0])
        except ValueError: continue
        if 12 <= midi <= 120:
            notes.append(analyze_note(f, midi))
    notes.sort(key=lambda d: d['midi'])
    if not notes:
        print("Keine Samples gefunden in", sample_dir); sys.exit(1)

    # ---- Referenzwerte / Aggregation ----
    peak_max = max(n['peak_db'] for n in notes)
    rich_max = max(n['richness_db'] for n in notes)

    # C1EC: Lautstärke-Verlauf (Dämpfung ggü. lautester Note)
    c1ec = build_ec_curve(notes, lambda n: peak_max - n['peak_db'], floor_db=36.0)
    # M1EC/M2EC: Helligkeit/FM-Index (Dämpfung ggü. hellster/reichster Note)
    m1ec = build_ec_curve(notes, lambda n: (rich_max - n['richness_db'])*1.0, floor_db=48.0)
    m2ec = build_ec_curve(notes, lambda n: (rich_max - n['richness_db'])*1.3, floor_db=48.0)
    c2ec = list(c1ec)  # zweiter Carrier folgt der Lautstärkekurve

    # ---- Hüllkurven: Referenznote nahe C4 (MIDI 60) ----
    ref = min(notes, key=lambda n: abs(n['midi']-60))
    knote = ref['midi'] - 21
    atk_scale = 1.0 + 3.0*knote/87.0
    dec_scale = 0.5 + 2.5*knote/87.0
    # ATE: Rate, sodass Attack-Zeit der Referenz getroffen wird
    atk_s = max(ref['atk_s'], 0.002)
    ate_base = EA_FULL / (atk_s * ENGINE_SR * atk_scale)
    # DTE: aus Decay-Slope (dB/s) → EA/sample → /Keyboardfaktor
    EA_PER_DB = 256.0 * AMP_PER_DB
    dbps = abs(ref['decay_dbps'])
    dt = dbps * EA_PER_DB / ENGINE_SR
    dte_base = max(1.0, dt / dec_scale)

    # DTE1Scaling: aus Verhältnis Decay(tief) vs Decay(hoch)
    lo = min(notes, key=lambda n: n['midi']); hi = max(notes, key=lambda n: n['midi'])
    ratio = abs(hi['decay_dbps']) / (abs(lo['decay_dbps']) + 1e-6)
    dte1 = float(np.clip(0.8 + 0.4*ratio, 1.0, 4.0))

    # ---- Double-Stack: heller Anschlags-Layer (Hammer) ----
    # Mittlerer Helligkeits-Überschuss Attack ggü. Sustain → Layer-Helligkeit
    cen_a = np.mean([n['cen_atk'] for n in notes])
    cen_s = np.mean([n['cen_sus'] for n in notes])
    atk_bright_ratio = cen_a / (cen_s + 1e-6)

    # ---- Ratio-Prior (Klavier: harmonisch, Oktav-C2 + Hammer-M2) ----
    ratio4 = [1.0, 2.0, 1.0, 4.0]
    detune4 = [0, 4, 0, 12]

    # ---- Vollständiges Base-Preset (Stufe-1-Seed) als Dict + .gs1-Datei ----
    P = dict(
        Ratio=ratio4, Detune=detune4,
        C1EC=c1ec, C2EC=c2ec, M1EC=m1ec, M2EC=m2ec,
        ATE=[ate_base, ate_base, ate_base*1.3, ate_base*1.3],
        DTE1Scaling=[dte1],
        DTE=[round(dte_base), round(dte_base), max(1, round(dte_base*1.2)), 40],
        RTE=[80, 80, 80, 80], IL=[0, 0, 0, 0], SL=[0, 0, 0, 0], FMmode=[0, 0],
        DS_Detune=[0.0, 0.0, 0.0, 0.0], DS_MixBase=[1.0], DS_MixLayer=[0.7],
        DS_ATScale=[0.5, 0.5, 0.4, 0.4], DS_DTScale=[120.0, 200.0, 160.0, 10.0],
        DS_Ratio=[1.0, 1.0, 2.0, 6.0], DS_FMmode=[0, 0],
        DS_LevelDb=[6.0, 4.0, 2.0, 0.0], DS_ModKbdDb=[3.0], DS_DTKbdTrack=[0.4],
        DS_MixMode=[1], BaseLevelDb=[0.0, 0.0, 0.0, 0.0], OutLevelDb=[0.0],
        DTEKbdScale=[3.0],
        doublestack=[1],
    )
    gs1_path = os.path.join("tools/sample2gs1/out", f"{name.lower()}_base.gs1")
    with open(gs1_path, "w") as fh:
        for k, v in P.items():
            fh.write(k + " " + " ".join(repr(float(x)) for x in v) + "\n")

    # ---- Report ----
    rep = []
    rep.append(f"sample2gs1 — {name}  ({len(notes)} Noten, MIDI {notes[0]['midi']}..{notes[-1]['midi']})")
    rep.append("")
    rep.append(f"{'midi':>4} {'f0':>7} {'peak_dB':>7} {'atk_ms':>7} {'dec_dB/s':>8} {'rich_dB':>7} {'cenA':>6} {'cenS':>6}")
    for n in notes[::6]:
        rep.append(f"{n['midi']:>4} {n['f0']:7.1f} {n['peak_db']:7.1f} {n['atk_s']*1000:7.1f} "
                   f"{n['decay_dbps']:8.1f} {n['richness_db']:7.1f} {n['cen_atk']:6.0f} {n['cen_sus']:6.0f}")
    rep.append("")
    rep.append("=== Abgeleitete GS1-Parameter ===")
    rep.append(f"Ratio        = {ratio4}")
    rep.append(f"Detune       = {detune4}")
    rep.append(f"ATE base     = {ate_base:.0f}   (Ref MIDI {ref['midi']}, atk {atk_s*1000:.0f} ms)")
    rep.append(f"DTE base     = {dte_base:.1f}   (Ref decay {ref['decay_dbps']:.1f} dB/s)")
    rep.append(f"DTE1Scaling  = {dte1:.2f}   (decay lo/hi ratio {ratio:.2f})")
    rep.append(f"Attack-Helligkeit/Sustain = {atk_bright_ratio:.2f}  → Stack-2-Layer (ADD)")
    rep.append(f"C1EC zonen[2,10,20,30,40,45] = " + ", ".join(f"{c1ec[i]:.3f}" for i in [2,10,20,30,40,45]))
    rep.append(f"M1EC zonen[2,10,20,30,40,45] = " + ", ".join(f"{m1ec[i]:.3f}" for i in [2,10,20,30,40,45]))
    report = "\n".join(rep)
    print(report)
    with open(os.path.join("tools/sample2gs1/out", "report.txt"), "w") as fh:
        fh.write(report + "\n")

    # ---- C++ PatchConsts-Header emittieren ----
    var = f"gs1_{name}Gen"
    ate = f"{{{ate_base:.0f}.0f, {ate_base:.0f}.0f, {ate_base*1.3:.0f}.0f, {ate_base*1.3:.0f}.0f}}"
    dte = f"{{{round(dte_base)}, {round(dte_base)}, {max(1,round(dte_base*1.2))}, 40}}"
    # Stack-2-Layer: kurzer heller Hammer (ADD)
    ds_level = "{6.0f, 4.0f, 2.0f, 0.0f}"
    h = []
    h.append(f"#pragma once")
    h.append(f"// AUTO-GENERIERT von tools/sample2gs1/analyze.py aus '{name}'-Samples.")
    h.append(f"// {len(notes)} Noten analysiert. Stufe-1-Heuristik (vor CMA-ES-Feinschliff).")
    h.append(f'#include "gs1_presets.h"   // PatchConsts-Definition')
    h.append(f"")
    h.append(f"static const PatchConsts {var} = {{")
    h.append(f"    {{{ratio4[0]:.1f}f, {ratio4[1]:.1f}f, {ratio4[2]:.1f}f, {ratio4[3]:.1f}f}}, // Ratio")
    h.append(f"    {{{detune4[0]}, {detune4[1]}, {detune4[2]}, {detune4[3]}}},               // Detune")
    h.append(f"    {fmt_arr(c1ec)}, // C1EC")
    h.append(f"    {fmt_arr(c2ec)}, // C2EC")
    h.append(f"    {fmt_arr(m1ec)}, // M1EC")
    h.append(f"    {fmt_arr(m2ec)}, // M2EC")
    h.append(f"    {ate}, // ATE")
    h.append(f"    {dte1:.2f}f, // DTE1Scaling")
    h.append(f"    {dte}, // DTE")
    h.append(f"    {{80, 80, 80, 80}},     // RTE")
    h.append(f"    {{0, 0, 0, 0}},         // IL")
    h.append(f"    {{0, 0, 0, 0}},         // SL")
    h.append(f"    {{0, 0}},               // FMmode")
    h.append(f'    "{name} Gen",')
    h.append(f"    {{0.0f, 0.0f, 0.0f, 0.0f}}, // DS_Detune (reiner Layer)")
    h.append(f"    1.0f, 0.7f,             // DS_MixBase : DS_MixLayer")
    h.append(f"    {{0.5f, 0.5f, 0.4f, 0.4f}}, // DS_ATScale (schneller Hammer)")
    h.append(f"    {{120.0f, 200.0f, 160.0f, 10.0f}}, // DS_DTScale (kurzer Anschlag)")
    h.append(f"    {{1.0f, 1.0f, 2.0f, 6.0f}}, // DS_Ratio (heller Hammer-Cluster)")
    h.append(f"    {{0, 0}},               // DS_FMmode")
    h.append(f"    {ds_level}, // DS_LevelDb")
    h.append(f"    3.0f,                   // DS_ModKbdDb")
    h.append(f"    0.4f,                   // DS_DTKbdTrack")
    h.append(f"    1,                      // DS_MixMode = ADD")
    h.append(f"    {{0.0f, 0.0f, 0.0f, 0.0f}}, // BaseLevelDb")
    h.append(f"    0.0f                    // OutLevelDb")
    h.append(f"}};")
    outp = os.path.join("tools/sample2gs1/out", f"gs1_{name}_gen.h")
    with open(outp, "w") as fh:
        fh.write("\n".join(h) + "\n")
    print(f"\n→ Preset geschrieben: {outp}")

if __name__ == "__main__":
    main()
