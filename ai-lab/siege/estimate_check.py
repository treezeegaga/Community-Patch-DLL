#!/usr/bin/env python3
"""AI lab (M3): compare the siege estimate ("siege estimate for <city>: TAKE|NO, turn t, ...", PlayerTacticalAILog.csv)
on each battle's first attacker turn with what happened (Lua.log RESULT lines).

Usage: python3 -I ai-lab/siege/estimate_check.py <logs dir or .zip> [...]
Prints one row per battle and a per-scenario summary: battles, captured, first-turn estimate TAKE count, and whether
the majority estimate agrees with the majority outcome. Attacker civ: Songhai.
"""
import collections
import io
import os
import re
import sys
import zipfile

START = re.compile(r"\[SiegeLab\] START,([^,]+),(\d+),(\d+),")
RESULT = re.compile(r"\[SiegeLab\] RESULT,([^,]+),(\d+),(\d+),(-?\d+),")
EST = re.compile(r"^(\d+), Songhai, siege estimate for (\S+): (TAKE|NO), turn (-?\d+), melee left (\d+), city hp at end (\d+); "
                 r"city hp (\d+) of (\d+), heal (\d+), strike (\d+); within 4: melee (\d+), ranged (\d+)")


def read(src, name):
    if os.path.isdir(src):
        p = os.path.join(src, name)
        return open(p, encoding="latin-1").read() if os.path.exists(p) else ""
    with zipfile.ZipFile(src) as z:
        for n in z.namelist():
            if n.endswith("/" + name) or n == name:
                return io.TextIOWrapper(z.open(n), encoding="latin-1").read()
    return ""


def battles(src):
    lua = read(src, "Lua.log")
    starts, results = [], {}
    for line in lua.splitlines():
        m = START.search(line)
        if m:
            starts.append((m.group(1), int(m.group(2)), int(m.group(3))))
        m = RESULT.search(line)
        if m:
            results[(m.group(1), int(m.group(2)))] = (int(m.group(3)), int(m.group(4)))
    ests = []
    for line in read(src, "PlayerTacticalAILog.csv").splitlines():
        m = EST.match(line)
        if m:
            ests.append((int(m.group(1)), m.group(3), int(m.group(4)), int(m.group(11)), int(m.group(12)), int(m.group(7))))
    out = []
    for i, (scen, rep, t0) in enumerate(starts):
        t1 = starts[i + 1][2] if i + 1 < len(starts) else 10 ** 6
        mine = [e for e in ests if t0 < e[0] <= t1]
        first = mine[0] if mine else None
        cap, capturn = results.get((scen, rep), (None, None))
        out.append((scen, rep, cap, capturn, first, len(mine)))
    return out


def main(argv):
    rows = []
    for src in argv:
        for b in battles(src):
            rows.append((os.path.basename(src.rstrip("/")),) + b)
    print("run,scenario,rep,captured,capture_turn,first_estimate,est_turn,melee,ranged,city_hp,estimate_lines")
    for run, scen, rep, cap, ct, first, n in rows:
        f = first or (None, "none", -1, 0, 0, 0)
        print(f"{run},{scen},{rep},{cap},{ct},{f[1]},{f[2]},{f[3]},{f[4]},{f[5]},{n}")
    agg = collections.OrderedDict()
    for run, scen, rep, cap, ct, first, n in rows:
        a = agg.setdefault(scen, [0, 0, 0])
        a[0] += 1
        a[1] += cap == 1
        a[2] += bool(first and first[1] == "TAKE")
    print()
    print("scenario,battles,captured,estimate_take,agrees")
    agree = 0
    for scen, (n, c, t) in agg.items():
        ok = (c * 2 > n) == (t * 2 > n)
        agree += ok
        print(f"{scen},{n},{c},{t},{'yes' if ok else 'NO'}")
    print(f"agree in {agree} of {len(agg)} scenarios")


if __name__ == "__main__":
    main(sys.argv[1:])
