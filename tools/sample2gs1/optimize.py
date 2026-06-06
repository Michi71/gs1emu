#!/usr/bin/env python3
# ============================================================
# sample2gs1 — Stufe 2: CMA-ES Analysis-by-Synthesis
# ------------------------------------------------------------
# Verfeinert das Stufe-1-Base-Preset, indem es es über libgs1emu rendert
# und gegen die Ziel-Samples optimiert (Multi-Noten Spektral- + Hüllkurven-
# Loss). Python besitzt die Parametrisierung; die C++-CLI gs1_render_params
# rendert nur (kein Recompile pro Iteration).
#
# Nutzung:
#   python3 tools/sample2gs1/optimize.py <sample_dir> <name> [iters]
# Voraussetzung: tools/sample2gs1/out/<name>_base.gs1 (von analyze.py),
#                tools/sample2gs1/out/gs1_render_params (kompiliert).
# ============================================================
import numpy as np, wave, os, sys, subprocess, math, cma

OUT = "tools/sample2gs1/out"
CLI = os.path.join(OUT, "gs1_render_params")
ENGINE_SR = 34687
DUR = 2.0
AMP_PER_DB = 4095.0 / 96.0
EVAL_NOTES = [33, 43, 48, 60, 72, 84]   # A1 G2 C3 C4 C5 C6 (low-lastig für Bias)
def note_weight(midi):                  # tiefe Töne stärker gewichten
    return 2.0 if midi <= 50 else 1.0

# ---------- .gs1 I/O ----------
def read_gs1(path):
    d = {}
    for line in open(path):
        t = line.split()
        if t: d[t[0]] = [float(x) for x in t[1:]]
    return d

def write_gs1(d, path):
    with open(path, "w") as f:
        for k, v in d.items():
            f.write(k + " " + " ".join(repr(float(x)) for x in v) + "\n")

# ---------- EC <-> dB ----------
def ec_to_db(ec):
    ec = min(1.0, max(1e-6, ec))
    return (1.0 - ec**0.1) * 4095.0 / AMP_PER_DB
def db_to_ec(db):
    g = 1.0 - max(0.0, db) * AMP_PER_DB / 4095.0
    g = min(1.0, max(1e-4, g))
    return min(1.0, max(0.0, g**10))

def tilt_curve(ec46, tilt_db):
    """EC-Kurve (Zonen 2..45) um tilt_db kippen: hohe Zonen +tilt/2, tiefe -tilt/2."""
    out = list(ec46)
    for z in range(2, 46):
        frac = (z - 2) / 43.0            # 0..1 über die Tastatur
        d = ec_to_db(ec46[z]) + tilt_db * (frac - 0.5) * 2.0
        out[z] = db_to_ec(d)
    return out

