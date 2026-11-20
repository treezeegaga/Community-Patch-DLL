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

`ab-78d5e8a` (M1 iterations 3 and 7 accepted, plus logging), runs m1f7a/b/c (2026-11-19, newAI attacker, lab v9,
6 battles per scenario): 68 of 90 captured (recounted 2026-11-19 by implementer 4 from the results files; earlier
records said 65, an arithmetic slip; the per-scenario rows were right). Previous: m1f3 (`ab-401f952`, iteration 3 only), 62 of 90.

| Scenario | Captured | Capture turns | Units lost (mean) | Bar (5/6 by turn) | Meets M1 bar |
|---|---|---|---|---|---|
| ancient_rush_vs_open_city | 6/6 | 2 | 0.0 | 3 | yes |
| small_vs_weak | 6/6 | 2 | 0.0 | 3 | yes |
| small_vs_medium | 6/6 | 6 | 0.0 | 10 | yes (stock 4/6) |
| small_vs_strong | 0/6 | | 1.7 | not takeable | |
| medium_vs_weak | 6/6 | 2 | 0.0 | 3 | yes |
| medium_vs_medium | 6/6 | 3-4 | 0.0 | 6 | yes |
| medium_vs_strong | 2/6 | 6, 9 | 0.0 | not takeable | |
| large_vs_weak | 6/6 | 2 | 0.0 | 3 | yes |
| large_vs_medium | 6/6 | 3 | 0.0 | 6 | yes |
| large_vs_strong | 6/6 | 6-8 | 0.5 | 12 | yes (stock 2/6) |
| ref_cb4_melee6_vs_walls | 6/6 | 5 | 0.0 | 9 | yes |
| melee_only_vs_medium | 0/6 | | 0.0 | 7 | **no**, set aside by the user (2026-11-19) |
| ranged_only_vs_weak | 0/6 | | 0.0 | not takeable | |
| siege_train_vs_strong | 6/6 | 6-8 | 0.0 | 9 | yes (stock 2/6) |
| medium_vs_medium_rough | 6/6 | 5 | 0.0 | 12 | yes (stock 0/6) |

## New AI as defender (M1 bar, defender part)

`ab-78d5e8a`, 2026-11-19, stock attacker, 6 battles per scenario: against the new-AI defender (m1v9a/b/c) 55 of 90
captured; against the stock defender on the same build and lab (m1s9a/b/c) 57 of 90. No scenario more than 1 in 6
worse for the defender (large_vs_strong 3/6 vs 2/6). Met.

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

## Regression set (PLAN.md 8a item 2)

Run reg0 (2026-11-19, `ab-78d5e8a`, lab v13, BOX; new AI as attacker rgn0a/b/c, stock rgs0a/b/c, 6 battles each,
clean). **Rep 2 of regress_capital and regress_coastal is void** in every launch: it came after rep 1's artillery
battle, the attacker's era stayed industrial, and its classical army was disbanded as "long obsolete", one unit a turn
(fixed in lab v15). Those two rows are from the rerun `2026-11-19-22-reg1` (same build, rgn1a/b/c and rgs1a/b/c, 6 battles each, clean).

| Scenario | New AI: captured, turns | Stock: captured, turns | Attackers lost (mean, new / stock) |
|---|---|---|---|
| regress_capital (reg1) | 6/6, turn 5 | 6/6, turn 5 | 0.0 / 0.0 |
| regress_coastal (reg1) | 6/6, turns 4-5 | 6/6, turns 4-5 | 0.0 / 0.0 |
| regress_medieval | 6/6, turns 3-4 | 6/6, turn 3 | 0.0 / 0.0 |
| regress_artillery | 6/6, turns 6-8 | **1/6**, turn 7 (city at 30-92% most damage) | 0.0 / 0.3 |

New AI on `ab-455fe7a` (M2 iteration 2), lab v16, m2x3a/b/c, 6 battles each: capital 6/6 (turns 4-5), coastal 6/6
(4-5), medieval 6/6 (3-4), artillery 6/6 (6-12), losses 0 except artillery 0.2.

## March (M2)

