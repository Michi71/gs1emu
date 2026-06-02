#!/usr/bin/env python3
# ============================================================
# gs1_to_header — wandelt ein .gs1-Textpreset in einen einfügefertigen
# C++ PatchConsts-Initializer um (z.B. das CMA-ES-optimierte steinway_opt.gs1).
#
# Nutzung:
#   python3 tools/sample2gs1/gs1_to_header.py <preset.gs1> <VarName> "<Display Name>" [out.h]
# Beispiel:
#   python3 tools/sample2gs1/gs1_to_header.py out/steinway_opt.gs1 \
#       gs1_SteinwayOpt "Steinway Opt" presets/gs1_Steinway_opt.h
#
# Die Feldreihenfolge entspricht exakt PatchConsts in emulator/src/gs1_presets.h.
# ============================================================
import sys, os

# (key, typ 'f'/'i', count) — Reihenfolge = PatchConsts. Name kommt nach FMmode.
SCHEMA = [
    ("Ratio", 'f', 4), ("Detune", 'i', 4),
    ("C1EC", 'f', 46), ("C2EC", 'f', 46), ("M1EC", 'f', 46), ("M2EC", 'f', 46),
    ("ATE", 'f', 4), ("DTE1Scaling", 'f', 1),
    ("DTE", 'i', 4), ("RTE", 'i', 4), ("IL", 'i', 4), ("SL", 'i', 4),
    ("FMmode", 'i', 2),
    ("__NAME__", 's', 1),
    ("DS_Detune", 'f', 4), ("DS_MixBase", 'f', 1), ("DS_MixLayer", 'f', 1),
    ("DS_ATScale", 'f', 4), ("DS_DTScale", 'f', 4), ("DS_Ratio", 'f', 4),
    ("DS_FMmode", 'i', 2), ("DS_LevelDb", 'f', 4), ("DS_ModKbdDb", 'f', 1),
    ("DS_DTKbdTrack", 'f', 1), ("DS_MixMode", 'i', 1),
    ("BaseLevelDb", 'f', 4), ("OutLevelDb", 'f', 1), ("DTEKbdScale", 'f', 1),
]

def read_gs1(path):
    d = {}
    for line in open(path):
        t = line.split()
        if t: d[t[0]] = [float(x) for x in t[1:]]
    return d

def fmt_f(v):  # kompakt, gültiges C++-Float-Literal (immer mit Dezimalpunkt + f)
    s = f"{v:.5f}".rstrip('0')
    if s.endswith('.'): s += '0'
    return s + "f"

def fmt_vals(vals, typ):
    if typ == 'i':
        return ", ".join(str(int(round(v))) for v in vals)
    return ", ".join(fmt_f(v) for v in vals)

def emit_array(vals, typ, perline):
    parts = []
    for i in range(0, len(vals), perline):
        parts.append("      " + fmt_vals(vals[i:i+perline], typ))
    return "{\n" + ",\n".join(parts) + " }"

def main():
    if len(sys.argv) < 4:
        print("usage: gs1_to_header.py <preset.gs1> <VarName> \"<Display Name>\" [out.h]")
        sys.exit(1)
    src, var, disp = sys.argv[1], sys.argv[2], sys.argv[3]
    out = sys.argv[4] if len(sys.argv) > 4 else None
    d = read_gs1(src)

    header = [
        "#pragma once",
        f"// AUTO-GENERIERT von tools/sample2gs1/gs1_to_header.py aus '{os.path.basename(src)}'.",
        "// CMA-ES-optimiertes Preset (Stufe 2). Feldreihenfolge = PatchConsts.",
        '#include "gs1_presets.h"',
        "",
        f"static const PatchConsts {var} = {{",
    ]
    # Jedes Feld als (value_str, comment). Komma kommt VOR den Kommentar,
    # beim letzten Feld entfällt es.
    fields = []  # (value_str, comment)
    for key, typ, cnt in SCHEMA:
        if key == "__NAME__":
            fields.append((f'    "{disp[:16]}"', "Name")); continue
        vals = d.get(key, [0.0]*cnt)
        if len(vals) < cnt: vals = vals + [0.0]*(cnt-len(vals))
        vals = vals[:cnt]
        if cnt == 1:
            v = (str(int(round(vals[0]))) if typ == 'i' else fmt_f(vals[0]))
            fields.append((f"    {v}", key))
        elif cnt <= 4:
            fields.append((f"    {{{fmt_vals(vals, typ)}}}", key))
        else:  # EC-Arrays (46 Werte, 12 pro Zeile)
            fields.append((f"    {emit_array(vals, typ, 12)}", key))
    body = []
    for i, (vs, cm) in enumerate(fields):
        sep = "," if i < len(fields)-1 else ""
        body.append(f"{vs}{sep} // {cm}")
    text = "\n".join(header) + "\n" + "\n".join(body) + "\n};\n"

    if out:
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        with open(out, "w") as f: f.write(text)
        print(f"→ geschrieben: {out}")
    else:
        print(text)

if __name__ == "__main__":
    main()