# ---------- Suchraum, normiert [0,1] ----------
#            name            lo     hi     seed
SPEC = [
    # Operator-Ratios (kontinuierlich, weit, NICHT-ganzzahlig erlaubt → auch
    # inharmonische Partials für Metall/Holz/Glocke). C1 ist fest 1.0 (Referenz).
    ("c2_ratio",            0.5,   8.0,   2.0),
    ("m1_ratio",            0.5,  16.0,   1.0),
    ("m2_ratio",            0.5,  16.0,   4.0),
    ("out_db",            -12.0,  12.0,   0.0),
    ("base_mod_db",       -15.0,  20.0,   0.0),
    ("mod_tilt_db",       -18.0,  18.0,   0.0),
    ("car_tilt_db",        -8.0,   8.0,   0.0),
    ("log_ate",            -1.2,   1.2,   0.0),
    ("log_dte",            -1.5,   1.5,   0.0),
    ("dte1",                1.0,   4.0,   2.0),
    ("ds_mixlayer",         0.0,   2.0,   0.7),
    ("ds_at",               0.6,   2.0,   1.0),    # Layer-Attack ~ Body (kein Doppelanschlag)
    ("sl_level",            0.0, 255.0,   0.0),    # Sustain-Level (0=perkussiv, hoch=Orgel/Streicher)
    ("ds_dt",               5.0, 400.0, 150.0),
    ("ds_lvl_db",          -6.0,  16.0,   4.0),
    ("ds_modkbd",           0.0,   8.0,   3.0),
    ("ds_dtkbd",            0.0,   1.0,   0.4),
    ("det_c2",              0.0,  20.0,   4.0),
    ("det_m2",              0.0,  30.0,  12.0),
    ("dtkbd_scale",         0.8,   3.0,   3.0),
    ("ds_ratio_m1",         0.5,   8.0,   2.0),   # Layer-Cluster, M1 (Knock-Grundpartial)
    ("ds_ratio_m2",         1.0,  12.0,   6.0),   # Layer-Cluster, M2 (heller Knock/Ting)
]
def seed_norm(over=None):
    """Normierte Startbelegung; over={dim_name: absoluter Wert} überschreibt einzelne."""
    over = over or {}
    out = []
    for nm, lo, hi, s in SPEC:
        v = over.get(nm, s)
        out.append((min(hi, max(lo, v)) - lo) / (hi - lo))
    return out
def denorm(xn):
    return { nm: lo + min(1,max(0,xn[i]))*(hi-lo) for i,(nm,lo,hi,_) in enumerate(SPEC) }

SNAP_HARMONIC = False   # harmonisch ODER sustained → Ratios auf Ganzzahl
SUSTAINED     = False   # gehaltener Klang (Orgel/Streicher): schneller Attack, BLEND-Chorus

def snap_ratio(r):
    """Auf harmonisches Raster runden: <0.75 → 0.5 (Sub-Oktave), sonst Ganzzahl."""
    return 0.5 if r < 0.75 else float(round(r))

def decode(xn, base):
    """Suchvektor (normiert) + Base-Preset → vollständiges Preset-Dict."""
    p = {k: list(v) for k, v in base.items()}
    g = denorm(xn)
    p["OutLevelDb"]   = [g["out_db"]]
    bm = g["base_mod_db"]
    p["BaseLevelDb"]  = [0.0, 0.0, bm, bm]                 # Modulatoren M1,M2
    p["M1EC"]         = tilt_curve(base["M1EC"], g["mod_tilt_db"])
    p["M2EC"]         = tilt_curve(base["M2EC"], g["mod_tilt_db"])
    p["C1EC"]         = tilt_curve(base["C1EC"], g["car_tilt_db"])
    p["C2EC"]         = tilt_curve(base["C2EC"], g["car_tilt_db"])
    am = math.exp(g["log_ate"]); dm = math.exp(g["log_dte"])
    ate_floor = 250.0 if SUSTAINED else 40.0   # sustained: schneller, gleichmäßiger Attack
    p["ATE"]          = [max(ate_floor, a*am) for a in base["ATE"]]
    p["DTE"]          = [max(1, round(d*dm)) for d in base["DTE"]]
    p["SL"]           = [int(round(g["sl_level"]))]*4            # Sustain-Level (Orgel/Streicher)
    p["DTE1Scaling"]  = [g["dte1"]]
    p["DS_MixLayer"]  = [g["ds_mixlayer"]]
    lv = g["ds_lvl_db"]
    p["DS_LevelDb"]   = [0.0, 0.0, lv, lv]
    p["DS_ModKbdDb"]  = [g["ds_modkbd"]]
    p["DS_DTKbdTrack"]= [g["ds_dtkbd"]]
    if SUSTAINED:
        # Sustained: Layer = sanfter BLEND-Chorus, der die Basis-Hüllkurve verfolgt
        # (kein perkussiver ADD-Transient → keine zeitversetzten Doppel-Anschläge).
        p["DS_MixMode"] = [0]
        p["DS_ATScale"] = [1.0]*4
        p["DS_DTScale"] = [1.0]*4
    else:
        p["DS_ATScale"] = [g["ds_at"]]*4
        p["DS_DTScale"] = [g["ds_dt"]]*4
    det = list(base["Detune"]); det[1] = g["det_c2"]; det[3] = g["det_m2"]
    p["Detune"]       = det
    p["DTEKbdScale"]  = [g["dtkbd_scale"]]
    c2, m1, m2 = g["c2_ratio"], g["m1_ratio"], g["m2_ratio"]
    if SNAP_HARMONIC:   # harmonisch/sustained → Ratios aufs harmonische Raster (in Tune)
        c2, m1, m2 = snap_ratio(c2), snap_ratio(m1), snap_ratio(m2)
    p["Ratio"]        = [1.0, c2, m1, m2]                                   # Basis-Ratios
    p["DS_Ratio"]     = [1.0, 1.0, g["ds_ratio_m1"], g["ds_ratio_m2"]]      # Layer-Cluster suchbar
    return p

