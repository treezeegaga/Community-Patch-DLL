# Code map

What this project depends on in the game code and in the lab, so a new implementer doesn't relearn it from the logs.
Keep it current at the end of every iteration (PLAN.md section 7, step 8): a new gate found, a new log line, a change
accepted or reverted, a script changed. Line numbers are approximate (5.4.8 base plus `ai/v2`); search by name.
"LOG M1 it. 7" means the LOG.md (or LOG-archive.md) entry for that iteration.

## The switch

- `CvPlayer::IsNewAI()` (CvPlayer.cpp, near `RefreshNewAI`): the one switch (PLAN rule 5). Reads custom game options
  `GAMEOPTION_AI_V2_ALL` (every AI major civ) and `GAMEOPTION_AI_V2_PLAYER_<id>`; cached in `m_bNewAI` (not saved),
  refreshed at turn start, after load and in `gameStartInit`. Lua `Player:IsNewAI()`, `Player:SetNewAI(bool)` (lab
  only: Lua-set options are local to one machine). Game setup: `ai-lab/newai/` adds the "New AI (test build)" option.
  Logs every change to `AILab_NewAI.csv`. LOG M1 it. 1; "new-AI switch at game setup".
- Every accepted behaviour change sits behind it. Current sites (`grep -n "IsNewAI()" *.cpp`):
  `CvEconomicAI::DoReconState`, `CvMilitaryAI::GetArmyPathsFromCity`, `CvTacticalAI::PlotReinforcementMoves`.

## Military AI: choosing a target and starting an operation (M2, M4)

| Function | What it decides | Gates and limits found | Log lines (file) |
|---|---|---|---|
| `CvMilitaryAI::GetArmyPathsFromCity` | which enemy cities an army from a muster city can reach | path length capped at `iMaxNormalizedDistance` = land trade route range: **10** on every map size in CP (`TRADE_ROUTE_BASE_LAND_DISTANCE`, no VP range mod). New AI: x2 = 20 (M2 it. 2, being tested) | "no army path to: <city>, from muster ..., plot distance, max normalized distance" (`MilitaryAILog.csv`) |
| `CvMilitaryAI::UpdateAttackTargets` | the attack target list | rejects reachable cities on approach score or path length | "land target rejected: ..., approach score, path length"; "preferred attack target" (`MilitaryAILog.csv`) |
| `CvMilitaryAI::RequestCityAttack(eTarget, iNumUnitsWillingToBuild, bCareful)` | starts `AI_OPERATION_CITY_ATTACK_LAND/NAVAL/COMBINED` | careful: only preferred targets, one op per target until the last army left our territory; passes `iNumUnitsWillingToBuild` (1-3 from the diplomacy AI) as allowed missing units | (operation log) |
| `CvPlayer::addAIOperation` → `CvAIOperation::PreconditionsAreMet(muster, target, iMaxMissingUnits)` | whether the op may start | `HaveEnoughUnits(fake slots, iMaxMissingUnits)`: starts even if it must build units | |
| `MilitaryAIHelpers::GetCurrentBestFormationTypeForLandAttack` | the army's formation | by **game** era: ancient `MUFORMATION_EARLY_RUSH` (2 req. melee, 2 req. ranged, 2 opt. ranged); classical/medieval `MUFORMATION_SMALL_CITY_ATTACK_FORCE` (**3 req. melee, 2 req. ranged, optional slots melee only**); renaissance/industrial `BASIC_` (3 melee, 3 ranged req.); later `BIGGER_`. Data: `(1) Community Patch/Database Changes/Units/CoreNewUnitFormations.xml` | |

## Operations: recruiting, gathering, moving (M2)

`CvAIOperation.cpp`, `CvArmyAI.cpp`.

