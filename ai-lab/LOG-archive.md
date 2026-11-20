# AI lab log: archive

Entries of closed milestones, moved from `LOG.md` unchanged, newest first (PLAN.md section 10). M1 and M0
(moved 2026-11-19 by implementer 6), M2 (moved 2026-11-20 by implementer 7). The summaries stay in `LOG.md`.

## 2026-11-19 Process: TREEDESKTOP set up (its setup session's report, 9:20 PM ET)
Steps 4-8 done: calibration `hosts/TREEDESKTOP.json` and `.screens.json` (window 1600x1410 at 480,15) pushed in
90a9c4b; both calibration launches ended "Run saved"; watcher registered (every 5 min while logged on); dry run takes
pc2tree4-6. Civ V starts slowly there (110-230 s to the legal screen), so the runner now reads per-PC timing from the
host file (`menuWaitSeconds` 420, `calibrateWaitSeconds` 270, `calibrateContinueClicks` 1; BOX keeps 240/60/2) and
keeps Civ V's window topmost during a run (the Claude app there is always on top). Commit bbb5a1c. pc2tree4 was
claimed before it. Pack: TREEDESKTOP's only game core DLL is `Mods\(1) Community Patch\CvGameCore_Expansion2.dll`,
SHA-256 262AEE88..., 16,837,632 bytes, dated 2026-11-02 8:24 PM; BOX's is 2FB6A9B9.... Same .Civ5Pkg priority and
key; 893 files, 123,750,824 bytes. Lab runs that name a release swap this DLL anyway, but other pack files may differ
too: both PCs were asked for a per-file hash list (`ai-lab/runs/pack/<PC>.txt`) to compare.
Two-PC check (`2026-11-19-16-pc2tree`, pc2tree4-6 on TREEDESKTOP, all clean, read 9:45 PM ET) against BOX's pc2box1-3
(same build `ab-78d5e8a`, stock AI, 6 battles per scenario each): captures differ by 0 or 1 in 14 of 15 scenarios
and by 2 in large_vs_strong (2/6 vs 4/6); totals 54 and 55 of 90 (BOX's m1s9: 57). RUN lines match (civs, map
56x36, King, Quick). The count bar is met. The pack DLL hash is not the same (262AEE88... vs 2FB6A9B9...); every lab
launch that names a release swaps that DLL, so TREEDESKTOP is trusted once BOX's per-file listing shows the rest of
the pack is identical. TREEDESKTOP's listing is in (`runs/pack/TREEDESKTOP.txt`, 893 files); BOX's is pending.
10:05 PM ET: the user said "approve everything", so TREEDESKTOP is added to `runs/trusted-hosts.txt` now, on the
count check alone; the pack comparison follows as a check, not a gate (if other pack files differ, its results are
set aside). Its first job: `2026-11-19-24-m2f3t`, the new-AI and stock half of m2f3 (iteration 2's real test), so it
doesn't wait about 2 hours behind BOX's queue. m2f3 on BOX still runs later; its battles add to these.

## 2026-11-19 M2 iteration 4: ranged units close in on the city too (rough-ground march)
Class: the one scenario that misses the proposed M2 bar: march3_vs_medium_rough, captured 6 of 6 but on turns 17-18
(limit 16.5; script median 11).
Cause (m2n6b rep 1, `PlayerTacticalAILog.csv`, game turns 42-47, implementer 6, 7:05 AM ET on 2026-11-20): the army is within 3
tiles on battle turn 7, but on the first turns at the city `ExecuteCaptureCityMoves` logs "too early for attacking
Knossos, required damage 250, expected max damage per turn 26" (on rough ground few units can strike this turn), and
the Composite Bowmen are "unassigned after tactical moves, left to homeland" 3-4 tiles out for several turns. Only 2 of
4 bowmen fire; damage starts on battle turn 10 and climbs about 28 a turn. M1 iteration 7's close-in (in
`PlotReinforcementMoves`) moves idle melee units up but skips ranged ones.
Change: the same close-in for ranged land units (new AI only): idle, not in an army, within 6 tiles, above a third of
their hit points, they move to (or hold at) their range of the city, at most 2 tiles. Melee unchanged. Log line
"ranged closing in on <city> (ring n)". `CvTacticalAI::PlotReinforcementMoves`.
Predicted (new AI, lab v17): march3_vs_medium_rough capture by turn 16 in at least 5 of 6 (from 17-18); damage starts
within a turn or two of arrival; no "left to homeland" lines for bowmen within 6 tiles of the city. Other march
scenarios: no capture count down by more than 1 in 6, turns the same or earlier. Siege set: no scenario down by more
than 1 in 6 (70 of 90 now); small_vs_strong and medium_vs_strong may lose more units (ranged units now stand in the
city's reach). Regress set: stays 24 of 24.
First results (`ab-153a72e`, 7:20-7:30 AM ET): m2n9a march3 8 of 8, **rough on turn 13** (from 17-18: the target
moved); m2n8a march 14 of 14 but march_medium_vs_medium_d8 on turns 22 and 12 (iteration 3: 10-11); m2g9a siege 23
of 30. Cause of the 8-tile slowdown (m2n8a rep 1, game turns 32-48): a spare bowman (not in the army) now "closes in"
alone 4 tiles ahead, is shot to 40 HP, the defenders come out, and the marching army logs "Contact with enemy!" and
"No progress to target" for 5 turns (`PlotArmyMovesCombat` holds an army in place while enemies are near it). 20
"Contact" lines in m2n8a against 0-2 in each iteration-3 launch.
**Revision 4b** (before deciding): ranged units close in only once one of our melee units is within 3 tiles of the
city, so they follow the army instead of leading it. Same prediction as above. The rest of m2f9 (on 4) runs on and
counts only as data; 4b gets its own runs.
Result 4b (m2fb, `ab-9238090`, lab v17, new AI, both PCs, all clean, read 8:05 AM ET on 2026-11-20): **march and march3
65 of 66; all 11 march scenarios meet the provisional M2 bar** (5 of 6 within 1.5x the script's median turn).
march3 rough turns 13-16 (bar 16.5; iteration 3: 17-18); march 8 tiles 10-11 (4: 12-22; 3: 10-11); d16 5/6 (15-16).
Siege 69 of 90 (iteration 3: 70): medium_vs_strong 2/6 (from 4/6; M1 build 2/6; not takeable), melee_only_vs_medium
1/6 (its first capture). Regress 24 of 24.
Decision: **accepted (4b).** medium_vs_strong's 2-in-6 drop is against iteration 3's high; it matches the M1 build
and the scenario is not takeable, so not counted as a regression. Taught: a unit that moves alone ahead of an army
stops the army (`PlotArmyMovesCombat` holds on contact); helpers of the army should follow it, not lead it.

## 2026-11-19 M2 iteration 3: spare ranged units fill an army's optional slots (user ruling: "go with what it has")
Class: never leaves home: every march battle of m2f2 (both AIs) and m2n4ta (march3, new AI): the operation starts and
stays in "recruiting".
Cause (m2n4ta, `OperationalAILog.csv`, implementer 6, 10:15 PM ET): two blocks in the recruiting stage.
(1) `FindBestFitReserveUnit` fills slots in order with the best-scored unit that fits, so the Swordsman, the strongest,
takes slot 0 (ATTACK/DEFENSE), and slot 2 (ATTACK/FAST_ATTACK) is left with only Spearmen, which can't take it
(greedy, not a matching; a stock bug, noted in CODEMAP, not fixed here). (2) Spare ranged units match no slot (the
classical formation's optional slots take melee only), so `HaveEnoughUnits` (free required <= optional present / 2)
never passes. Also, at 12-16 tiles some battles still log "no army path" with the range at 20 (rep 1 of march3 at 12,
16 and rough; rep 2 at 12 and rough found one): the cap is a cost budget, and rough plots cost 1.57, roads 0.54.
Change: the draft from earlier tonight. `OperationalAIHelpers::IsSpareRangedForOptionalSlot`: for a new-AI player, a
land ranged unit (RANGED or CITY_BOMBARD role) that fits no slot may take an optional slot whose primary role is a land
combat role (ATTACK, DEFENSE, COUNTER, FAST_ATTACK, RANGED, CITY_BOMBARD); used by `IsUnitSuitableForRecruitment`
(after normal matching fails) and `FindBestFitReserveUnit`. Required slots, civilian and naval slots unchanged; stock
unchanged. Two spare bowmen in the two optional slots then stand in for the missing melee unit by the stock rule.
Logging: the "no army path" line adds the path's uncapped normalized distance and plot count.
Predicted (new AI): in every march battle with an operation, "Transition to gathering stage" and then "Transition to
movement stage" within a few turns; at 8 and 10 tiles the army (melee included) gets within 3 tiles in at least 5 of 6
battles of each scenario, march set and march3 set alike; at 12 and 16 tiles the same wherever a path is found (the new
log line says how far over the cap the others are). Captures: not predicted (M2's bar measures them). Siege and regress
sets: no capture count down by more than 1 in 6 (their armies start within the operation's recruiting exclusion).
Stock: unchanged by construction (not rerun).
Result so far (m2n5a, TREEDESKTOP, `ab-8893a66`, lab v16, march set x2, clean, read 10:35 PM ET): **9 of 14 captured**,
the first march captures by either AI. 10 of 11 attack operations went recruiting -> gathering -> movement ->
finished. Rep 2: 6 of 7 (12 tiles weak 9, medium 12, large 10; 8 tiles 11; 10 tiles 10; rough 13; d16 arrived but
lost 3 and didn't take it). Rep 1: 8, 10 and rough captured (turns 10-13); 12 and 16 tiles never got an operation:
"no army path ... uncapped normalized distance -1, plots -1", i.e. **no path at all, not the cost cap** (my cost
theory was wrong). Army paths only cross tiles the player has seen; on a fresh map the corridor isn't seen yet, and the
lab only shows the target's tile. Lab v17: a march battle also shows the attacker a corridor 5 rows wide from home to
the target (a scouted route). The rest of m2f5 runs on v17 (m2n5a on v16). Ready turn = capture turn in most battles:
the army arrives in a stream, not together.
Result (all of m2f5, `ab-8893a66`, new AI, both PCs, all clean, read 11:15 PM ET):
- March set on v17 (m2n5b/c, 4 battles per scenario): **26 of 28 captured.** 12 tiles weak 4/4 (turn 9), medium 3/4
  (12-13), large 4/4 (11-13); 8 tiles 3/4 (11); 10 tiles 4/4 (10); 16 tiles 4/4 (16-23); rough 4/4 (13-15).
  Losses 0-0.5. (m2n5a on v16: 9 of 14, the misses all "no path" on unseen ground.)
- March3 set on v17 (m2n6b/c): **16 of 16.** 8 tiles turn 9, 12 tiles 11, 16 tiles 15-16, rough 17-18.
  (m2n6a on v16: 4 of 8.)
- Against the script (m2r0/m2r1/m2r5, median turns): march3 8 tiles 9 vs 7 (1.3x), 12 tiles 11 vs 9.5 (1.2x), 16
  tiles 16 vs 12 (1.3x), rough 17 vs 11 (1.55x); march medium 12.5 vs 9 (1.4x), large 12 vs 10 (1.2x).
- Siege set (m2g5a/b/c): 70 of 90 (iteration 2: 70; M1 build: 68). No scenario down; losses equal or lower.
- Regress set (m2x5a/b/c): 24 of 24.
- "Ready" (80% within 3 tiles) within 2 turns of the first arrival: new AI in 5 of 11 scenarios (all battles), else
  ready only on the capture turn: the ranged units come in early and the rest later. The script misses it too in
  march_large (2/6) and march_medium_vs_weak (3/6). So "ready" is a weak measure; captures and turns are the clear one.
Decision: **accepted** (iteration 3, and with it iteration 2, whose range the 12- and 16-tile captures need). The
targeted scenarios went from 0 of 6 (both AIs, every march scenario) to at least 4 of 4 or 3 of 4 on v17; nothing
else dropped. Taught: the march was blocked by four things in a row (range, formation slots, unseen ground in the lab,
and the lab's own wrong army), each hidden behind the last; the operational log showed each one.
m2f6 queued: one more new-AI launch per set (to 6 battles on v17) and the stock AI on v17 (stock baseline with the
corridor shown).
m2f6 (11:30 PM ET, both PCs, clean): new AI m2n5d 14 of 14, m2n6d 8 of 8; stock m2s7a/b/c and m2s8a/b/c **0 of 66**.
With 6 battles per scenario on v17 the new AI takes 64 of 66 (two misses: march_medium_vs_medium and the 8-tile
march, one each). Against the proposed bar it meets 10 of 11 scenarios; march3_vs_medium_rough is taken 6 of 6 but
on turns 17-18 against a limit of 16.5 (script median 11). Table in BASELINE.md "March (M2)".

## 2026-11-19 M2 iteration 2: the new AI looks twice as far for attack targets
Class: never leaves home: 36 of 36 (m2n0/m2s0); cause confirmed by m2n1a: "no army path ... plot distance 12 (16), max
normalized distance 10" every turn, no attack target, no operation.
Cause: `CvMilitaryAI::GetArmyPathsFromCity` limits army paths to the land trade route range (10).
Change: for a new-AI player, land range x2 (20). Distance still lowers the target's score (ScoreAttackTarget, lowest
weight at path length 24). Commit `dce08ed`, build `ab-455fe7a`. Also a new scenario, march_medium_vs_medium_rough
(12 tiles, rough ground near the city).
Predicted: new AI at 12 and 16 tiles: "attack target" and an OP_CITY_ATTACK_LAND from the first turns, army leaves home
in at least 5 of 6 battles; arrival and capture are what M2's bar then measures (not fixed yet). Stock unchanged (still
stays home at 12+). Siege set and regress set with the new AI must not move (siege: 68 of 90 on m1f7, no scenario down
by more than 1 in 6). Also expected and not counted: more "exposed city" defensive targets, since the same paths feed
the defence list.
Run (overnight, BOX): 2026-11-19-17-m2f2 (march x6 new AI and stock), -18-m2g2 (siege x6), -19-m2x2 (regress x6),
-20-m2r3 (script on the rough march x6).
Prediction amended before the result (implementer 6, 9:30 PM ET, from the m2d1 8-tile logs, `OperationalAILog.csv`
m2n1c rep 1): at 8 tiles the operation starts but **never leaves the recruiting stage** in 25 turns. In the classical
era a land city attack uses `MUFORMATION_SMALL_CITY_ATTACK_FORCE` (`GetCurrentBestFormationTypeForLandAttack`, by
game era): 3 required melee slots, 2 required ranged slots, optional slots for melee only. ARMY_MEDIUM (2 Spearmen,
5 Composite Bowmen) fills 4 slots; slot 2 (a third melee) stays open and `HaveEnoughUnits(..., 0)` never passes; the
3 other bowmen fit no slot and are left to the tactical AI (they are the ones that walked in and shot the city). The
operation was started willing to build the missing unit (`RequestCityAttack`'s iNumUnitsWillingToBuild), but the
lab's attacker never builds units. So with iteration 2 the 2-Spearman armies (march_medium_vs_*, _rough, _d16) should
get a target and an operation and then **stay in recruiting**, as at 8 tiles; only march_large_vs_medium (3 Spearmen)
should fill the required slots and march. "Leaves home in 5 of 6" is predicted for march_large_vs_medium only.
Lab v14 (scripts only, user ruling of 9:10 PM): four march3 scenarios added to the march set (3 Spearmen + 4
Composite Bowmen, which fills the formation's required slots): march3_vs_medium_d8, march3_vs_medium (12 tiles),
march3_vs_medium_d16, march3_vs_medium_rough. m2f2 runs the whole march set, so it plays them too, new AI and stock.
Predicted for them: new AI leaves home in at least 5 of 6 at 8, 12 and 16 tiles and on the rough ground; stock only
at 8 tiles. Script reference: `2026-11-19-21-m2r4`.
**Correction (implementer 6, 9:40 PM ET, from m2n3a, the first iteration-2 launch):** iteration 2 works as far as it
goes: with the range at 20 the new AI names the 12- and 16-tile cities as "preferred attack target" and starts an
OP_CITY_ATTACK_LAND in every battle. But m2n3a took 0 of 22, and **march_large_vs_medium and all four v14 march3
scenarios also stayed in "recruiting"**: "Found 7 potential units", then only 2 Spearmen (slots 0-1) and 2 bowmen
(slots 3-4) added; slot 2 open; the third Spearman goes to a sentry point ("UNIT DIVERSITY - WE NEED attack"). Slot 2
of MUFORMATION_SMALL_CITY_ATTACK_FORCE is ATTACK/FAST_ATTACK, and `FindBestFitReserveUnit` matches a slot only by the
unit type's AI roles: a Spearman has DEFENSE (slots 0-1 are ATTACK/DEFENSE) but not ATTACK, so no number of Spearmen
fills slot 2. My v14 march3 army (3 Spearmen) was built on a wrong assumption. Lab v16: the march3 scenarios move to
their own set `march3`, with 2 Spearmen, 1 Swordsman and 4 bowmen (Iron Working), so the rest of m2f2 runs the
original 7 march scenarios as asked; the v14 march3 battles in m2n3a are void (a lab mistake). An empty battle list
now ends the launch at once (DONE after "no scenarios to run"); m2r4, queued for the v14 names, will be one. New
request `2026-11-19-23-m2f3`: march3 set on `ab-455fe7a`, new AI, stock and script, 6 battles each.
Prediction for m2f3: new AI leaves home ("Transition to gathering stage", then movement) in at least 5 of 6 at 12 and
16 tiles and on rough ground; at 8 tiles both AIs do; stock stays home at 12 and 16. And for the rest of m2f2, every
2-Spearman and 3-Spearman army stays in recruiting for both AIs.
Result m2f2 (`ab-455fe7a`, lab v14-v15, m2n3a/b/c new AI, m2s3a/b/c stock, 6 battles per march scenario each, clean,
read 9:55 PM ET): **0 captures for both AIs.** At 12 and 16 tiles and on rough ground no unit came within 3 tiles in
any battle (mean distance stays 11-16): the new AI's operations start (iteration 2 works) but stay in recruiting,
like stock's at 8 and 10. 8 tiles: both AIs shoot the city (new AI to 229 in 3 of 6; stock in 2 of 6), never ready.
10 tiles: one battle each got a unit within 3 tiles. The v14 march3 battles (m2n3a/m2s3a, 3 Spearmen) behave the same
and are void. As the corrected prediction said.
Decision: **pending.** The change does what it claims, but no army can leave recruiting in this set, so it can't move
a capture here. Its real test is m2f3 (march3 with a Swordsman, armies that fill the formation, queued on the same
build): accept if the new AI's army leaves home at 12 and 16 tiles in at least 5 of 6 and stock's doesn't; else revert.
Iteration 3 (spare ranged units fill optional slots) waits for that decision. m2g2/m2x2 (siege and regress sets on
this build) are the no-regression check.
**Decision (10:15 PM ET, overnight, user asleep and "approve everything"): accepted provisionally on its logged
effect.** m2n4ta showed that no army can leave recruiting until the slot problem is fixed, so m2f3 can't test it by
captures either. What it claims is in the logs: new-AI "preferred attack target" at 12 and 16 tiles and an operation
started, where stock logs "no army path" (m2f2). No regression: m2g2 70 of 90 (M1: 68). It stays on the branch under
iteration 3 and is judged by captures with it; if iteration 3's runs show the range change does nothing useful or
harms, both are reverted. For the user to confirm in the morning.
m2f3t (TREEDESKTOP, `ab-455fe7a`, march3 set, m2n4ta/b/c new AI, m2s4ta/b/c stock, 6 each, clean, read 10:30 PM ET):
0 captures; 0 "Transition" lines. 8 tiles: the bowmen outside the army reach the city (turns 7-17) and do little (new AI
2-148, stock 0-92). 12, 16, rough: no unit within 3 tiles for either AI. As found in m2n4ta: slot 2 stays open (the
Swordsman takes slot 0). m2f3 on BOX (same build, plus the script half) adds nothing new for the AIs; its script
battles give the march3 reference.
m2g2 (siege set, `ab-455fe7a`, new AI, m2g3a/b/c, clean): **70 of 90** (m1f7 on the M1 build: 68). No scenario's
captures dropped. medium_vs_strong 4/6 (from 2/6), the rest equal, capture turns the same. Losses equal or lower
except small_vs_strong, 2.5 from 1.7: not takeable, and losses don't count (ruling 2026-11-18). The M1 bar holds.
m2x2 (regress set, `ab-455fe7a`, new AI, lab v16, m2x3a/b/c, clean): 24 of 24. capital 6/6 (turns 4-5), coastal 6/6
(4-5), medieval 6/6 (3-4), artillery 6/6 (6-12), 0.2 units lost on artillery. Lab v15's era grouping works: no
battle void.
Result: (pending)

## 2026-11-19 Process: why BOX stalled 5:09-8:19 PM, and the pack check (BOX runner's report, 8:20 PM ET)
Cause (BOX runner): BOX's lab clone fetched only `claude/bold-feynman-cgnhjs`, so `origin/ai/v2` went stale; the
watcher's unpushed-commit count saw phantom commits and dropped every claim, retrying every 22 s. The runner added the
ai/v2 fetch refspec to the clone; the watcher now fetches with an explicit refspec as well (so a fresh clone can't
repeat it). Queue moving again from 8:19 PM (m2s1a, m2n1b, m2s1b...). v3 parses cleanly on BOX.
Pack: BOX's pack DLL SHA-256 is `2FB6A9B9...` (the stock DLL the launches report); TREEDESKTOP's setup session reported
`262AEE88...`. **The two packs differ**, so the two-PC check can't pass until that is explained. The pack loads whole
folders (`<GameplaySkin><Directory>Mods</Directory><Directory>UI</Directory>`, no `<Gameplay>` section);
Install-NewAI.ps1 now puts the checkbox files in `Mods\AILab NewAI\` for such a pack and leaves the .Civ5Pkg alone.

## 2026-11-19 Process: less user input (user request, 7:30 PM ET)
BOX stalled twice today (2:21-5:05 PM, and from 5:09 PM) with nothing in the repository saying why; each time the user
had to go to the PC. Watcher v3 (scripts only): a per-PC status file pushed on every change of state (paused, Civ V
open, pull or push failing, error, idle), and a self-heal for a failed pull when there are no unpushed commits
(reset the lab checkout to the branch, pull again). This session may now message the PC runner sessions on the user's
PCs (the user allowed it on 2026-11-19, about 8:20 PM ET); the BOX runner was asked to diagnose the 5:09 PM stall.

## 2026-11-19 The new-AI switch at game setup (user addition, PLAN.md 8a item 3)
The DLL already reads the game option `GAMEOPTION_AI_V2_ALL` (every AI major civ on the new AI); nothing in setup
showed it. Change: `ai-lab/newai/` adds a GameOptions row and its text ("New AI (test build)") to the pack through
two lines in its .Civ5Pkg (`Install-NewAI.ps1 -Release ab-<commit>`; `-Remove` undoes it, puts the stock DLL back
and clears Civ V's database cache). Game options are set in setup and shared in multiplayer, so the switch is the
same on every PC. DLL: `CvPlayer::gameStartInit` reads the switch, so it holds from turn 0 (before, from the
player's first turn), and `AILab_NewAI.csv` logs each change of a player's switch. Commit `0b401eb`.
Also: `MP-SMOKE-TEST.md`, the 30-turn LAN test for the user (desyncs, assertion dialogs, crashes, turn times).
Not yet tried on a PC: the .Civ5Pkg layout of the user's pack is not in this repo; the script stops without
changing it if it has no `</Gameplay>` section. Check first in single player (the box shows; the CSV lists the AIs).
Result: (pending: build, then the user's single-player check)

## 2026-11-19 Regression set: baseline (measurement only; user addition, PLAN.md 8a)
Lab v12 (scripts only): a scenario's city can be the defender's capital (`capital`: the palace moves from the reserve
city for the battle; the game hands it back when the target falls or is wiped, and the lab checks that) or on the
coast (`coast`: sea on its north side out to 4 tiles). New set `regress` in SiegeLabScenarios.lua:
regress_capital and regress_coastal (both medium_vs_medium with that one change), regress_medieval (2 trebuchets,
4 crossbows, 3 longswords vs pop 8, walls and castle, crossbow inside, 2 pikes outside; 15 turns) and
regress_artillery (3 artillery, 4 riflemen vs pop 12, walls, castle and arsenal, rifleman inside, 2 outside;
15 turns). Units lost is the summary's "Attackers lost" column and goes into BASELINE.md for this set.
Queued: `2026-11-19-14-reg0`, `ab-78d5e8a`, newAI attacker and stock, 3 launches x 2 repeats each, on BOX.
Result (rgn0a/b/c new AI, rgs0a/b/c stock, clean, read 9:15 PM ET; table in BASELINE.md): regress_medieval 6/6 both
(turns 3-4); regress_artillery new AI 6/6 (turns 6-8), stock 1/6; capital and coastal 3 of 3 on rep 1 for both, turn
4-7, no losses. **Lab leak:** every rep-2 capital and coastal battle is void: it ran after rep 1's artillery battle,
the attacker's era stayed industrial (an era never drops), and `DisbandLongObsoleteUnits` scrapped its Spearmen and
bowmen one a turn ("Disbanding long obsolete unit", PlayerHomelandAILog.csv, 17 lines in rgn0a; all 7 gone by turn 7,
0 killed by the enemy). Fix: lab v15 groups battles by a scenario `era` field, lowest first, repeats outer within a
group (regress_medieval era 2, regress_artillery era 4). Rerun of the two: `2026-11-19-22-reg1` (ab-78d5e8a, new AI and
stock, 6 each). m2x2 (regress set, queued) runs on v15. Also found: `test/mock_run.lua` fails at SiegeLab.lua's
`rosterAlive` (pairs on nil) before and after this change; the mock is out of date, not fixed yet.
Decision: the artillery and medieval rows stand; capital and coastal wait for reg1.
reg1 (10:40 PM ET): capital 6/6 (turn 5) and coastal 6/6 (turns 4-5), both AIs, no losses. Baseline complete in
BASELINE.md. Capital and coastal are no harder than medium_vs_medium for either AI.

## 2026-11-19 Process: two-PC check, first try (BOX ran both halves)
The request `2026-11-19-13-pc2check` ran on BOX between 2:00 and 2:21 PM ET under watcher v1, which predates the
`host` field, so BOX also ran the three launches pinned to TREEDESKTOP (`ab-9109672` was not out yet, so v1 skipped
m2d1 and took this one). Not wasted: it measures BOX against itself, the noise the two-PC bar must allow.
Captures of 6 per scenario, launches 1-3 ("box") vs launches 4-6 ("tree"), all on BOX, stock DLL `ab-78d5e8a`:
13 of 15 scenarios differ by 0 or 1; small_vs_medium (6 vs 4) and siege_train_vs_strong (2 vs 4) differ by 2.
Totals 55 vs 57 of 90 (m1s9 on the same build: 57). So the bar in runs/README.md (at most 1 apart in 13 of 15, at
most 2 in all) is the same-PC noise exactly; kept as is.
Next: a TREEDESKTOP-only request (3 launches) once the user has set that PC up and BOX runs watcher v2, compared
against these 6 BOX launches.

## 2026-11-19 M2 reference: the script on the march3 set (measurement only)
m2r5a/b/c (BOX, `ab-455fe7a`, lab v16, scripted attacker, march3 set, 6 battles each, clean, read 11:10 PM ET): 24 of 24
captured, no losses. Capture turns (median): 8 tiles 7, 12 tiles 9.5, 16 tiles 12, rough 11. (m2f3's AI half on the
iteration-2 build: 0 captures, as m2f3t.)

## 2026-11-19 M2 reference: the script on the rough march (measurement only)
m2r3a/b/c (`ab-455fe7a`, scripted attacker, march_medium_vs_medium_rough, 6 battles, clean, read 10:20 PM ET):
3 of 6 captured (turns 13, 25, 25), 0-3 units lost. Rep 1 of each launch: first unit within 3 tiles on turn 9,
ready on 18; rep 2: turn 4 and 6 (the same pattern in all three launches, as march_medium_vs_weak rep 1 in m2r0).
So on rough ground the script takes it only half the time within 25 turns: by the takeable rule (2 of 6) it is
takeable, reference turns 25 (the slower half).

## 2026-11-19 M2 iteration 0: the march baseline (measurement only)
Class: none yet. M2's question: does an army that starts at home (12 tiles away) arrive ready and then take the city?
Change: lab v10 (Lua only, no build): each TURN line adds AttackersWithin3, MeleeWithin3, RangedWithin3; each RESULT
adds FirstArrivalTurn (first turn any unit is within 3 tiles of the city) and ReadyTurn (first turn at least 80% of
the starting army is within 3 tiles with melee and ranged among them, or the capture turn). The bar's "within 2 turns
of each other" is read as ReadyTurn - FirstArrivalTurn <= 2.
Run: queue request 2026-11-19-11-m2base: the march set (march_medium_vs_weak, march_medium_vs_medium,
march_large_vs_medium; 25 turns) on `ab-78d5e8a`, newAI attacker (m2n0a/b/c), stock (m2s0a/b/c) and the scripted
attacker (m2r0a/b/c), 6 battles each.
Result: m2n0a/b/c, m2s0a/b/c, m2r0a/b/c (`ab-78d5e8a`, lab v10, clean: no assertions, no lab errors).
- New AI and stock AI: **0 of 18 battles each** reached the city; no unit ever came within 3 tiles; no damage. The
  army stayed at home (mean distance 11-13 all 25 turns).
- Scripted attacker: **18 of 18 captured**, first unit within 3 tiles on turn 4, ready on turn 6-7, capture on turn
  7-11. (march_medium_vs_weak rep 1 was slower in all three launches: arrival turn 9, ready 15, capture 17, with
  the same pattern each time; not read yet.)
- The attacker's logs: no `OperationalAILog.csv` was written at all and `MilitaryAILog.csv` has no "attack target"
  line, so no attack operation was ever started. `PlayerHomelandAILog.csv`: the units are moved to "garrison" and
  "sentry point" at home every turn. (The same is true in the siege lab: there the tactical AI fights because the
  army starts next to the city, without any operation.)
Decision: baseline recorded. One failure class: "never leaves home", 36 of 36 for both AIs. Iteration 1 looks for the
cause.

## 2026-11-19 M2 iteration 1: why the army never leaves home (logging only)
Class: never leaves home: 18 of 18 (new AI) and 18 of 18 (stock), march set, m2n0/m2s0.
Cause (suspected, not yet in a log): `CvMilitaryAI::UpdateAttackTargets` only keeps enemy cities that
`GetArmyPathsFromCity` reaches within `iMaxNormalizedDistance` = the land trade route range, which is 10 on every map
size in the Community Patch (`TRADE_ROUTE_BASE_LAND_DISTANCE` 10, no `TradeRouteDistanceMod` without Vox Populi) and
the attacker has no range bonus. The march target is 12 tiles away, so it is never an attack target, and
`DoCityAttacks` has nothing to request.
Change: logging only (commit 9109672): `GetArmyPathsFromCity` logs "no army path to: <city>, from muster ..., plot
distance, max normalized distance" for each enemy city at war it can't reach; `UpdateAttackTargets` logs "land target
rejected: ..., approach score, path length" when a reachable city fails the approach or path check. Lab v11: march
scenarios get `homeDistance`; added march_medium_vs_medium_d8, _d10 and _d16 (the target city moves; home stays).
Predicted: d12 and d16 log "no army path" every turn and stay at home, as before. d8 (and probably d10: 10 flat steps
is at the limit) log "attack target" and an operation starts (OperationalAILog.csv appears); whether it then arrives
and takes the city is what the M2 bar measures. If d8 also never gets an attack target, the range is not the cause.
Build: `ab-9109672`. Run: queue request 2026-11-19-12-m2d1 (m2n1a/b/c new AI, m2s1a/b/c stock, all six march
scenarios x 2 each; m2r1 the script on the three new distances x 2).
Result so far (m2n1a, BOX 5:05-5:09 PM ET, new AI, 12 battles; the other six launches still running):
- 12 and 16 tiles: "no army path to: <city>, plot distance 12 (16), max normalized distance 10" every turn; no attack
  target, no operation; the army stays home (0 damage). As predicted.
- 8 tiles: "preferred attack target" from the first turn, OP_CITY_ATTACK_LAND recruits the army, it marches (mean
  distance 12 -> 5.4) and shoots the city to 92% in both battles, no capture in 25 turns.
- 10 tiles: a target and an operation too, but the army barely moves (mean distance 9.6). **Lab leak:** the 8-tile
  battle's operation (target plot 17:14) was never ended; in the 10-tile battle (city at 19:14) it recruited the new
  army for a city that was no longer there, until it timed out on its 43rd turn. The 10-tile result is void.
  Fix: lab v13 (commit `1ca9bea`): `Player:LabEndOperations()` (lab-only Lua method, ends all of a player's AI
  operations) called for both sides at each battle's setup. The remaining m2d1 launches run with v13's Lua on the
  old DLL (no such method: a WARN per battle, behaviour as v12), so their 10-tile battles are void too.
  Recheck: `2026-11-19-15-m2d2` (8 and 10 tiles, new AI and stock, 6 each, `ab-1ca9bea`, BOX).
- Earlier results: in the siege set each battle starts next to the city, so leftover operations could also have
  recruited units there; M1's results are not reread for this, but M2 and later results use v13.
Recheck m2d2 (`ab-1ca9bea`, lab v13, m2n2a/b/c new AI, m2s2a/b/c stock, 6 battles per distance each, clean, read 9:30
PM ET): 0 captures. 8 tiles: both AIs damage the city in every battle (new AI most damage 0-229, stock 5-229; three
battles at 229 of 250), first unit within 3 tiles on turns 5-15, never "ready" (mean distance 5.4-6.6). 10 tiles: an
operation now starts in every battle (v13 ends the last battle's operation; one new operation per battle in the log),
but no damage and the army stays home (mean distance 9.5-9.7). Every operation stays in "recruiting" ("Initial stage
is recruiting", 3-4 units, never "Transition to gathering stage"): the 3-melee formation, as found above. At 10 tiles
not even the unrecruited bowmen go in. The 10-tile battles of m2d1 stay void; these replace them.
Result (all of m2d1, read by implementer 6, 9:05 PM ET): m2n1a/b/c (new AI), m2s1a/b/c (stock), m2r1 (script), all
clean, `ab-9109672` (the old DLL: 10-tile battles void in every launch).
- 12 and 16 tiles: 0 of 12 per AI, no damage, army at home all 25 turns. As predicted.
- 8 tiles: **both AIs** march (stock too: the 8-tile city is inside the range of 10 for everyone). 0 of 6 captured per
  AI. The army stops at a mean distance of 5.4-5.7 tiles and never reaches "ready" (80% within 3 tiles); only the
  ranged units come in to shoot. New AI: the city shot to 229 of 250 (its last hit points) in 4 of 6, 41 and 192 in
  the others; stock: 229 in 2 of 6, 5-84 in the rest. First unit within 3 tiles on turn 5 or 8-11. No losses.
- Script: 6 of 6 (d8 turn 11, d10 turn 8, d16 turn 17).
Decision: cause confirmed (range); iteration 2 widens it. A second class shows behind it, for M2's next iteration:
"arrives, melee stay back", the army stalls about 5 tiles out and the city sits on its last hit points with all
melee alive (M1 iteration 7's "close in" only reaches melee within 6 tiles of the city).

## 2026-11-19 M1 iteration 9: the new AI as defender (measurement only)
Class: none (the M1 bar's defender part: new AI as defender must not make stock attackers more successful than the
stock defender does).
Change: none. Build `ab-78d5e8a` (iterations 3 and 7). The new code only changes what a player does near an enemy
city it is at war with, so as defender it should change little.
Predicted: stock attacker vs new-AI defender (m1v9a/b/c) captures no more than stock vs stock on the same build and
lab (m1s9a/b/c), within 1 in 6 per scenario.
Run: queue request 2026-11-19-10-m1v9.
Result: m1v9a/b/c (stock attacker vs new-AI defender) and m1s9a/b/c (stock vs stock), `ab-78d5e8a`, 6 battles per
scenario each, interleaved launches, all clean. Captures: 55 of 90 against the new-AI defender, 57 of 90 against the
stock defender (m0d-f on lab v6: 50). Per scenario, stock attackers did better against the new AI only in
large_vs_strong (3/6 vs 2/6, within 1 in 6); worse in siege_train_vs_strong (4/6 vs 6/6) and small_vs_medium (5/6 vs
6/6); the rest equal. Same-build stock-vs-stock also confirms the stock baseline's shape (large_vs_strong 2/6,
medium_vs_medium_rough 0/6, melee_only 0/6).
Decision: the defender part of the M1 bar is met.


## 2026-11-19 M1 iteration 8: a melee army that can wear the city down attacks it (third try on melee_only)
Class: "never attacks" in a takeable scenario: melee_only_vs_medium NOATTACK 6/6 (m1f7; script 6/6 by turn 5, 0.7 lost).
The last failing takeable scenario.
Cause (m1d7, `PlayerTacticalAILog.csv`, and iteration 6's log): two blocks in `ScoreAttackDamage`. (1) Every melee
attack on the city is "refused (too early)": one Swordsman alone deals 15 and takes 32, so it would be worn down
(2.4 turns) before the city (6.8). (2) When iteration 6 allowed it, the attack still scored below doing nothing: the
score is damage dealt minus damage taken (15 - 32), so the tactsim chose positions with no attack. Iteration 6's third
block (a wounded Swordsman leaving to heal) is gone since iteration 7: it now holds 2 tiles out ("melee closing in ...
holds" at 71 HP in m1d7).
Change: with `IsNewAI()`, iteration 6's army test returns (`MeleeArmyCanWearDownCity`: own melee within 2 tiles,
their hit points less 10 each, the city's strike shared, healing counted); when it is what allows the attack, the
attack's score counts the damage it deals and not the damage it takes ("allowed (army)" in the log). Other checks
unchanged (hit point limit, damage worth it, suicide).
Predicted (newAI attacker): melee_only_vs_medium from 0/6 to at least 4/6 (decision rule), by turn 7, losses
allowed. ref_cb4_melee6_vs_walls (6 melee) and the other scenarios: no capture count drops by more than 1 in 6; some
may capture a turn earlier. small_vs_strong/medium_vs_strong may lose more units. If melee_only doesn't move, this is
the third iteration on it that moved nothing: stop and write to the user (rule 9).
Build: `ab-481fb7c`. Run: queue request 2026-11-19-09-m1f8 (m1d8: melee_only and ref_cb4 x 2; then m1f8a/b/c).
Result: m1d8 + m1f8a/b/c (`ab-481fb7c`, newAI attacker, clean). melee_only_vs_medium 0/2 and 0/6, but it now
attacks: LOWDMG 6 (37-191 damage; m1d8 121 and 128) instead of NOATTACK 6 (0 damage), 0.2 units lost. The log has
"allowed (army)" lines and attacks by 3-4 Swordsmen, but only on some turns (m1f8a rep 1: battle turns 2, 4-6, 8,
10); on the others the city heals 20 back. Full set 64 of 90 [recount: 67] (m1f7: 65 [recount: 68]): medium_vs_strong 1/6 (from 2/6),
large_vs_strong one capture on turn 11 (bar 12), ref_cb4_melee6_vs_walls turns 4-5 (from 5). Prediction missed.
The fourth block (m1f8a rep 1, game turns 55-66): on the turns without attacks `ExecuteCaptureCityMoves` never starts
the attack: "too early for attacking Miletos, required damage 208, expected max damage per turn 16" (also 29, and 0
on the last two turns with five Swordsmen adjacent). Its estimate (`ComputeTotalExpectedDamage`) counts at most 3
melee units, drops wounded ones, and needs (damage - heal) x 13 turns to cover the city. So two functions judge "can
this army take this city" differently, and the stricter one wins every other turn.
Decision: reverted (code only; this entry stays). Rule 9: third iteration on melee_only_vs_medium without a capture,
so stop and write to the user. Proposed next step: the plan's "one function" item, one army estimate used by both
`ExecuteCaptureCityMoves` and `ScoreAttackDamage`.

## 2026-11-19 M1 iteration 7: melee units close in on the city instead of going home
Class: "shot to its last hit points, not taken, melee alive" in a takeable scenario: medium_vs_medium_rough LASTHP 5/6,
CAP 1/6 (m1f3; the script takes it 6/6, turns 7-12).
Cause (m1d3a rep 1, `PlayerTacticalAILog.csv`, game turns 19-30): on battle turn 1 both Spearmen stand 3 tiles out,
"available 1, in reach 0" (rough ground: they can't get adjacent and still attack this turn), so the capture attempt
runs with "0 melee" every turn. `PlotReinforcementMoves` doesn't move them either (it returns early when the zone is
friendly-dominated and has any melee strength; otherwise its tactsim may leave units within 4 tiles unmoved), so
they are "left to homeland" and walk off (distance 5 on turn 2, home city by turn 6). The city sits at 21 HP from
battle turn 6 to 12 with both Spearmen alive and far away.
Change: in `PlotReinforcementMoves` (CvTacticalAI.cpp), with `IsNewAI()`, for a land zone whose city belongs to a
player we are at war with: every own land melee unit still usable this turn, not in an army, within 6 tiles and above
a third of its hit points, moves to 2 tiles from the city (or holds there) while the city is above half health, and
next to it (or holds there) once it is at half health or less. Logged as "melee closing in on <city> (ring n)". Runs
in the reinforcement pass, after the turn's attacks, so units that attacked are not affected.
Predicted (newAI attacker): medium_vs_medium_rough from 1/6 to at least 5/6, capture around battle turn 6-8 (the city
passes half health on turn 5; Spearmen step adjacent then and attack the next turn); no "left to homeland" lines for
its Spearmen. melee_only_vs_medium stays 0/6 (Swordsmen wait 2 tiles out instead of standing adjacent; the city is
never damaged). Full set: no takeable scenario's captures drop by more than 1 in 6, capture turns within the bar.
Build: `ab-78d5e8a`. Run: queue request 2026-11-19-08-m1f7 (m1d7: the two scenarios x 2; then m1f7a/b/c: full set x 3).
Result: m1d7 + m1f7a/b/c (`ab-78d5e8a`, newAI attacker, clean: no assertions, no lab errors). medium_vs_medium_rough
2/2 and 6/6 captured, all on battle turn 5 (m1f3: 1/6; script 7-12). Log: Spearmen at distance 3 "move" to ring 2 on
turn 1 and "hold" there, then step in. Full set 65 of 90 [recount: 68] (m1f3: 62). melee_only_vs_medium 0/6 (0 damage), as
predicted. No capture count dropped: medium_vs_strong 2/6 (from 1/6), large_vs_medium turn 3 in all 6 (from 3-4);
large_vs_strong turns 6-8 (from 6-7, bar 12, 0.5 units lost), siege_train_vs_strong 6-8 (from 6-9). Losses rose only
in small_vs_strong (1.7 from 0.7, not takeable, 0-7 damage): the melee now wait 2 tiles out of a strong city instead
of going home; by the user's ruling of 2026-11-18 losses don't count, but it's a sign the "close in" doesn't ask
whether the city can be taken (M1's "one estimate" item).
Decision: accepted. 13 of 14 takeable scenarios now meet the M1 bar; only melee_only_vs_medium is left.

## 2026-11-19 M1 iteration 4: melee units count each other when attacking a city
Class: "never attacks" in a takeable scenario: melee_only_vs_medium NOATTACK 6/6 (m1f3; the script takes it 6/6 by
turn 5). Tied in size with medium_vs_medium_rough (LASTHP 5/6, melee left behind: iteration 5).
Cause (m1f3a rep 1, `PlayerTacticalAILog.csv`): from battle turn 2 four to six full-health Swordsmen stand next to the
city, "capture attempt ... executed: 6 units in reach, 6 melee", and the city takes 0 damage in 12 turns. So the
capture move runs and the combat simulation assigns no attack. In `ScoreAttackDamage` (CvTacticalAI.cpp) a melee attack
on a city is impossible when the attacker would be worn down before the city ("fRemainingTurnsOnAttacker <
fRemainingTurnsOnCity"), unless it is a "good first attack" (damage received under 23). Turns on the city are counted
from this one unit's damage or the city's damage taken last turn; with no ranged units the city never took damage, so
each Swordsman alone faces 250 HP and the city's full strike, and none starts. The stock comment says it: "ideally
ranged units start attacking and melee joins in later".
Change: with `IsNewAI()`, the turns-on-the-city estimate multiplies the unit's damage by the number of friendly combat
units next to the city in the simulated position, and the city's counterattack is divided among them (a city strikes
one unit a turn). No other change to the scoring; the later damage-versus-danger check still applies.
Predicted (newAI attacker, full set x 3): melee_only_vs_medium from 0/6 to at least 5/6, by turn 7, with losses (the
user's ruling: losses don't count). ref_cb4_melee6_vs_walls and the medium/large scenarios captured as before or
sooner. No scenario's captures drop by more than 1 in 6. small_vs_strong and medium_vs_strong may engage more and lose
more units without capturing; that is acceptable only if it doesn't cost captures elsewhere.
Build: `ab-e6beb62`. Run: queue request 2026-11-19-05-m1f4 (m1f4a/b/c).
Result: m1f4a/b/c (newAI attacker, full set x 3, no assertions, no lab errors): 62 of 90 captured (m1f3: 62).
melee_only_vs_medium 0/6, now LOWDMG 6 (48-91 damage, the first hits only on battle turns 10-12) instead of NOATTACK 6.
Everything else within 1 in 6 of m1f3 (medium_vs_strong 2/6 from 1/6, medium_vs_medium_rough 0/6 from 1/6), losses
not higher. Prediction missed.
Decision: reverted (the code; this entry stays). Taught: the "too early" check was not the only thing stopping the
attack, or the adjacent-unit count in the simulated position is not what I assumed. The log can't say which.

## 2026-11-19 M1 iteration 5: why melee units next to a city don't attack it (logging only)
Class: as iteration 4: melee_only_vs_medium never or barely attacks (0/6).
Change: one log line in `ScoreAttackDamage` (CvTacticalAI.cpp) each time the combat simulation scores a melee attack
on a city, once per unit, plot and outcome per turn: "melee on city <city>: <unit> from x:y hp, <outcome>: <values>",
outcome "refused (too early)" (with the turns-on-city/attacker figures and the adjacent count), "refused (damage not
worth it)" or "refused (suicide)" (with danger and scaled damage), or "allowed". No behaviour change; iteration 4's
code is reverted in the same commit.
Predicted: in melee_only_vs_medium the Swordsmen next to the city show "refused (too early)" on the early turns, or
"allowed" with no attack following (then the simulation's choice of position is the cause, not the scoring).
Result: m1d5 (`ab-c57e923`, newAI attacker, melee_only_vs_medium and medium_vs_medium_rough x 2, clean): 107 "melee on
city" lines for the attacker, all "refused (too early)", none "allowed". Typical (battle turn 2, 4 adjacent): "city hp
250, damage 15, received 32, city strike 16, last turn 0, turns on city 6.8, on attacker 2.4, agg 3 bias 1.73". So the
first guess was right about the check and wrong about the numbers: each Swordsman takes twice what it deals, so
counting neighbours (iteration 4) only just tipped full-health units over and refused every wounded one.
Decision: accepted (logging stays).

## 2026-11-19 M1 iteration 6: judge the melee army around a city together
Class: melee_only_vs_medium never attacks, 0/6 (script: 6/6 by turn 5, 0.7 units lost).
Cause: iteration 5's log: `ScoreAttackDamage` refuses every melee attack on the city as "too early" because one unit
alone would be worn down (2.4 turns) long before the city (6.8). Six Swordsmen together deal about 90 a turn against a
heal of 20, and have 540 hit points to spend at about 35 per attack: enough to take it, as the script shows.
Change: with `IsNewAI()`, when the stock per-unit test says "too early", a second test may allow the attack:
`MeleeArmyCanWearDownCity` counts own land melee units within 2 tiles of the city and their hit points (less 10 each),
assumes each deals and takes what this unit does per attack, shares the city's strike among them, and allows the
attack if the total damage they can deal before being worn down (times the stock aggression bias) covers the city's
hit points plus its healing over the turns it would take. The other checks (hit point limit, damage worth it, suicide)
are unchanged. melee_only_vs_medium: 540 / 35.7 x 15 x 1.58 = 358 against 250 + 72: allowed.
Predicted (newAI attacker, full set x 3): melee_only_vs_medium from 0/6 to at least 5/6 by turn 7, with losses. Other
scenarios: no capture count drops by more than 1 in 6; scenarios with ranged units mostly unaffected (the stock test
passes once the city has taken damage). small_vs_strong and medium_vs_strong may lose more units.
Result: m1f6a/b/c (`ab-3fd2741`, newAI attacker, full set x 3, clean): 60 of 90 (m1f3: 62). melee_only_vs_medium 0/6
(LOWDMG 5, NOATTACK 1). Others within 1 in 6 of m1f3 (small_vs_medium 5/6, medium_vs_medium_rough 0/6), losses
about equal. Prediction missed.
What the log shows (m1f6a melee_only rep 1, game turns 63-74): battle turn 2, five Swordsmen "allowed" (city hp 250,
damage 15, received 32), yet the tactsim ("6 units and 2 enemies ... 561 completed") picked positions with no attack
and the city took 0 damage. Battle turn 3 on: the wounded Swordsman went to heal ("heal (second pass): Swordsman 1289,
hp 68"), five were left within 2 tiles, the army test came out just short (about 302 against 333) and every attack
was "refused (too early)" again; the second tactsim "used 807 positions, 0 completed" and the capture ABORTED.
Decision: reverted (code only). Taught: two more blocks behind the first: (1) an allowed attack still loses to a
no-attack position in the simulation's scoring (the "2 enemies": city and garrison, or a unit outside; look at how
the tactsim scores a melee city attack against ending the turn adjacent, and why 0 positions complete); (2) units
leave to heal at 68 HP mid-siege, which shrinks the army. Rule 9: the next iteration on this failure is the third.

## 2026-11-19 M1 iteration 3: no exploring with units at the enemy's walls
Class: "shot to its last hit points, not taken, melee alive" in takeable scenarios (BASELINE.md): large_vs_strong
3/6, siege_train_vs_strong 4/6, medium_vs_medium_rough 6/6, small_vs_medium 1/6. Still the largest failing class.
Iteration 2's result: m1d2 (`ab-30a92af`, the four scenarios x 2, stock AI): small_vs_medium 2/2 and large_vs_strong
2/2 captured, siege_train_vs_strong 1/2, medium_vs_medium_rough 0/2 (LASTHP both). **The heal cause is wrong:** not
one attacking melee unit was sent to heal by the tactical AI (0 "heal (" lines for Songhai melee).
Cause (verified, `PlayerEconomyAILog.csv` with the "melee near" lines): `CvEconomicAI::DoReconState` turns melee
units into explorers ("Creating new land explorer (Swordsman 1222). Have 0, want 2.73"). It takes candidates
farthest from the player's own cities first, i.e. the army at the enemy's walls, and units become candidates when
more than 3 turns old, which is battle turn 4 in the lab. siege_train_vs_strong rep 2: Swordsmen 1222 (94 HP) and
1221 converted on battle turns 7 and 8, "available 0" to the tactical AI from then on, city at 21 HP for 4 turns.
The same in every run since m0d: in every LASTHP battle of siege_train_vs_strong (6), large_vs_strong (6) and
small_vs_medium (1) in m0d-f, sw0, sw1 and m1d2, 2 or 3 of the 2-3 melee units were converted, most on battle turns
4-8; in captured battles usually 0-2, later. medium_vs_medium_rough: one Spearman converted on turn 4; the other
"left to homeland" and walked to garrison the empty home city (a second cause, for later). The earlier "Spearman
healing" lines in the homeland log were explorers healing. The lab inflates how often this fires (no scouts, a
mostly unknown map: want 2.5 explorers), but the decision itself is a blunder a human wouldn't make.
Change: in `DoReconState`, with `IsNewAI()`, a combat unit that isn't a native explorer is not a candidate while it
is within 5 tiles of a revealed city of a player we are at war with (`CvEconomicAI::IsNearEnemyCityAtWar`); logged
as "Not exploring with ...". Revealed cities only, no saved state, deterministic.
Predicted (newAI attacker, 6 battles each): no attacking melee unit converted within 5 tiles of the city.
siege_train_vs_strong and large_vs_strong to at least 4/6 captured (from 2/6), small_vs_medium at least 5/6 (from
4/6); medium_vs_medium_rough may stay failing (the second Spearman still goes home): at most 3/6. Then the full set:
no other scenario's captures drop by more than 1 in 6; melee_only_vs_medium stays at 0/6 (a different cause).
Build: `ab-401f952` (code of 22902fd plus a workflow change; 22902fd's own build was cancelled). Run: queue request 2026-11-19-03-m1d3 (m1d3a, m1d3b).
Result: m1d3a/b (`ab-401f952`, newAI attacker, 6 battles each, no assertions, no lab errors): small_vs_medium 6/6
(turn 6; stock 4/6), large_vs_strong 6/6 (turns 6-7; stock 2/6), siege_train_vs_strong 6/6 (turns 6-10, five by
turn 9; stock 2/6), medium_vs_medium_rough 0/6 (LASTHP 6, as predicted). All melee alive at the end of every battle.
104 "Not exploring with" lines. Conversions still seen (large_vs_strong 6, rough 6) were all of Spearmen that the
tactical AI had "left to homeland" and that had walked more than 5 tiles off (to sentry points or to garrison the
home city) before being converted: the second cause, not a gap in this change. Targeted prediction met.
Full set: m1f3a/b/c (`ab-401f952`, newAI attacker, 3 launches x 2 = 6 per scenario, no assertions, no lab errors):
62 of 90 captured (stock baseline m0d-f: 50). small_vs_medium 6/6 (turns 6-7), large_vs_strong 6/6 (6-7),
siege_train_vs_strong 6/6 (6-9). No scenario's captures dropped; no losses rose (mean losses equal or lower
everywhere). Untargeted moves: medium_vs_strong 1/6 (turn 12; stock 0/6), medium_vs_medium_rough 1/6 (turn 5).
melee_only_vs_medium 0/6 (NOATTACK 6), ranged_only_vs_weak WALK 6, small_vs_strong 0/6: unchanged.
Decision: accepted. Three of the five failing takeable scenarios now meet the M1 bar (5 of 6 within the reference
turns). Left: medium_vs_medium_rough and melee_only_vs_medium. Taught: the AI's own logs beat inference from code;
iteration 2's heal theory was plausible and wrong.

## 2026-11-19 Process: run queue and handover rule (user request)
The user asked for two things, now in PLAN.md: a handover rule (section 12: at milestone sign-off or above
300,000 tokens of context, write the current state, start a successor Opus 5.5 session with a fixed prompt, tell
the user its name and stop), and a run queue in the repo instead of run requests by message (section 4;
`ai-lab/runs/README.md`, `Watch-LabQueue.ps1`, `Register-LabWatcher.ps1`). Implementer 1 was at about 546,000
tokens when the rule was written, so it handed over at once.

## 2026-11-19 M1 iteration 2: why melee units don't finish the city (logging only)
Class: "shot to its last hit points, not taken" in takeable scenarios: large_vs_strong 3 of 6, siege_train_vs_strong
4 of 6, medium_vs_medium_rough 6 of 6 (rough1/2), small_vs_medium 1 of 6 (BASELINE.md). The largest failing class.
Iteration 1's result: sw0 (-NewAI none) and sw1 (-NewAI attacker) on `ab-d2b4f0c`: the RUN line read the switch
back as 0,0 and 1,0. Captures 16 and 20 of 30; every difference from the baseline was in a scenario that already
varied there (small_vs_medium, large_vs_strong, siege_train_vs_strong, medium_vs_strong), and nothing reads the
switch yet, so it is noise from 2 battles per scenario. Decision: accepted.
What the log shows (sw0, large_vs_strong rep 1, verified): the city reached 21 HP on battle turn 6 and stayed there
to the end. Every turn "capture attempt ... executed: 7 units in reach, 0 melee". The three Spearmen: one was "left
to homeland" every turn and walked to the empty home city to garrison it ("Moving Spearman 1207 to garrison"); the
other two were in "Spearman healing at 36:16" in the homeland log for the rest of the battle.
Inferred from code, to check: `CvUnit::shouldHeal` second pass (after attacks) sends any unit missing more than 20
HP and not next to an enemy away to heal, with no regard for a city about to fall; and the capture attempt only
counts melee that can get adjacent this turn, so a Spearman 3 tiles out is never part of it, and nothing moves it in.
Change: two log lines in `CvTacticalAI.cpp`, no behaviour change: every combat unit the tactical AI sends to heal
(pass, position, HP); and at each capture attempt, every own land melee unit within 5 tiles of the city (distance,
HP, moves, whether it is available to the tactical AI this turn, whether it counts as in reach, its tactical move).
Predicted: the lines show the Spearmen sent to heal in the second pass while 2-4 tiles from a city at 21 HP, or
unavailable to the tactical AI because the homeland AI holds them.
Result: pending (diag M1-2: large_vs_strong, siege_train_vs_strong, medium_vs_medium_rough, small_vs_medium x 2).

## 2026-11-18 M1 iteration 1: the new-AI switch (plumbing, no behaviour change)
Class: none (plumbing, PLAN.md M1).
Change: `CvPlayer::IsNewAI()`, the one switch every new behaviour will sit behind (rule 5). It reads custom game
options, which the game already saves by name and shares in multiplayer: `GAMEOPTION_AI_V2_ALL` (every AI major
civ) and `GAMEOPTION_AI_V2_PLAYER_<id>` (one player). So no new saved field and no change to the save format;
older saves load as before. The value is cached in `m_bNewAI` (not saved), refreshed at the start of the player's
turn and after loading. `CvPlayer::SetNewAI` sets the player's option. Lua: `Player:IsNewAI()`,
`Player:SetNewAI(bool)`. Lab v9: `-NewAI none|attacker|defender|both` on Install-SiegeLab.ps1 and Run-SiegeLab.ps1;
the RUN line gains the two sides' switch as read back from the DLL (1 on, 0 off, -1 the DLL lacks it). Nothing reads
`IsNewAI()` yet. Note for later: options set from Lua mid-game are local to one machine; real multiplayer games
must set the option in the game setup (M7).
Predicted: on the new build the RUN line reads 0,0 with -NewAI none and 1,0 with -NewAI attacker, and both runs
match the stock baseline's classes (same build behaviour, so within the noise of BASELINE.md).
Result: see M1 iteration 2.
Decision: accepted.

## 2026-11-18 M0 iteration 10: an aggressive scripted attacker
Class: the reference script still played worse than the stock AI.
Iteration 9's result (ref2, rules v2, 5 scenarios x 1): same capture turns as ref1 (small_vs_weak 3, medium_vs_medium
5, ref_cb4 7), fewer losses; melee_only_vs_medium and siege_train_vs_strong not taken. In small_vs_weak both
Spearmen hit the city at 13% on turn 2 without taking it; in siege_train_vs_strong the Catapults were hurt early,
rested most of the battle, and the city out-healed the remaining shots (most damage 29 of 250; the stock AI gets it
to its last hit points every time). Caution was the problem.
Change (rules v3, after the user's ruling on losses): nobody falls back; ranged units fire every turn (enemy field
units first, then the city); melee attack the city from half health down (or, with no ranged units, whenever they
are at half health or more) and otherwise stand 2 tiles out.
Predicted: the script takes at least everything the stock AI takes, no slower, and takes some of the scenarios the
stock AI doesn't (melee_only_vs_medium, siege_train_vs_strong, the strong cities), losing units to do it.
Result: ref3a/b/c (rules v3, `ab-00ae537`, full set x 2, three launches): 21, 24 and 22 of 30 captured (stock AI:
17, 17, 16), no errors. The script took everything the stock AI takes, at the same turn or within one, and also
melee_only_vs_medium 6/6 (AI 0/6), siege_train_vs_strong 6/6 (AI 2/6), large_vs_strong 5/6 (AI 2/6) and
medium_vs_medium_rough 6/6 (AI 0/6). Neither takes small_vs_strong, medium_vs_strong or ranged_only_vs_weak; the
script did worse than the AI only on small_vs_medium (2/6 vs 4/6). Prediction met except small_vs_medium.
Decision: accepted as the reference. Takeable list and reference turns in BASELINE.md, for the user's sign-off.

## 2026-11-18 M0 iteration 9: better rules for the scripted attacker
Class: the reference script played worse than the stock AI in ref1.
Iteration 8's result (ref1, lab v8, `ab-00ae537`, 5 scenarios x 1): the hook works: 165 SCRIPT lines, no errors, the
units moved as the script said. Captures: small_vs_weak turn 3 (stock AI: turn 2), medium_vs_medium turn 5 (AI 3-4),
ref_cb4_melee6_vs_walls turn 7 (AI 5-6); melee_only_vs_medium not taken (66 damage, the AI never attacks);
siege_train_vs_strong not taken, 4 of 8 units lost (the AI took it 2 of 6 and lost none).
Causes (from the SCRIPT lines and turns.csv): melee waited 2-3 tiles out, so at "attack city (city at 4%)" a unit
3 tiles away could only walk up, and the capture came a turn late; wounded units went back into the city's fire as
soon as they were above 35%, so they never healed (melee_only: from turn 5 the same Swordsmen fell back every turn);
enemy Spearmen outside the city were only shot when already half dead and melee only hit units at 40% or less, so the
siege train's Catapults were worn down.
Change (rules v2, lab v8 still): wounded units rest 4-6 tiles out until back at 75%; ranged units shoot the most
damaged enemy field unit in range before the city; melee attack an adjacent enemy field unit no healthier than
themselves; melee move up to 2 tiles from the city once it is at three quarters health or less, attack it at a
quarter or less, and otherwise wait 3 tiles out, beyond the city's fire.
Predicted: small_vs_weak taken on turn 2; medium_vs_medium by turn 4; siege_train_vs_strong taken with few losses;
melee_only_vs_medium uncertain.
Result: see iteration 10.
Decision: superseded by iteration 10.

## 2026-11-18 M0 iteration 8: a scripted attacker as the reference
Class: none (M0 needs a reference; see the user ruling of 2026-11-18).
Before it, iteration 7's result: rough1 and rough2 (lab v7, 2 launches x 3): medium_vs_medium_rough shot the city
to its last hit points and didn't take it in 6 of 6 battles. With the ruling on close cases, all 15 scenarios now
repeat. (rough2's first launch didn't start a game; its retry was clean.)
Change: lab v8 adds a scripted attacker (`-Script` on Install-SiegeLab.ps1 and Run-SiegeLab.ps1). It moves the
attacker's staged units from the `PlayerPreAIUnitUpdate` hook, which the DLL fires at the start of the AI's unit
update, after moves are restored and before the tactical AI runs, then ends their moves. Rules, fixed for all
scenarios: units at 35% health or less fall back 4-6 tiles; ranged units shoot an enemy unit at half health or less
within range, else the city, moving to a free tile 2 from it first if needed; melee attack the city at a quarter
health or less (or, with no ranged units in the army, whenever they are at 80% health or more), kill adjacent enemy
units at 40% or less, and otherwise wait 2-3 tiles out. Each action is logged as a SCRIPT line in Lua.log. RUN's
attacker field is 2 for this mode. Not verified in the game yet: that `PushMission` from that hook executes at once
for an AI-controlled player. No DLL change; AI mode is unchanged (mock run passes).
Predicted: SCRIPT lines for every battle turn; the AI's tactical log shows no moves for the attacker's staged
units; the script takes every weak and medium city within 2-5 turns and at least sometimes the strong ones.
Result: see iteration 9.
Decision: accepted (the mechanism); the rules are revised in iteration 9.

## 2026-11-15 M0 iteration 7: the same rough terrain on every map
Class: lab problem: medium_vs_medium_rough never attacked in m0d and m0e (4 battles) and shot the city down in m0f
(2 battles); within a launch both battles agreed.
Cause: lab v6 laid hills and forest at every other tile of rings 2-3 west of the city in map-scan order, filtered by
map x. Hex rows are offset, so the layout depended on where the arena landed, which changes with each map.
Change: lab v7 walks rings 2 and 3 with hex directions from the city and alternates along that walk, on the west
half of each ring. Same layout on every map. Mock run passes. No DLL change.
Predicted: medium_vs_medium_rough lands in one class (or two neighbouring classes) in at least 5 of 6 battles
across two launches. Which class can't be predicted: the new layout differs from all of the old ones.
Result: see iteration 8 (all 6 battles: shot to its last hit points, not taken).
Decision: accepted.

## 2026-11-15 M0 iteration 6: no research during a run
Class: lab problem. m0a/b/c (lab v5, `ab-00ae537`, full set x 2, three launches = 6 battles per scenario):
12, 13 and 11 of 30 captured. Only 6 of 15 scenarios landed in one class in at least 5 of 6 battles. In every
scenario with melee units, rep 2 (the run's second half) ended with 0 melee alive although none was killed by the
defender.
Cause (verified in `PlayerHomelandAILog.csv`): "Disbanding long obsolete unit. Spearman/Warrior" on the first turns
of every rep-2 battle. `CvEconomicAI::DisbandLongObsoleteUnits` scraps a unit whose tech era is more than 2 eras
below the player's current era. The lab removes techs at every battle but not the progress made on them, so a tech
researched once came back at once later and the attacker climbed eras over the run; removing a tech doesn't lower
the era.
Rep 1 alone (before the climb) agrees well: 13 of 15 scenarios in one class 3 of 3; captures on turn 2 for the
weak cities, 3-6 for medium, 5-7 for siege_train and ref_cb4; small_vs_strong and medium_vs_strong barely engage;
melee_only_vs_medium never attacks; ranged_only_vs_weak walks home; medium_vs_medium_rough never attacks.
Also: one assertion per launch, `m_iOverflowResearch >= 0` (CvPlayer.cpp 30402). Not yet understood; the dialog only
shows once per session, so it may fire more often. Expected to be tied to the same research handling.
Change: lab v6 sets every tech's research progress and the overflow to 0 at each battle and at the start of each
side's turn (`clearResearch`). Mock run passes. No DLL change.
Predicted: no "long obsolete" lines; rep 2 matches rep 1; at least 12 of 15 scenarios in one class in 5 of 6 battles;
captures around 20 of 30 per launch. The assertion may disappear.
Result: m0d/e/f (lab v6, `ab-00ae537`): 17, 17 and 16 of 30 captured. No "long obsolete" lines, no assertions (the
overflow assertion went with the research reset). 10 of 15 scenarios in one class in at least 5 of 6 battles, below
the 12 predicted. The other five: small_vs_strong splits between two classes that mean the same thing (barely
engages); large_vs_strong, siege_train_vs_strong and small_vs_medium vary within a launch at a threshold (melee in
before turn 12 or not); medium_vs_medium_rough varies by launch, because its hill and forest pattern follows the
arena's coordinates and so changes with the map. Table in BASELINE.md.
Decision: accepted as the stock baseline. With the user's ruling on the bar (close cases count), 14 of 15
scenarios repeat; medium_vs_medium_rough is iteration 7.

## 2026-11-15 M0 iteration 5: the attacker knows where its target is
Class: lab problem: in a run's first battles the attacker's AI has never seen the target tile (new map each launch).
Cause: see iteration 4's result.
Change: lab v5 shows the city's tile and its ring to the attacker's team for a moment when staging
(`ChangeVisibilityCount` +1 then -1), as if its army had seen it on the march. Live sight is not given, so the
spotter behaviour stays the AI's own. Mock run passes. No DLL change.
Predicted: every battle has a zone for its city on its first turn; small_vs_weak and medium_vs_weak capture 3 of 3
each; ranged_only_vs_weak still walks home 3 of 3 (no spotter).
Result: diag6 (lab v5, `ab-00ae537`, 3 x 3): exactly as predicted. small_vs_weak captured 3 of 3 and
medium_vs_weak 3 of 3, each on the battle's 2nd turn with no losses; ranged_only_vs_weak walked home 3 of 3 with 0
damage, every capture attempt "ABORTED ... target visible 0". Every battle had its zone from its first turn.
(The runner's first launch failed to get past the menus; it loosened the screen-match tolerance for the "menu" and
"single" screens from 2.5 to 5 in Run-SiegeLab.ps1, commit 0aea460, and the second launch ran clean.)
Decision: accepted. Lab v5 is the baseline lab. Next: the full set three times (M0 repeat check).

## 2026-11-15 M0 iteration 4: stage battles before the attacker's turn
Class: lab problem: on the first turn of every battle the attacker's AI doesn't know the target city exists.
Cause: `SiegeLab.lua` staged each battle in the attacker's own `PlayerDoTurn`, which the DLL fires after that
player's AI has read the map for the turn (`CvPlayer::doTurnPostDiplomacy`, after `AI_doTurnPost`). The tactical
log shows the zone appearing only on the battle's second turn.
Change: lab v4 stages the battle during the defender's turn (after scoring the previous battle there), so the
attacker's first battle turn starts with the city on its map. Side effect: the defender's field units now get one
move before the attacker's first turn. Mock run passes (30 of 30). No DLL change.
Predicted: no battle on the first turn has all units "left to homeland"; small_vs_weak and medium_vs_weak stop
walking home and land in the same class in at least 5 of 6 battles each ("shot to last hit points, not taken",
since the melee problem is untouched); ranged_only_vs_weak unchanged.
Result: diag5 (lab v4, `ab-00ae537`, 3 scenarios x 3): small_vs_weak and medium_vs_weak captured in reps 2 and 3
(4 of 4, within 2-3 turns, no losses) but walked home in rep 1; ranged_only_vs_weak walked home 3 of 3 with 0 damage.
From battle 3 on, the zone appears on the battle's first turn, as predicted. But the run's first two battles had no
zone on their first turn (battle 1 never had one), and their capture attempts logged "target not visible ...
ABORTED". On a new map the attacker has never seen the target's tile, so the city isn't on its map until a unit sees it.
ranged_only_vs_weak: every capture attempt "target not visible, looking for a spotter among 6 units ... ABORTED,
target visible 0", then all units left to the homeland AI. Verified: with no ATTACK-type unit in the group,
`ExecuteSpotterMove` finds no spotter and the attack is dropped before a shot. With Spearmen in the group (reps 2-3)
the spotter search succeeded and the attack ran with "2 melee" in reach. Real stock-AI failure; kept as a scenario.
Decision: accepted, with iteration 5 for the first-battles problem.


## 2026-11-15 M0 iteration 3: why the army walks home (logging only)
Class: walked home: 4 of 4 in diag3 (`ab-e2b44ca`, small_vs_weak and medium_vs_weak, 2 each), 6 of 6 in diag2,
2 of 4 in diag1. City damage 0 in all of them.
What the logs show (diag3, verified): on a battle's first turn the tactical AI has no zone for the new city (the
zone line names the previous, razed city). From the second turn it has one, posture `P_STEAMROLL`, and logs
"attempting capture of <city>, expected max damage per turn 99/128", yet no attack happens. Meanwhile the homeland
AI moves the Spearmen to "sentry point" and "garrison" at the home city 12 tiles west, and later the Composite
Bowmen to "garrison" too. As the army leaves, expected damage falls (128, 64, 32), the AI logs "too early for
attacking", and the posture turns to `P_WITHDRAW` for the rest of the battle.
Inferred from code, not yet seen in a log: `ExecuteAttackWithUnits` starts with `ExecuteSpotterMove`, which returns
false when the city tile isn't visible and no unit of AI type ATTACK, FAST_ATTACK, COUNTER, SKIRMISHER (or naval)
is in the group. The army stands 3 to 5 tiles away with sight 2, and Spearmen and Composite Bowmen are DEFENSE and
RANGED, so the attack is abandoned before any shot. Units the tactical AI doesn't use go to the homeland AI, which
sends them to guard the empty home city. Open question: diag1's ranged_only_vs_weak (6 Composite Bowmen, same
start) did shoot the city to its last hit points, so something else moved those bowmen into sight.
Change: three log lines in `CvTacticalAI.cpp`, no behaviour change: the outcome of each capture attempt (executed
or ABORTED, units in reach, melee count, target visible); "target not visible, looking for a spotter"; and every
combat unit the tactical AI leaves to the homeland AI, with its position.
Predicted: in small_vs_weak the capture attempt logs ABORTED with target visible 0 and the Spearmen and Bowmen
appear in the "left to homeland" lines; ranged_only_vs_weak and ancient_rush_vs_open_city (Warriors are ATTACK
units) show what differs when the army does engage.
Result: diag4 on `ab-00ae537` (ancient_rush_vs_open_city, small_vs_weak, ranged_only_vs_weak, 2 each, a new map):
all 6 shot the city to 179 of 200 damage (its last hit points), none captured, none walked home. So small_vs_weak
walked home 6 of 6 times in diag2/3 and engaged 2 of 2 here: same build behaviour, different launch.
Verified from the new lines:
- On every battle's first turn, every staged unit is "left to homeland" (no zone for the city yet).
- From turn 2, "capture attempt ... executed", target visible 1, with "0 melee" in reach on every turn of every
  battle. No ABORTED and no "not visible" line at all, so the spotter idea was not tested by this run.
- Every melee unit (Warriors, Spearmen) is left to the homeland AI on every turn, and the homeland AI moves them to
  "garrison" and "sentry point" at the home city. That is why the city sits on its last hit points untaken.
Decision: accepted (logging stays). What decides engage or walk home: on turn 1 the homeland AI moves the whole army
(the lab's fault, iteration 4); whether the bowmen can still see the city on turn 2 depends on where they were sent,
which depends on the map. The melee units leaving is a real stock-AI failure and becomes M1's first class.


## 2026-11-15 M0 iteration 2: a DLL that writes the AI logs
Class: lab problem. diag2 (6 battles, stock DLL, `AILog = 1` before and after) wrote no AI log at all; the game's
`Logs` folder has AI logs only from Nov 7, the earlier experiment's builds.
Cause: the pack's game core is loaded as a DLC and never receives the `[Debug]` settings, so the stock DLL's
`GC.getAILogging()` stays false whatever `config.ini` says. The earlier experiment found the same and patched it
(fork commit `4cf291b`, `RESULTS.md` in the old notebook).
Change: `CvGame::init` reads `LoggingEnabled` and `AILog` from `config.ini` itself (`ApplyConfigLoggingSettings`,
commit `03b00ce`). Logging only, not behind the AI switch, since it changes no decision. New `Set-LabDll.ps1`
swaps only the pack's `CvGameCore_Expansion2.dll` for a release build and restores the saved stock copy (checked by
SHA-256); `Run-SiegeLab.ps1 -Release ab-<commit>` uses it before the run and restores in its `finally`.
Predicted: a run on the build writes `PlayerTacticalAILog.csv`, `MilitaryAILog.csv`, `OperationalAILog.csv` and
the diplomacy logs. The battle results should match stock (no behaviour change).
Result: diag3 on `ab-e2b44ca` (4 battles): 91 log files, 3.8 MB, including `PlayerTacticalAILog.csv` and
`PlayerHomelandAILog.csv`. The stock DLL was restored and its hash checked by the script. Battle results as in diag2.
Decision: accepted. Also: the downloaded SDKs are now cached for `ai/v2`, so builds take about 10 minutes.
Also seen: Civ V did not close within 30 s of `CloseMainWindow` and was stopped by force, so the logs' last
buffered lines may be lost. To be checked against the first logged run.

## 2026-11-15 M0 iteration 1: get the game's AI logs saved
Class: lab problem, not an AI failure class. No run so far has saved an AI log (diag1: "No tactical AI logs found").
Cause (from the scripts, not yet checked on the PC): `Install-SiegeLab.ps1 -EnableLogging` only ever set
`LoggingEnabled`, never `AILog`; the reader only looked for files named `*Tactical*`; and the runner killed Civ V
with `Stop-Process -Force`, while the DLL opens its AI logs with `kDontFlushOnWrite`, so a kill can lose their tail.
Change: installer sets `AILog = 1` too; runner closes the window and waits 30 s before forcing; reader copies every
log written during the launch plus a listing of the `Logs` folder. Scripts only, no DLL change. Commit: see git log.
Predicted: the next run's results folder holds `PlayerTacticalAILog*.csv`, `MilitaryAILog*`, `OperationalAILog*`
and the diplomacy logs.
Result: diag2 (6 battles) saved 14 logs from the launch and a listing of the folder, but no AI log: the scripts were
not the only problem (iteration 2). All 6 battles again 0 captures, 0 damage; see the diag2 results.
Decision: accepted (the scripts now do their part).

Lead for the "engage or walk home" split (inferred from code, unverified): the lab stages each battle inside the
attacker's `PlayerDoTurn` Lua event, which `CvPlayer::doTurnPostDiplomacy` fires after `AI_doTurnPost`, so on a
battle's first turn the military and diplomacy AI have already made their turn's decisions about a world without
the new target city and army; only the unit moves (`AI_unitUpdate`) see them. In diag1 the walk-home happened in
the run's first two battles (game turns 1 and 13); in stock548-1021 in battle 1 (after shooting the city) and in
`medium_vs_weak` rep 2 (turn 236). The logs are meant to settle it.
