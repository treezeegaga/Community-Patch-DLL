# Siege lab baseline

Stock AI (Community Patch 5.4.8 game code plus logging only, build `ab-00ae537`), lab v6, King, Quick pace.
Runs m0d, m0e, m0f (2026-11-15, three launches of the full set at 2 repeats = 6 battles per scenario).
Results in `siege/results/m0[def]-*`. Classes as in `PLAN.md` section 7:
CAP captured; LASTHP city shot to at least 85% damage, not taken; LOWDMG attacked but less; NOATTACK no damage,
army stayed; WALK no damage, army walked 8+ tiles away.

The reference is the lab's scripted attacker (rules v3, aggressive) in place of a human player (user rulings of
2026-11-18 in LOG.md): runs ref3a, ref3b, ref3c, same build, same lab, 6 battles per scenario. A scenario is
**takeable** if the stock AI or the script took it in at least 2 of 6 battles; its **reference turns** are the
median capture turn of whichever of the two took it faster. M1 bar: in takeable scenarios, capture in at least 5 of
6 battles within 1.5 times the reference turns; losses don't count.

| Scenario | Stock: classes (6 battles) | Agree | Capture turns | Units lost (mean) | Script: captured, turns, units lost |
|---|---|---|---|---|---|
| ancient_rush_vs_open_city | CAP 6 | 6/6 | 2 | 0.0 | 6/6, 2-3, 0.0 |
| small_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | 6/6, 2-3, 0.0 |
| small_vs_medium | CAP 4, LOWDMG 1, LASTHP 1 | 4/6 | 7 | 0.2 | 2/6, 6 and 9, 2.2 |
| small_vs_strong | NOATTACK 4, LOWDMG 2 | 4/6 | | 0.8 | 0/6, -, 4.7 |
| medium_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | 6/6, 2, 0.0 |
| medium_vs_medium | CAP 6 | 6/6 | 3-4 | 0.0 | 6/6, 4, 0.0 |
| medium_vs_strong | LOWDMG 6 | 6/6 | | 0.8 | 0/6, -, 3.0 |
| large_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | 6/6, 2-3, 0.0 |
| large_vs_medium | CAP 6 | 6/6 | 3-4 | 0.0 | 6/6, 5-6, 0.0 |
| large_vs_strong | LASTHP 3, CAP 2, LOWDMG 1 | 3/6 | 7-8 | 0.0 | 5/6, 8-9, 2.0 |
| ref_cb4_melee6_vs_walls | CAP 6 | 6/6 | 5-6 | 0.0 | 6/6, 5-6, 0.0 |
| melee_only_vs_medium | NOATTACK 6 | 6/6 | | 0.0 | 6/6, 5, 0.7 |
| ranged_only_vs_weak | WALK 6 | 6/6 | | 0.0 | 0/6, -, 1.2 |
| siege_train_vs_strong | LASTHP 4, CAP 2 | 4/6 | 5-6 | 0.0 | 6/6, 5-9, 0.5 |
| medium_vs_medium_rough | NOATTACK 3, LASTHP 2, WALK 1 (lab v6); LASTHP 6 (lab v7, rough1/2) | 3/6; 6/6 | | 0.0 | 6/6, 7-12, 0.5 |

Totals: 50 of 90 captured (17, 17, 16 per launch). No assertions, no "long obsolete" disbands.

Notes on the scenarios that don't agree 5 of 6:
- small_vs_strong: NOATTACK and LOWDMG are the same behaviour (the army barely engages, 0-60 damage); the class
  boundary splits it.
- large_vs_strong and siege_train_vs_strong: the city is always shot to its last hit points; whether the melee walk
  in before turn 12 varies within a launch, not between launches (combat randomness at a threshold).
- small_vs_medium: 4 of 6 captured on turn 7; the other two came close (49% and 92% damage).
- medium_vs_medium_rough: per launch, not per battle (m0d and m0e never attacked, m0f shot the city down in both).
  The rough terrain pattern (hills and forest every other tile at distance 2-3) follows the arena's coordinates,
  which change with the map, so each launch gets a different layout. A lab issue to fix before this scenario counts.

## Current build (new AI as attacker)

`ab-401f952` (M1 iteration 3 accepted), runs m1f3a/b/c (2026-11-19, newAI attacker, lab v9, 6 battles per scenario):
62 of 90 captured.

| Scenario | Captured | Capture turns | Bar (5/6 by turn) | Meets M1 bar |
|---|---|---|---|---|
| ancient_rush_vs_open_city | 6/6 | 2 | 3 | yes |
| small_vs_weak | 6/6 | 2 | 3 | yes |
| small_vs_medium | 6/6 | 6-7 | 10 | yes (stock 4/6) |
| small_vs_strong | 0/6 | | not takeable | |
| medium_vs_weak | 6/6 | 2 | 3 | yes |
| medium_vs_medium | 6/6 | 3-4 | 6 | yes |
| medium_vs_strong | 1/6 | 12 | not takeable | |
| large_vs_weak | 6/6 | 2 | 3 | yes |
| large_vs_medium | 6/6 | 3-4 | 6 | yes |
| large_vs_strong | 6/6 | 6-7 | 12 | yes (stock 2/6) |
| ref_cb4_melee6_vs_walls | 6/6 | 5 | 9 | yes |
| melee_only_vs_medium | 0/6 | | 7 | **no** (never attacks) |
| ranged_only_vs_weak | 0/6 | | not takeable | |
| siege_train_vs_strong | 6/6 | 6-9 | 9 | yes (stock 2/6) |
| medium_vs_medium_rough | 1/6 | 5 | 12 | **no** (melee left behind, last hit points 5/6) |

## Takeable list (proposed for the user's sign-off)

| Scenario | Takeable | Reference turns | Bar (5 of 6 by turn) | Stock AI today |
|---|---|---|---|---|
| ancient_rush_vs_open_city | yes | 2 (both) | 3 | passes (6/6, turn 2) |
| small_vs_weak | yes | 2 (both) | 3 | passes |
| small_vs_medium | yes | 7 (stock AI) | 10 | fails (4/6) |
| small_vs_strong | no | | | neither takes it |
| medium_vs_weak | yes | 2 (both) | 3 | passes |
| medium_vs_medium | yes | 4 (both) | 6 | passes |
| medium_vs_strong | no | | | neither takes it |
| large_vs_weak | yes | 2 (both) | 3 | passes |
| large_vs_medium | yes | 4 (stock AI) | 6 | passes |
| large_vs_strong | yes | 8 (both) | 12 | fails (2/6) |
| ref_cb4_melee6_vs_walls | yes | 6 (both) | 9 | passes |
| melee_only_vs_medium | yes | 5 (script) | 7 | fails (0/6: never attacks) |
| ranged_only_vs_weak | no | | | can't capture without melee; walks home |
| siege_train_vs_strong | yes | 6 (script) | 9 | fails (2/6) |
| medium_vs_medium_rough | yes | 9 (script) | 12 (the battle's limit) | fails (0/6) |

So M1 starts with five failing takeable scenarios: small_vs_medium, large_vs_strong, melee_only_vs_medium,
siege_train_vs_strong and medium_vs_medium_rough. Seven takeable scenarios already pass and must not get worse.
