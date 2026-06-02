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
EVAL_NOTES = [36, 48, 60, 72, 84]   # C2 C3 C4 C5 C6

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

# ---------- Suchraum: 15 Dimensionen, normiert [0,1] ----------
#            name            lo     hi     seed
SPEC = [
    ("out_db",            -12.0,  12.0,   0.0),
    ("base_mod_db",       -15.0,  20.0,   0.0),
    ("mod_tilt_db",       -18.0,  18.0,   0.0),
    ("car_tilt_db",        -8.0,   8.0,   0.0),
    ("log_ate",            -1.2,   1.2,   0.0),
    ("log_dte",            -1.5,   1.5,   0.0),
    ("dte1",                1.0,   4.0,   2.0),
    ("ds_mixlayer",         0.0,   2.0,   0.7),
    ("ds_at",               0.2,   1.5,   0.45),
    ("ds_dt",               5.0, 400.0, 150.0),
    ("ds_lvl_db",          -6.0,  16.0,   4.0),
    ("ds_modkbd",           0.0,   8.0,   3.0),
    ("ds_dtkbd",            0.0,   1.0,   0.4),
    ("det_c2",              0.0,  20.0,   4.0),
    ("det_m2",              0.0,  30.0,  12.0),
    ("dtkbd_scale",         0.8,   3.0,   3.0),
]
def seed_norm():
    return [ (s-lo)/(hi-lo) for (_,lo,hi,s) in SPEC ]
def denorm(xn):
    return { nm: lo + min(1,max(0,xn[i]))*(hi-lo) for i,(nm,lo,hi,_) in enumerate(SPEC) }

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
    p["ATE"]          = [a*am for a in base["ATE"]]
    p["DTE"]          = [max(1, round(d*dm)) for d in base["DTE"]]
    p["DTE1Scaling"]  = [g["dte1"]]
    p["DS_MixLayer"]  = [g["ds_mixlayer"]]
    p["DS_ATScale"]   = [g["ds_at"]]*4
    p["DS_DTScale"]   = [g["ds_dt"]]*4
    lv = g["ds_lvl_db"]
    p["DS_LevelDb"]   = [0.0, 0.0, lv, lv]
    p["DS_ModKbdDb"]  = [g["ds_modkbd"]]
    p["DS_DTKbdTrack"]= [g["ds_dtkbd"]]
    det = list(base["Detune"]); det[1] = g["det_c2"]; det[3] = g["det_m2"]
    p["Detune"]       = det
    p["DTEKbdScale"]  = [g["dtkbd_scale"]]
    return p

# ---------- Audio-Features (SR-unabhängig vergleichbar) ----------
GRID = np.geomspace(60, 16000, 256)
def spec_db(x, sr):
    a, b = int(0.1*sr), int(1.5*sr); seg = x[a:b]
    win, hop = 2048, 1024
    if len(seg) < win: return np.full(len(GRID), -60.0)
    acc = None; cnt = 0
    for i in range(0, len(seg)-win, hop):
        sp = np.abs(np.fft.rfft(seg[i:i+win]*np.hanning(win)))
        acc = sp if acc is None else acc+sp; cnt += 1
    acc /= max(cnt, 1); fr = np.fft.rfftfreq(win, 1/sr)
    mag = np.interp(GRID, fr, acc)
    db = 20*np.log10(mag + 1e-9); db -= db.max()
    return np.clip(db, -60, 0)