12 tiles from home unless named; 25 turns; King, Quick. Script = scripted attacker (the reference; m2r0, m2r1, m2r3,
m2r5). New AI = `ab-8893a66` (M2 iterations 2 and 3), lab v17, m2n5b/c/d and m2n6b/c/d, 6 battles each. Stock =
same build and lab, m2s7a/b/c and m2s8a/b/c, 6 battles each. Proposed bar (for the user's ruling): capture in at
least 5 of 6 within 1.5 times the script's median capture turn.

| Scenario | Script: captured, median turn | New AI: captured, turns | Units lost (new AI) | Within 1.5x | Proposed bar | Stock |
|---|---|---|---|---|---|---|
| march_medium_vs_weak | 6/6, 12.5 | 6/6, 9-9 | 0.0 | 6 of 6 (by 18.75) | meets | 0/6 |
| march_medium_vs_medium | 6/6, 9 | 5/6, 12-13 | 0.2 | 5 of 6 (by 13.5) | meets | 0/6 |
| march_large_vs_medium | 6/6, 10 | 6/6, 11-13 | 0.0 | 6 of 6 (by 15) | meets | 0/6 |
| march_medium_vs_medium_d8 | 2/2, 11 | 5/6, 10-11 | 0.3 | 5 of 6 (by 16.5) | meets | 0/6 |
| march_medium_vs_medium_d10 | 2/2, 8 | 6/6, 10-10 | 0.0 | 6 of 6 (by 12) | meets | 0/6 |
| march_medium_vs_medium_d16 | 2/2, 17 | 6/6, 16-23 | 0.3 | 6 of 6 (by 25.5) | meets | 0/6 |
| march_medium_vs_medium_rough | 3/6, 25 | 6/6, 12-15 | 0.0 | 6 of 6 (by 37.5) | meets | 0/6 |
| march3_vs_medium_d8 | 6/6, 7 | 6/6, 9-11 | 0.3 | 5 of 6 (by 10.5) | meets | 0/6 |
| march3_vs_medium | 6/6, 9.5 | 6/6, 11-11 | 0.0 | 6 of 6 (by 14.25) | meets | 0/6 |
| march3_vs_medium_d16 | 6/6, 12 | 6/6, 15-16 | 0.0 | 6 of 6 (by 18) | meets | 0/6 |
| march3_vs_medium_rough | 6/6, 11 | 6/6, 17-18 | 0.0 | 0 of 6 (by 16.5) | **misses** | 0/6 |

### Current build: `ab-9238090` (M2 iterations 2, 3, 4b), lab v17, m2nb/m2nc a-c, 6 battles each

| Scenario | Script median turn | Bar (1.5x) | New AI: captured, turns | Within bar | Units lost |
|---|---|---|---|---|---|
| march_medium_vs_weak | 12.5 | 18.75 | 6/6, 9-9 | 6 of 6: meets | 0.0 |
| march_medium_vs_medium | 9 | 13.5 | 6/6, 11-13 | 6 of 6: meets | 0.2 |
| march_large_vs_medium | 10 | 15 | 6/6, 10-14 | 6 of 6: meets | 0.0 |
| march_medium_vs_medium_d8 | 11 | 16.5 | 6/6, 10-11 | 6 of 6: meets | 0.0 |
| march_medium_vs_medium_d10 | 8 | 12 | 6/6, 10-11 | 6 of 6: meets | 0.0 |
| march_medium_vs_medium_d16 | 17 | 25.5 | 5/6, 15-16 | 5 of 6: meets | 0.3 |
| march_medium_vs_medium_rough | 25 | 37.5 | 6/6, 11-12 | 6 of 6: meets | 0.0 |
| march3_vs_medium_d8 | 7 | 10.5 | 6/6, 9-9 | 6 of 6: meets | 0.0 |
| march3_vs_medium | 9.5 | 14.25 | 6/6, 11-11 | 6 of 6: meets | 0.0 |
| march3_vs_medium_d16 | 12 | 18 | 6/6, 15-16 | 6 of 6: meets | 0.0 |
| march3_vs_medium_rough | 11 | 16.5 | 6/6, 13-16 | 6 of 6: meets | 0.2 |

Total 65 of 66. Same build: siege set 69 of 90 (m2gb), regress set 24 of 24 (m2xb). The table above is iteration 3.

Stock never leaves home at 10+ tiles and barely engages at 8 (most damage 2-13%), with the corridor shown or not.

## Current build: `ab-7aa945f` (M3 iteration 2: both "too early" gates use the one siege estimate)

Lab v17, new AI as attacker, `2026-11-20-05-m3f2`, 6 battles per scenario (regress 2). Siege set 71 of 90 (previous
build 69 of 90, m3e1 on `ab-69b9d72`); march 42 of 42 and march3 24 of 24, every battle within the M2 bar; regress 8 of 8.

| Scenario | Captured | Capture turns | Units lost (mean) | Previous (m3e1) |
|---|---|---|---|---|
| ancient_rush_vs_open_city | 6/6 | 2 | 0.0 | 6/6 |
| small_vs_weak | 6/6 | 2 | 0.0 | 6/6 |
| small_vs_medium | 6/6 | 6-7 | 0.0 | 5/6 |
| small_vs_strong | 0/6 | | 2.3 | 0/6 |
| medium_vs_weak | 6/6 | 2 | 0.0 | 6/6 |
| medium_vs_medium | 6/6 | 3-4 | 0.0 | 6/6 |
| medium_vs_strong | 4/6 | 9-11 | 0.2 | 4/6 |
| large_vs_weak | 6/6 | 2 | 0.0 | 6/6 |
| large_vs_medium | 6/6 | 3 | 0.0 | 6/6 |
| large_vs_strong | 6/6 | 5-6 | 0.0 | 6/6 |
| ref_cb4_melee6_vs_walls | 6/6 | 5-6 | 0.0 | 6/6 |
| melee_only_vs_medium | 1/6 | 11 | 0.0 | 0/6 |
| ranged_only_vs_weak | 0/6 | | 0.0 | 0/6 |
| siege_train_vs_strong | 6/6 | 6-8 | 0.2 | 6/6 |
| medium_vs_medium_rough | 6/6 | 4-5 | 0.0 | 6/6 |

March (bar = 1.5x the script's median turn): medium_vs_weak 6/6 t9; medium_vs_medium 6/6 t11; large_vs_medium 6/6
t10-13; d8 6/6 t10; d10 6/6 t10; d16 6/6 t16-20; rough 6/6 t11-12; march3 d8 6/6 t9-10; march3 6/6 t11; march3 d16
6/6 t15-16; march3 rough 6/6 t12-13. Units lost 0.0 except d16 0.5.

## Current build: `ab-b196ee4` (M3 iterations 2 and 3b)

Lab v17, new AI as attacker, `2026-11-20-07-m3f4`, 6 battles per scenario (regress 2). Siege set **76 of 90**
(`ab-7aa945f`: 71); march 41 of 42 and march3 24 of 24, all 11 scenarios meet the M2 bar; regress 8 of 8. All 14
takeable siege scenarios meet the M1 bar (melee_only_vs_medium now 6/6, turns 5-7).

| Scenario | Captured | Capture turns | Units lost (mean) |
|---|---|---|---|
| ancient_rush_vs_open_city | 6/6 | 2 | 0.0 |
| small_vs_weak | 6/6 | 2 | 0.0 |
| small_vs_medium | 6/6 | 6 | 0.0 |
| small_vs_strong | 0/6 | | 1.3 |
| medium_vs_weak | 6/6 | 2 | 0.0 |
| medium_vs_medium | 6/6 | 3-4 | 0.0 |
| medium_vs_strong | 4/6 | 8-12 | 0.0 |
| large_vs_weak | 6/6 | 2 | 0.0 |
| large_vs_medium | 6/6 | 3 | 0.0 |
| large_vs_strong | 6/6 | 5-7 | 0.3 |
| ref_cb4_melee6_vs_walls | 6/6 | 4 | 0.0 |
| melee_only_vs_medium | 6/6 | 5-7 | 0.0 |
| ranged_only_vs_weak | 0/6 | | 0.0 |
| siege_train_vs_strong | 6/6 | 6-7 | 0.3 |
| medium_vs_medium_rough | 6/6 | 4-5 | 0.0 |

March: medium_vs_weak 6/6 t9; medium_vs_medium 6/6 t11; large_vs_medium 6/6 t10-13; d8 5/6 t10-11; d10 6/6 t9-12;
d16 6/6 t15-17; rough 6/6 t11; march3 d8 6/6 t9-13 (5 within 10.5); march3 6/6 t11; march3 d16 6/6 t15-16; march3
rough 6/6 t13-14. Regress: capital t4, coastal t4, medieval t3-4, artillery t6-8; no losses.
