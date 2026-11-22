#!/usr/bin/env python3
"""Compare full-game lab runs straight from the ai/v2 branch, without checking out its 18 GB of results.

Usage (from a clone that has fetched origin/ai/v2):
    python3 ai-lab/review/compare_games.py NAME=PREFIX:SEEDS[:WHO] ...

    PREFIX  label prefix of the runs, e.g. nx22N (the label is PREFIX + seed)
    SEEDS   comma-separated seeds, or a-b ranges of the doubled-number seeds (1919-6666 = 1919,2020,...,6666)
    WHO     which players to score: "seat0" (default: the test seat) or "others"

Example, the 48 measuring maps on the build of iteration 22:
    python3 ai-lab/review/compare_games.py "new Songhai=nx22N:1919-6666" "stock Songhai=stx22S:1919-6666" \
        "others new=nx22C:1919-6666:others" "others stock=stx22S:1919-6666:others"

It reads only the lab's own lines in each run's Lua.log (OPEN per civ per turn, CAPTURE per city taken), so it
says what happened, not why; the game's AI logs in the -logs folders say why. With exactly two seat0 groups it
also prints a same-map comparison of the first against the second.
"""
import statistics as st
import subprocess
import sys

REF = "origin/ai/v2"
BASE = "ai-lab/siege/results"
COLS = ("turn,id,civ,newAI,cities,pop,techs,era,gold,goldRate,science,policies,happiness,melee,ranged,siege,"
        "mounted,naval,upgradeGold,settlers,workers,wars,might").split(",")


def git(*args):
    return subprocess.run(["git", *args], capture_output=True, text=True, errors="replace").stdout


DIRS = [line.rsplit("/", 1)[-1] for line in git("ls-tree", "--name-only", f"{REF}", f"{BASE}/").splitlines()]


def load(label):
    runs = sorted(d for d in DIRS if d.startswith(label + "-") and d.endswith("-run")
                  and d[len(label) + 1:len(label) + 9].isdigit())
    if not runs:
        return None
    game = {"open": {}, "caps": [], "civs": {}}
    for line in git("show", f"{REF}:{BASE}/{runs[-1]}/Lua.log").splitlines():
        at = line.find("[SiegeLab] ")
        if at < 0:
            continue
        body = line[at + 11:].strip()
        if body.startswith("OPEN,"):
            f = body.split(",")[1:]
            if len(f) < len(COLS):
                continue
            row = {k: (v if k == "civ" else int(v)) for k, v in zip(COLS, f)}
            game["open"].setdefault(row["id"], {})[row["turn"]] = row
            game["civs"][row["id"]] = row["civ"]
        elif body.startswith("CAPTURE,"):
            turn, old, new, capital = map(int, body.split(",")[1:5])
            game["caps"].append((turn, old, new, capital))
    return game if game["open"] else None


def value_at(game, pid, turn, key):
    rows = game["open"].get(pid, {})
    turns = [t for t in rows if t <= turn]
    return rows[max(turns)][key] if turns else None


def first_turn(game, pid, test):
    for turn in sorted(game["open"].get(pid, {})):
        if test(game["open"][pid][turn]):
            return turn
    return None


def score(label, who):
    game = load(label)
    if not game:
        return []
    players = [0] if who == "seat0" else [p for p in game["civs"] if p != 0]
    out = []
    for pid in players:
        # captures of a major civ's city (the old owner is one of the majors in the OPEN lines)
        taken = [(t, cap) for (t, old, new, cap) in game["caps"] if new == pid and old in game["civs"]]
        out.append({
            "label": label, "civ": game["civs"][pid],
            "first": min((t for t, _ in taken), default=None),
            "capital": min((t for t, cap in taken if cap), default=None),
            "taken": len(taken),
            "lost": sum(1 for (t, old, new, cap) in game["caps"] if old == pid),
            "war": first_turn(game, pid, lambda r: r["wars"] > 0),
            "army": first_turn(game, pid, lambda r: r["melee"] >= 6 and r["ranged"] >= 4),
            **{k + "100": value_at(game, pid, 100, k) for k in ("cities", "pop", "techs", "science", "gold", "might")},
        })
    return out


def median(values):
    values = [v for v in values if v is not None]
    return st.median(values) if values else None


def mean(values):
    values = [v for v in values if v is not None]
    return round(st.mean(values), 1) if values else None


def report(name, rows):
    n = len(rows)
    count = lambda test: sum(1 for r in rows if test(r))
    print(f"\n== {name}: {n} civ-games")
    print(f"  full rush army (6 melee + 4 ranged): {count(lambda r: r['army'] is not None)}, median turn {median(r['army'] for r in rows)}")
    print(f"  at war with a major: {count(lambda r: r['war'] is not None)}, median first turn {median(r['war'] for r in rows)}")
    for cut in (60, 80, 100):
        print(f"  by turn {cut}: a major's city taken in {count(lambda r: r['first'] and r['first'] <= cut)}, "
              f"a capital in {count(lambda r: r['capital'] and r['capital'] <= cut)}")
    print(f"  first-capture turns: {sorted(r['first'] for r in rows if r['first'])}")
    print(f"  wars with no capture: {count(lambda r: r['war'] is not None and r['first'] is None)}; lost a city: {count(lambda r: r['lost'] > 0)}")
    print("  at turn 100: " + ", ".join(f"{k} {mean(r[k + '100'] for r in rows)}" for k in ("cities", "pop", "techs", "science", "gold", "might")))


def seeds_of(spec):
    seeds = []
    for part in spec.split(","):
        if "-" in part:
            lo, hi = part.split("-")
            seeds += [int(str(k) * 2) for k in range(int(lo[:len(lo) // 2]), int(hi[:len(hi) // 2]) + 1)]
        else:
            seeds.append(int(part))
    return seeds


def main(argv):
    if not argv:
        print(__doc__)
        return 1
    groups = []
    for arg in argv:
        name, spec = arg.split("=", 1)
        parts = spec.split(":")
        prefix, seeds, who = parts[0], seeds_of(parts[1]), (parts[2] if len(parts) > 2 else "seat0")
        rows = {s: score(f"{prefix}{s}", who) for s in seeds}
        missing = [s for s, r in rows.items() if not r]
        if missing:
            print(f"({name}: no run found for seeds {missing})")
        groups.append((name, who, rows))
        report(name, [r for s in seeds for r in rows[s]])
    seat0 = [(n, rows) for n, who, rows in groups if who == "seat0"]
    if len(seat0) == 2:
        (name_a, a), (name_b, b) = seat0
        both = [s for s in a if a[s] and b.get(s)]
        print(f"\n== same map, {name_a} minus {name_b}: {len(both)} maps")
        for key in ("cities", "pop", "techs", "science", "might"):
            diffs = [a[s][0][key + "100"] - b[s][0][key + "100"] for s in both
                     if a[s][0][key + "100"] is not None and b[s][0][key + "100"] is not None]
            if diffs:
                print(f"  {key} at turn 100: mean {round(st.mean(diffs), 1)}, better on {sum(d > 0 for d in diffs)}, worse on {sum(d < 0 for d in diffs)}")
        for title, group in (("where the first captured a city", [s for s in both if a[s][0]["first"]]),
                             ("where it did not", [s for s in both if not a[s][0]["first"]])):
            if group:
                print(f"  {title} ({len(group)} maps): " + "; ".join(
                    f"{key} {mean(a[s][0][key + '100'] for s in group)} vs {mean(b[s][0][key + '100'] for s in group)}"
                    for key in ("cities", "pop", "techs")))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