# ---------- Audio-Features (SR-unabhängig vergleichbar) ----------
GRID = np.geomspace(50, 16000, 384)   # feineres Log-Frequenz-Grid → inharmon. Peaks
def active_window(x, sr, drop_db=40.0, lo=0.25, hi=1.5):
    """Aktives Zeitfenster (s) eines Tons: vom Onset bis der RMS drop_db unter den
    Peak fällt (für kurze perkussive Klänge → nicht ins Rauschen analysieren)."""
    hop = 256; m = (len(x)//hop)*hop
    if m < hop*2: return 0.05, hi
    e = np.sqrt((x[:m].reshape(-1, hop)**2).mean(1) + 1e-12)
    pk = e.max(); pki = int(e.argmax()); thr = pk * 10**(-drop_db/20)
    end = len(e)
    for i in range(pki, len(e)):
        if e[i] < thr: end = i; break
    t1 = min(hi, max(lo, end*hop/sr))
    return 0.05, t1
def harmonicity(x, sr, f0, win=(0.1, 1.0), nh=16):
    """Anteil der Energie im aktiven Fenster nahe GANZZAHLIGEN Vielfachen von f0.
    ~1 = harmonisch (EP/Klavier), niedrig = inharmonisch (Metall/Holz/Glocke)."""
    a, b = int(win[0]*sr), int(win[1]*sr); seg = x[a:b]
    if len(seg) < 2048: return 1.0
    sp = np.abs(np.fft.rfft(seg*np.hanning(len(seg)))); fr = np.fft.rfftfreq(len(seg), 1/sr)
    nyq = min(16000, sr/2); band = (fr > f0*0.5) & (fr < nyq)
    total = sp[band].sum() + 1e-9; hsum = 0.0
    for h in range(1, nh+1):
        t = f0*h
        if t > nyq: break
        m = (fr > t - f0*0.06) & (fr < t + f0*0.06)   # ±6 % Fenster
        if m.any(): hsum += sp[m].sum()
    return float(min(1.0, hsum/total))
def spec_db(x, sr, win=(0.1, 1.5)):
    a, b = int(win[0]*sr), int(win[1]*sr); seg = x[a:b]
    fft, hop = 2048, 1024
    if len(seg) < fft: return np.full(len(GRID), -60.0)
    acc = None; cnt = 0
    for i in range(0, len(seg)-fft, hop):
        sp = np.abs(np.fft.rfft(seg[i:i+fft]*np.hanning(fft)))
        acc = sp if acc is None else acc+sp; cnt += 1
    acc /= max(cnt, 1); fr = np.fft.rfftfreq(fft, 1/sr)
    mag = np.interp(GRID, fr, acc)
    db = 20*np.log10(mag + 1e-9); db -= db.max()
    return np.clip(db, -60, 0)
def attack_spec(x, sr, dur=0.12):
    """Onset-Spektrum (erste dur s) → erfasst Anschlags-Transienten/Vor-Klingeln."""
    seg = x[:int(dur*sr)]
    if len(seg) < 512: return np.full(len(GRID), -60.0)
    sp = np.abs(np.fft.rfft(seg*np.hanning(len(seg)))); fr = np.fft.rfftfreq(len(seg), 1/sr)
    mag = np.interp(GRID, fr, sp); db = 20*np.log10(mag + 1e-9); db -= db.max()
    return np.clip(db, -60, 0)
def env_db(x, npts=200):
    hop = 256; m = (len(x)//hop)*hop
    e = np.sqrt((x[:m].reshape(-1, hop)**2).mean(1) + 1e-12)
    g = np.interp(np.linspace(0,1,npts), np.linspace(0,1,len(e)), e)
    db = 20*np.log10(g + 1e-9); db -= db.max()
    return np.clip(db, -60, 0)

NH = 12          # Anzahl betrachteter Harmonischer
NYQ_CMP = 16000  # gemeinsame Nyquist (Ziel-Samples 32 kHz)
def harm_db(x, sr, f0, win=(0.15, 1.2)):
    """Erste NH Partialamplituden (dB relativ zum Grundton) + Gültigkeitsmaske.
    Lange Analysefenster → feine Auflösung, trennt auch tiefe Harmonische sauber."""
    a, b = int(win[0]*sr), int(win[1]*sr); seg = x[a:b]
    db = np.full(NH, -60.0); mask = np.zeros(NH, bool)
    if len(seg) < 4096: return db, mask
    sp = np.abs(np.fft.rfft(seg*np.hanning(len(seg)))); fr = np.fft.rfftfreq(len(seg), 1/sr)
    amps = np.zeros(NH)
    for h in range(1, NH+1):
        t = f0*h
        if t > NYQ_CMP - 50: break
        bnd = (fr > t - f0*0.4) & (fr < t + f0*0.4)
        if bnd.any(): amps[h-1] = sp[bnd].max(); mask[h-1] = True
    ref = amps[0] + 1e-9
    db = np.clip(20*np.log10(amps/ref + 1e-9), -60, 10)
    return db, mask
def harm_loss(dr, mr, dt, mt):
    m = mr & mt
    return float(np.mean(np.abs(dr[m] - dt[m]))) if m.any() else 0.0

def load_wav(path):
    w = wave.open(path, 'rb'); sr = w.getframerate(); n = w.getnframes()
    x = np.frombuffer(w.readframes(n), dtype=np.int16).astype(np.float64)/32768.0
    return x, sr

import glob
def pick_samples(sample_dir, vel="hi"):
    """Pro Note EINEN Velocity-Layer (NNN-VVV.wav). vel: hi/lo/mid."""
    byn = {}
    for f in glob.glob(os.path.join(sample_dir, "*.wav")):
        parts = os.path.basename(f)[:-4].split("-")
        if len(parts) < 2: continue
        try: m, v = int(parts[0]), int(parts[1])
        except ValueError: continue
        if 12 <= m <= 120: byn.setdefault(m, []).append((v, f))
    out = {}
    for m, lst in byn.items():
        lst.sort()
        out[m] = (lst[-1] if vel == "hi" else lst[0] if vel == "lo"
                  else lst[len(lst)//2])[1]
    return out

# ---------- Render via CLI ----------
def render(preset_path, notes):
    out = subprocess.run([CLI, preset_path, str(DUR)] + [str(n) for n in notes],
                         capture_output=True).stdout
    a = np.frombuffer(out, dtype=np.float32).astype(np.float64)
    per = int(ENGINE_SR*DUR)
    return [a[i*per:(i+1)*per] for i in range(len(notes))]

def main():
    sample_dir = sys.argv[1] if len(sys.argv) > 1 else "samples/Steinway"
    name       = sys.argv[2] if len(sys.argv) > 2 else "Steinway"
    iters      = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    vel        = sys.argv[4] if len(sys.argv) > 4 else "hi"
    base = read_gs1(os.path.join(OUT, f"{name.lower()}_base.gs1"))

    # Eval-Noten: Schnittmenge aus Wunschliste und tatsächlich vorhandenen Samples
    sel = pick_samples(sample_dir, vel)
    eval_notes = [n for n in EVAL_NOTES if n in sel] or sorted(sel)[::max(1, len(sel)//5)]
    print(f"Velocity '{vel}', Eval-Noten: {eval_notes}")

    # Ziel-Features cachen (Spektrum, Hüllkurve, Harmonik)
    def f0_of(midi): return 440.0 * 2**((midi-69)/12.0)
    tgt = {}
    for nmidi in eval_notes:
        x, sr = load_wav(sel[nmidi]); f0 = f0_of(nmidi)
        tw = active_window(x, sr)               # aktives Fenster aus dem Ziel
        hd, hm = harm_db(x, sr, f0, tw)
        hrm = harmonicity(x, sr, f0, tw)
        tgt[nmidi] = (spec_db(x, sr, tw), env_db(x), hd, hm, attack_spec(x, sr), hrm, tw)
    wsum = sum(note_weight(n) for n in eval_notes)
    harm_mean = np.mean([tgt[n][5] for n in eval_notes])
    sus_mean  = np.mean([tgt[n][6][1] for n in eval_notes])   # Ø aktive Fensterlänge (s)
    global SNAP_HARMONIC, SUSTAINED
    SUSTAINED = sus_mean > 1.2          # klingt kaum ab → gehaltener Klang (Orgel/Streicher)
    SNAP_HARMONIC = (harm_mean > 0.65) or SUSTAINED   # sustained inharmonisch = Dissonanz → snappen
    print(f"Harmonizität (Ø) = {harm_mean:.2f}, Ø Fenster = {sus_mean:.2f}s  → "
          f"{'SUSTAINED (schneller Attack, BLEND-Chorus, ganzz. Ratios)' if SUSTAINED else ('harmonisch (ganzz. Ratios)' if SNAP_HARMONIC else 'perkussiv/inharmonisch (freie Ratios)')}")

    tmp = os.path.join(OUT, "_eval.gs1")
    def make_loss(b):
        def loss(xn):
            write_gs1(decode(xn, b), tmp)
            ren = render(tmp, eval_notes)
            tot = 0.0
            for k, nmidi in enumerate(eval_notes):
                gs = ren[k]; w = note_weight(nmidi)
                if gs.size < 4096 or not np.isfinite(gs).all() or np.abs(gs).max() < 1e-5:
                    tot += w*50.0; continue    # stille/kaputte Stimme bestrafen
                sd, ed, hd, hm, ad, hrm, tw = tgt[nmidi]
                ls = np.mean(np.abs(spec_db(gs, ENGINE_SR, tw) - sd))
                le = np.mean(np.abs(env_db(gs) - ed))
                rd, rm = harm_db(gs, ENGINE_SR, f0_of(nmidi), tw)
                lh = harm_loss(rd, rm, hd, hm)
                la = np.mean(np.abs(attack_spec(gs, ENGINE_SR) - ad))  # Onset/Vor-Klingeln
                # Spektrum stärker bei inharmonischen Klängen; Ganzzahl-Harmonik-Term
                # nur soweit der Klang harmonisch ist (sonst irreführend).
                w_spec = 0.5 + (1.0 - hrm)*0.8
                tot += w*(w_spec*ls + hrm*1.0*lh + 0.4*le + 0.5*la)
            return tot / wsum
        return loss

    def run_cma(b, niter, x0, sigma=0.25):
        es = cma.CMAEvolutionStrategy(x0, sigma, {
            'bounds': [0, 1], 'maxiter': niter, 'popsize': 12, 'verb_disp': 0, 'seed': 1})
        es.optimize(make_loss(b))
        return es.result.xbest, es.result.fbest

    def with_fmmode(b, fmmode):
        nb = {k: list(v) for k, v in b.items()}; nb["FMmode"] = [float(x) for x in fmmode]
        return nb
    def ratio_seed(r):   # Ratios als Startbelegung der kontinuierlichen Such-Dims
        return seed_norm({"c2_ratio": r[1], "m1_ratio": r[2], "m2_ratio": r[3]})

    print(f"Start-Loss (Stufe-1-Seed): {make_loss(base)(seed_norm()):.3f}\n")

    # ---- Phase 1: Ratio-Seed-Screen (von harmonisch bis inharmonisch/hoch) ----
    # Startpunkte für die kontinuierliche Ratio-Suche — CMA-ES verfeinert sie
    # danach frei (auch nicht-ganzzahlig).
    RATIOS = [[1,1,1,2], [1,1,1,3], [1,2,1,4],          # EP / Klavier (harmonisch)
              [1,1,3,4], [1,1,4,7], [1,1,7,7],          # heller / metallisch
              [1,1,4,9], [1,2,5,8],                     # Glocke / Vibraphon (hoch)
              [1,1,3.5,7], [1,2,5.5,11]]                # inharmonisch (Metall/Holz)
    screen = max(12, iters // 3)
    print(f"=== Ratio-Seed-Screen ({screen} Iter je Kandidat) ===")
    results = []
    for r in RATIOS:
        xb, fb = run_cma(base, screen, ratio_seed(r))
        results.append((fb, r, xb)); print(f"  Ratio {r}  → Loss {fb:.3f}")
    results.sort(key=lambda t: t[0])
    best_loss0, best_ratio, best_x = results[0]
    print(f"→ bester Ratio-Seed: {best_ratio}  (Loss {best_loss0:.3f})")

    # ---- Phase 2: FMmode-Screen (warm gestartet vom besten Ratio-Ergebnis) ----
    # 0=NORM, 1/2=Self-Feedback (leicht/stark, reedy/buzzy), 3=Cross-Mod.
    FMMODES = [[0,0], [1,1], [2,2], [2,1], [1,0], [2,0], [3,0], [3,3]]
    screen2 = max(10, iters // 4)
    print(f"\n=== FMmode-Screen ({screen2} Iter je Kandidat) ===")
    fres = []
    for fm in FMMODES:
        xb, fb = run_cma(with_fmmode(base, fm), screen2, best_x)
        fres.append((fb, fm)); print(f"  FMmode {fm}  → Loss {fb:.3f}")
    fres.sort(key=lambda t: t[0])
    best_fm = fres[0][1]
    print(f"→ bester FMmode: {best_fm}  (Loss {fres[0][0]:.3f})")

    # ---- Finaler, längerer Lauf (warm gestartet, FMmode fixiert) ----
    bfin = with_fmmode(base, best_fm)
    print(f"\n=== Finale Optimierung ({iters} Iter, FMmode {best_fm}) ===")
    es = cma.CMAEvolutionStrategy(best_x, 0.18, {
        'bounds': [0, 1], 'maxiter': iters, 'popsize': 12, 'verb_disp': 8, 'seed': 1})
    es.optimize(make_loss(bfin))
    xbest = es.result.xbest
    print(f"\nEnd-Loss: {es.result.fbest:.3f}")

    best = decode(xbest, bfin)
    opt_path = os.path.join(OUT, f"{name.lower()}_opt.gs1")
    write_gs1(best, opt_path)
    g = denorm(xbest)
    print(f"\n=== Optimierte Parameter (FMmode {best_fm}) ===")
    print(f"  Ratio = {[round(r,2) for r in best['Ratio']]}  "
          f"(Seed-Start {best_ratio}, {'ganzzahlig gerundet' if SNAP_HARMONIC else 'frei'})")
    for nm, _, _, _ in SPEC:
        print(f"  {nm:14s} = {g[nm]:.3f}")
    print(f"\n→ optimiertes Preset: {opt_path}")

if __name__ == "__main__":
    main()
