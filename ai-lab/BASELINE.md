# Siege lab baseline

Stock AI (Community Patch 5.4.8 game code plus logging only, build `ab-00ae537`), lab v6, King, Quick pace.
Runs m0d, m0e, m0f (2026-11-15, three launches of the full set at 2 repeats = 6 battles per scenario).
Results in `siege/results/m0[def]-*`. Classes as in `PLAN.md` section 7:
CAP captured; LASTHP city shot to at least 85% damage, not taken; LOWDMG attacked but less; NOATTACK no damage,
army stayed; WALK no damage, army walked 8+ tiles away.

The human column is empty until the user plays the reference battles (PLAN.md M0).

| Scenario | Stock: classes (6 battles) | Agree | Capture turns | Units lost (mean) | Human |
|---|---|---|---|---|---|
| ancient_rush_vs_open_city | CAP 6 | 6/6 | 2 | 0.0 | |
| small_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | |
| small_vs_medium | CAP 4, LOWDMG 1, LASTHP 1 | 4/6 | 7 | 0.2 | |
| small_vs_strong | NOATTACK 4, LOWDMG 2 | 4/6 | | 0.8 | |
| medium_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | |
| medium_vs_medium | CAP 6 | 6/6 | 3-4 | 0.0 | |
| medium_vs_strong | LOWDMG 6 | 6/6 | | 0.8 | |
| large_vs_weak | CAP 6 | 6/6 | 2 | 0.0 | |
| large_vs_medium | CAP 6 | 6/6 | 3-4 | 0.0 | |
| large_vs_strong | LASTHP 3, CAP 2, LOWDMG 1 | 3/6 | 7-8 | 0.0 | |
| ref_cb4_melee6_vs_walls | CAP 6 | 6/6 | 5-6 | 0.0 | |
| melee_only_vs_medium | NOATTACK 6 | 6/6 | | 0.0 | |
| ranged_only_vs_weak | WALK 6 | 6/6 | | 0.0 | |
| siege_train_vs_strong | LASTHP 4, CAP 2 | 4/6 | 5-6 | 0.0 | |
| medium_vs_medium_rough | NOATTACK 3, LASTHP 2, WALK 1 | 3/6 | | 0.0 | |

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