def env_db(x, npts=200):
    hop = 256; m = (len(x)//hop)*hop
    e = np.sqrt((x[:m].reshape(-1, hop)**2).mean(1) + 1e-12)
    g = np.interp(np.linspace(0,1,npts), np.linspace(0,1,len(e)), e)
    db = 20*np.log10(g + 1e-9); db -= db.max()
    return np.clip(db, -60, 0)

NH = 12          # Anzahl betrachteter Harmonischer
NYQ_CMP = 16000  # gemeinsame Nyquist (Ziel-Samples 32 kHz)
def harm_db(x, sr, f0):
    """Erste NH Partialamplituden (dB relativ zum Grundton) + Gültigkeitsmaske.
    Lange Analysefenster → feine Auflösung, trennt auch tiefe Harmonische sauber."""
    a, b = int(0.15*sr), int(1.2*sr); seg = x[a:b]
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
    base = read_gs1(os.path.join(OUT, f"{name.lower()}_base.gs1"))

    # Ziel-Features cachen (Spektrum, Hüllkurve, Harmonik)
    def f0_of(midi): return 440.0 * 2**((midi-69)/12.0)
    tgt = {}
    for nmidi in EVAL_NOTES:
        x, sr = load_wav(os.path.join(sample_dir, f"{nmidi:03d}-127.wav"))
        hd, hm = harm_db(x, sr, f0_of(nmidi))
        tgt[nmidi] = (spec_db(x, sr), env_db(x), hd, hm)

    tmp = os.path.join(OUT, "_eval.gs1")
    def make_loss(b):
        def loss(xn):
            write_gs1(decode(xn, b), tmp)
            ren = render(tmp, EVAL_NOTES)
            tot = 0.0
            for k, nmidi in enumerate(EVAL_NOTES):
                gs = ren[k]
                if gs.size < 4096 or not np.isfinite(gs).all() or np.abs(gs).max() < 1e-5:
                    tot += 50.0; continue      # stille/kaputte Stimme bestrafen
                sd, ed, hd, hm = tgt[nmidi]
                ls = np.mean(np.abs(spec_db(gs, ENGINE_SR) - sd))
                le = np.mean(np.abs(env_db(gs) - ed))
                rd, rm = harm_db(gs, ENGINE_SR, f0_of(nmidi))
                lh = harm_loss(rd, rm, hd, hm)
                tot += 0.5*ls + 1.0*lh + 0.4*le     # Harmonik stark gewichtet
            return tot / len(EVAL_NOTES)
        return loss

    def run_cma(b, niter, sigma=0.25):
        es = cma.CMAEvolutionStrategy(seed_norm(), sigma, {
            'bounds': [0, 1], 'maxiter': niter, 'popsize': 12, 'verb_disp': 0, 'seed': 1})
        es.optimize(make_loss(b))
        return es.result.xbest, es.result.fbest

    # ---- Ratio-Suche: Kandidaten kurz screenen, Sieger voll optimieren ----
    RATIOS = [[1,2,1,4], [1,1,1,3], [1,2,3,4], [1,1,2,4], [1,2,1,3]]
    screen = max(12, iters // 3)
    print(f"Start-Loss (Stufe-1-Seed): {make_loss(base)(seed_norm()):.3f}\n")
    print(f"=== Ratio-Screen ({screen} Iter je Kandidat) ===")
    results = []
    for r in RATIOS:
        b = {k: list(v) for k, v in base.items()}; b["Ratio"] = [float(x) for x in r]
        xb, fb = run_cma(b, screen)
        results.append((fb, r, xb)); print(f"  Ratio {r}  → Loss {fb:.3f}")
    results.sort(key=lambda t: t[0])
    best_loss0, best_ratio, _ = results[0]
    print(f"\n→ bester Ratio: {best_ratio}  (Loss {best_loss0:.3f})")

    # ---- Finaler, längerer Lauf auf dem Sieger-Ratio ----
    bfin = {k: list(v) for k, v in base.items()}; bfin["Ratio"] = [float(x) for x in best_ratio]
    print(f"\n=== Finale Optimierung ({iters} Iter, Ratio {best_ratio}) ===")
    es = cma.CMAEvolutionStrategy(seed_norm(), 0.25, {
        'bounds': [0, 1], 'maxiter': iters, 'popsize': 12, 'verb_disp': 8, 'seed': 1})
    es.optimize(make_loss(bfin))
    xbest = es.result.xbest
    print(f"\nEnd-Loss: {es.result.fbest:.3f}")

    best = decode(xbest, bfin)
    opt_path = os.path.join(OUT, f"{name.lower()}_opt.gs1")
    write_gs1(best, opt_path)
    g = denorm(xbest)
    print(f"\n=== Optimierte Parameter (Ratio {best_ratio}) ===")
    for nm, _, _, _ in SPEC:
        print(f"  {nm:14s} = {g[nm]:.3f}")
    print(f"\n→ optimiertes Preset: {opt_path}")

if __name__ == "__main__":
    main()
