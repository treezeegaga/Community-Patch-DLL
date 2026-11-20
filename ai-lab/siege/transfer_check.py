#!/usr/bin/env python3
"""AI lab: the transfer check (PLAN.md 8a item 4). Reads the logs of opening-mode games (-Set transfer or opening) and
prints, per game: the active civ's cities, army and wars at the last turn; every city capture; and how often each lab
fix's log line appears for that civ in the game's AI logs, so you can see whether the fixes fire in a real game.

Usage: python3 -I ai-lab/siege/transfer_check.py <logs dir or .zip> [...]
The active civ is the RUN line's second field (Songhai on the lab PCs); its player id is the first OPEN line's id with
that name.
"""
import io
import os
import re
import sys
import zipfile

# (log file, pattern, label): lines written by the active civ (the row's second field) that mark a lab fix at work
MARKERS = [
    ("PlayerTacticalAILog.csv", "siege estimate for .*: TAKE", "estimate TAKE (M3 it. 1)"),
    ("PlayerTacticalAILog.csv", "siege estimate NO", "too early by the estimate (M3 it. 2)"),
    ("PlayerTacticalAILog.csv", "capture attempt on .* executed", "capture attempt executed"),
    ("PlayerTacticalAILog.csv", "melee closing in on", "melee closing in (M1 it. 7)"),
    ("PlayerTacticalAILog.csv", "ranged closing in on", "ranged closing in (M2 it. 4b)"),
    ("PlayerTacticalAILog.csv", "melee on city .*: allowed", "melee hit on a city allowed"),
    ("PlayerEconomyAILog.csv", "Not exploring with", "not made an explorer near a city at war (M1 it. 3)"),
    ("OperationalAILog.csv", "Started Army", "army started (operations)"),
    ("MilitaryAILog.csv", "preferred attack target", "preferred attack target (M2 it. 2 range)"),
]
OPEN_FIELDS = ["turn", "id", "civ", "newAI", "cities", "pop", "techs", "era", "gold", "goldRate", "science", "policies",
               "happiness", "melee", "ranged", "siege", "mounted", "naval", "upgradeGold", "settlers", "workers", "wars",
               "might"]


def read(src, name):
    if os.path.isdir(src):
        p = os.path.join(src, name)
        return open(p, encoding="latin-1").read() if os.path.exists(p) else ""
    with zipfile.ZipFile(src) as z:
        for n in z.namelist():
            if n.endswith("/" + name) or n == name:
                return io.TextIOWrapper(z.open(n), encoding="latin-1").read()
    return ""


def game(src):
    lua = read(src, "Lua.log")
    run, opens, captures, done = None, [], [], False
    for line in lua.splitlines():
        m = re.search(r"\[SiegeLab\] (\w+),?(.*)$", line)
        if not m:
            continue
        kind, rest = m.group(1), m.group(2).split(",")
        if kind == "RUN":
            run = rest
        elif kind == "OPEN" and len(rest) == len(OPEN_FIELDS):
            row = dict(zip(OPEN_FIELDS, rest))
            opens.append(row)
        elif kind == "CAPTURE":
            captures.append(rest)
        elif kind == "DONE":
            done = True
    if not run:
        print(f"{src}: no RUN line")
        return
    civ = run[1]
    mine = [r for r in opens if r["civ"] == civ]
    me = mine[0]["id"] if mine else "?"
    names = {r["id"]: r["civ"] for r in opens}
    last = mine[-1] if mine else None
    print(f"== {os.path.basename(src.rstrip('/'))}: {civ} (player {me}, new AI {last['newAI'] if last else '?'}), "
          f"{len(set(r['id'] for r in opens))} majors, map {run[3]}x{run[4]}, {'finished' if done else 'NOT FINISHED'}")
    if last:
        first_war = next((r["turn"] for r in mine if int(r["wars"]) > 0), "never")
        print(f"   turn {last['turn']}: cities {last['cities']}, pop {last['pop']}, techs {last['techs']}, era {last['era']}, "
              f"gold {last['gold']} ({last['goldRate']}/turn), land melee {last['melee']}, ranged {last['ranged']}, "
              f"siege {last['siege']}; at war with {last['wars']} majors (first war turn {first_war})")
    for c in captures:
        turn, old, new, cap = c[0], c[1], c[2], c[3]
        who = lambda i: names.get(i, "city-state/barbarian " + i)
        mark = "  <== ours" if new == me else ("  <== from us" if old == me else "")
        print(f"   capture turn {turn}: {who(old)} -> {who(new)}{' (capital)' if cap == '1' else ''}{mark}")
    for fname, pat, label in MARKERS:
        text = read(src, fname)
        rx = re.compile(pat)
        n = sum(1 for line in text.splitlines() if f", {civ}," in line[:40] and rx.search(line))
        print(f"   {label}: {n}{'' if text else ' (no ' + fname + ')'}")


def main(argv):
    for src in argv:
        game(src)


if __name__ == "__main__":
    main(sys.argv[1:])