| Function | What it decides | Gates and limits | Log lines (`OperationalAILog.csv`) |
|---|---|---|---|
| `CvAIOperation::SetUpArmy` (≈1280) | first state | "Skip recruiting" only if no required slot is open, else "Initial stage is recruiting" | "Initial stage is recruiting", "Started Army ..." |
| `GrabUnitsFromTheReserves` / `FindBestFitReserveUnit` | fills free slots in slot order with the best-scored unit whose `Unit_AITypes` include the slot's primary or secondary type (**a Spearman has DEFENSE, not ATTACK: it can't take an ATTACK/FAST_ATTACK slot**, so a classical army needs a Swordsman or horse for slot 2) | `IsUnitSuitableForRecruitment`: not healing, ≥70% HP, not within `TACTICAL_COMBAT_MAX_TARGET_DISTANCE` of the target, not recently deployed, not explore/settle, reachable from muster. Units matching no free slot are left out (the lab's spare bowmen) | "Found N potential units ...", "Added <unit> to slot k ...", "PRE/POST Recruiting Units for army ...; <slot list, Open>" |
| `CheckTransitionToNextStage` (≈1530), `ARMYAISTATE_WAITING_FOR_UNITS_TO_REINFORCE` | recruiting → gathering | `OperationalAIHelpers::HaveEnoughUnits(slots, 0)` (CvArmyAI.cpp ≈809): at least 1 required filled and free required ≤ optional present / 2. **0 missing allowed here, though the op started allowing 1-3** (it waits for units to be built; the lab's attacker builds none: M2 it. 2 amended) | "Transition to gathering stage" |
| same, `WAITING_FOR_UNITS_TO_CATCH_UP` | gathering → moving | furthest unit within the gather tolerance of the muster | "Transition to movement stage" |
| `MOVING_TO_DESTINATION`, `PercentFromMusterPointToTarget` | progress; abort if stalled | within 2 of goal counts as there | |
| `CvAIOperation::Move` → `CvTacticalAI::PlotArmyMovesCombat` | the army's moves each turn | | |
| Timeout | an op that never moves ends after about 43 turns (seen in m2n1a) | | |

## Tactical AI: the assault (M1)

`CvTacticalAI.cpp`. Logs in `PlayerTacticalAILog.csv`.

| Function | What it decides | Gates and limits found | Log lines |
|---|---|---|---|
| `ExecuteCaptureCityMoves` (≈875) | whether to attack a city this turn | skips "enemy dominated" zones; "too early" when (expected damage − heal) × 13 turns < city HP, where `ComputeTotalExpectedDamage` (≈5011) counts **at most 3 melee, drops wounded**; attacks at high aggression only with >2 melee in reach | "capture attempt on <city> executed/ABORTED: n units in reach, m melee, target visible v"; "melee near <city>: <unit> at distance, hp, moves, available, in reach, move"; "too early for attacking <city>, required damage, expected max damage per turn" |
| `ExecuteAttackWithUnits` → `ExecuteSpotterMove` (≈3279) | needs the city visible | with the tile unseen, only ATTACK/FAST_ATTACK/COUNTER/SKIRMISHER units can spot; an all-ranged group aborts (ranged_only_vs_weak, a real stock failure) | "target not visible, looking for a spotter among n units" |
| `ScoreAttackDamage` (≈7346) | can this unit attack this target from this plot, and its score | `hpLimit` {70,40,20,-5} by aggression; melee on a city refused when the attacker is worn down before the city ("too early", one unit alone; "good first attack" exception if damage received < 23); "damage not worth it"; "suicide". Score = damage dealt − damage taken, so a costly melee hit loses to no attack | "melee on city <city>: <unit> from x:y hp, refused (too early / damage not worth it / suicide) or allowed: <figures>" |
| `PlotReinforcementMoves` (≈2002) | moving extra units toward a zone | stock returns early in a friendly-dominated zone with any melee. **New AI (M1 it. 7, accepted):** land melee within 6 tiles of an enemy city at war, above 1/3 HP, not in an army, go to ring 2 while the city is above half health, ring 1 at half or less | "melee closing in on <city> (ring n): <unit> at distance, hp, move/hold" |
| `PlotHealMoves` (≈1413), `ExecuteHeals` | sends units to heal | second pass sends units missing >20 HP not next to an enemy | "heal (first/second pass): <unit> at x:y, hp a of b" |
| `ReviewUnassignedUnits` (≈2170) | units the tactical AI didn't use go to the homeland AI | the homeland AI then garrisons/sentries them at home | "unassigned after tactical moves, left to homeland: <unit> at x:y, army a" |
| The two estimates disagree | `ComputeTotalExpectedDamage` vs `ScoreAttackDamage` | the single "can this army take this city" function is M3's first item (LOG M1 it. 8) | |

Reverted attempts on these (don't repeat without new evidence): M1 it. 4 (count neighbours in "too early"), it. 6
and 8 (`MeleeArmyCanWearDownCity` army test), M1 it. 2's heal theory (wrong cause).

## Other AI code with known effects

| Function | Effect | Status |
|---|---|---|
| `CvEconomicAI::DoReconState` (≈3510) | turns combat units into explorers, farthest from home first (the army at the enemy's walls), units older than 3 turns | **New AI (M1 it. 3, accepted):** not within 5 tiles of a revealed enemy city at war (`IsNearEnemyCityAtWar`). Log "Not exploring with <unit>: near an enemy city at war" / "Creating new land explorer" (`PlayerEconomyAILog.csv`) |
| `CvEconomicAI::DisbandLongObsoleteUnits` (≈4376) | scraps units more than 2 eras behind | the lab resets research (`clearResearch`, lab v6) so it doesn't fire; "Disbanding long obsolete unit" (`PlayerHomelandAILog.csv`) |
| `CvHomelandAI` | moves units the tactical AI left: "garrison", "sentry point", "patrolling to", "UNIT DIVERSITY - WE NEED attack" | `PlayerHomelandAILog.csv` |
| `CvGame::ApplyConfigLoggingSettings` (`CvGame::init`) | reads `LoggingEnabled`/`AILog` from `config.ini` (the pack's DLL never gets `[Debug]` otherwise) | logging only, not behind the switch (M0 it. 2) |
| `Player:LabEndOperations()` (Lua, lab only, `1ca9bea`) | ends all of a player's AI operations | called by the lab for both sides at battle setup (lab v13) |
| `PlayerPreAIUnitUpdate` Lua hook | fired before the AI's unit update; the scripted attacker moves there | lab v8 |

## Logs: where to look

Every run saves the game's `Logs` folder to `siege/results/<label>-<time>-logs/` (zipped if over 50 MB). Rows start
with the game turn (3 digits) and the civ; the attacker is **Songhai**, the defender **Greece**. A battle's game
turns: `GameTurn` in `<label>.results.csv` is the battle's last turn; it started `turns` earlier.
Most useful: `PlayerTacticalAILog.csv` (assault), `OperationalAILog.csv` (operations; absent if none started),
`MilitaryAILog.csv` (targets), `PlayerHomelandAILog.csv` (where unused units went), `PlayerEconomyAILog.csv` (explorers),
`Lua.log` (the lab's own RUN/START/TURN/RESULT/SCRIPT/WARN/ERROR lines).

## The lab (ai-lab/siege)

- `SiegeLab.lua` (in-game add-in, `VERSION` at the top, now 16; battles grouped by scenario `era`, lowest first, repeats outer within a group: an era never drops, so later-era battles must come last): on turn 0 clears a flat arena; per battle, during the
  defender's turn, stages the army and city (techs set, research cleared, city tile revealed to the attacker, palace
  moved for `capital`, sea for `coast`, rough ring for `terrain = "rough"`, `homeDistance` moves the target), ends both
  sides' operations; logs a TURN line each turn and a RESULT line per battle; fixed +10 gold a turn for the attacker.
  Scripted attacker rules v3 when `-Script`.
- `SiegeLabScenarios.lua`: sets `siege` (15 scenarios, 12 turns, army next to the city), `march` (25 turns, army at
  home; march_* with ARMY_MEDIUM/LARGE, march3_* with 3 melee, lab v14) and `regress` (capital, coastal, medieval,
  artillery). Army constants at the top.
- `Install-SiegeLab.ps1` (install/remove into the pack; `-Set -Only -Repeats -Script -NewAI`), `Run-SiegeLab.ps1`
  (unattended run: install, start Civ V, click through menus by screen match, wait for DONE, close, save results,
  remove, restore the stock DLL; per-PC clicks in `hosts/<PC>.json`), `Set-LabDll.ps1` (swap in a release build,
  restore by SHA-256), `Get-SiegeLabResults.ps1` (Lua.log → `.results.csv` and `.turns.csv`, summary table),
  `Measure-SiegeLabClicks.ps1` (new-PC setup), `test/mock_run.lua` (offline check, `lua5.1 test/mock_run.lua`; out of date as of v15: fails in `rosterAlive`).
- Classes (BASELINE.md): CAP, LASTHP (≥85% damage, not taken), LOWDMG, NOATTACK, WALK. March: FirstArrivalTurn,
  ReadyTurn (80% within 3 tiles with melee and ranged).

## The run queue (ai-lab/runs)

- Request: commit `queue/<id>.json` (launches: label, release, set, only, repeats, script, newAI, host).
- `Watch-LabQueue.ps1` (v3, Task Scheduler every 5 min on each lab PC): pull, claim a launch
  (`launches/<id>/<label>.claim.json`), run `Run-SiegeLab.ps1`, write `launches/<id>/<label>.json`, done marker
  `done/<id>.json`, status `status/<PC>.json`; self-heals a failed pull; unpinned launches only on hosts in
  `trusted-hosts.txt`. Waits up to 90 min for a release to publish.
- `wait-for-results.sh`: how the implementing session waits (PLAN section 7, step 7).
- Builds: every push to `ai/**` runs `.github/workflows/build_vp.yml` (about 12 min) and publishes release
  `ab-<short commit>`.
