#!/usr/bin/env python3
"""Turn a full game's siege snapshot (lab v21 SMAP/SCITY/SUNIT lines in Lua.log) into a battle-lab scenario (lab v22).

Usage: make_stall_scenario.py <Lua.log> <city name as logged> <turn> <scenario name> [turns]
Prints a Lua table for SiegeLabScenarios.lua: the ground within 4 tiles, the active civ's land units within 4 at their
offsets and hit points (armyAt), the enemy's (defendersAt; the unit inside the city becomes the garrison), and the
city's population is left for the caller to set (Lua.log has the city's hit points and strength, not its buildings).
"""
import sys

def main():
    log, city, turn, name = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    turns = int(sys.argv[5]) if len(sys.argv) > 5 else 15
    smap, scity, units, active = None, None, [], None
    for line in open(log, encoding="utf-8", errors="replace"):
        if "[SiegeLab] " not in line:
            continue
        body = line.split("[SiegeLab] ", 1)[1].strip()
        f = body.split(",")
        if f[0] == "SMAP" and f[2] == city:
            smap = f
        elif f[0] == "SCITY" and f[2] == city and int(f[1]) == turn:
            scity = f
        elif f[0] == "SUNIT" and f[2] == city and int(f[1]) == turn:
            units.append(f)
        elif f[0] == "OPEN" and active is None:
            active = int(f[2])
    if not smap or not scity:
        sys.exit("no SMAP/SCITY for %s on turn %d" % (city, turn))
    cy = int(smap[4])
    owner = int(smap[5])
    army, defenders, garrison = [], [], None
    for u in units:
        o, utype, dx, dy, hp = int(u[3]), u[4], int(u[6]), int(u[7]), int(u[9])
        if o == active:
            army.append('{ "%s", %d, %d, %d }' % (utype, dx, dy, hp))
        elif o == owner:
            if dx == 0 and dy == 0:
                garrison = utype
            else:
                defenders.append('{ "%s", %d, %d, %d }' % (utype, dx, dy, hp))
    hp, maxhp, strength = int(scity[3]), int(scity[4]), int(scity[5])
    print("\t-- %s on turn %d of the full game (%s): city %d of %d hp, strength %d" % (city, turn, log, hp, maxhp, strength))
    print('\t{ name = "%s", turns = %d, ground = { y = %d, map = "%s" },' % (name, turns, cy, smap[8]))
    capital = len(smap) > 9 and smap[9] == "1"
    if len(smap) > 11:
        print("\t-- owner knew %s techs (defenderTechCount); building defense %s; capital %s" % (smap[10], smap[11], smap[9]))
    print("\t  city = { population = %s,%s buildings = { %s }%s%s }," % (scity[6] if len(scity) > 6 else "POP", " capital = true," if capital else "",
        '"BUILDING_WALLS"' if maxhp > 200 else "", (', garrison = "%s"' % garrison) if garrison else "",
        (", damage = %d" % (maxhp - hp)) if hp < maxhp else ""))
    print("\t  armyAt = { %s }," % ", ".join(army))
    print("\t  defendersAt = { %s } }," % ", ".join(defenders))

main()
