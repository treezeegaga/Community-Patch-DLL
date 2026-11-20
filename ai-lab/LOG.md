# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## Current state (2026-11-19, 9:00 PM ET), written by implementer 5 at handover (context past 300k)

- **Milestone:** M2 (the march). Iteration 1 (logging) done in part: cause confirmed (LOG "M2 iteration 1", m2n1a).
  **Iteration 2 in hand:** new AI's land target range x2 in `CvMilitaryAI::GetArmyPathsFromCity` (commit `dce08ed`,
  build `ab-455fe7a`, check it published). Its runs are queued overnight; result not in. M2's bar is not fixed yet.
- **Builds:** `ab-455fe7a` = M1 behaviour + M2 iteration 2 + `Player:LabEndOperations` (lab-only Lua, `1ca9bea`).
  `ab-1ca9bea` = same without iteration 2. Lab is v13 (ends both sides' AI operations between battles; with an older
  DLL it WARNs and behaves as v12). New scenario march_medium_vs_medium_rough.
- **Queue (all pinned to BOX, in order):** rest of `2026-11-19-12-m2d1` (old lab, 10-tile battles void: lab leak);
  `-14-reg0` (regression baseline, ab-78d5e8a); `-15-m2d2` (8 and 10 tiles on v13, new AI and stock); `-17-m2f2`
  (iteration 2: march set x6, new AI vs stock); `-18-m2g2` (siege set x6, new AI: M1 bar must hold, m1f7 68/90);
  `-19-m2x2` (regress set x6, new AI); `-20-m2r3` (script on the rough march x6). Also `-16-pc2tree` pinned to
  TREEDESKTOP (its half of the two-PC check).
- **Read when in:** m2d1 rest: d8 capture counts, d12/d16 "no army path" (fill M2 iteration 1 Result/Decision).
  reg0: record the regression baseline in BASELINE.md (with "Attackers lost"). m2d2: does the army march at 10 tiles
  on v13? m2f2: iteration 2's prediction (LOG "M2 iteration 2"); then decide accept/revert by PLAN section 7 and
  propose the M2 bar to the user with these numbers. m2g2/m2x2: no regression.
- **Lab PCs:** BOX runs watcher v3 (status file `ai-lab/runs/status/BOX.json`, pull self-heal, explicit ai/v2 fetch;
  BOX's 5:09-8:19 PM stall was a stale origin/ai/v2, see LOG). `ai-lab/runs/trusted-hosts.txt` lists BOX only:
  unpinned launches go only to trusted PCs. TREEDESKTOP: session "TREEDESKTOP lab setup step 4"
  (session_01G2NZj3SpEuy1gkydMfsdEi) is finishing setup steps 6-8 on my instructions (the user told it to do what the
  implementer asks). Pack DLL hashes differ (BOX `2FB6A9B9...`, TREEDESKTOP reported `262AEE88...`); it was asked to
  list all DLLs in its pack. Add TREEDESKTOP to trusted-hosts.txt only after the two-PC check passes
  (runs/README.md bar: pc2tree4-6 vs pc2box1-3).
- **Runner sessions report to implementer 5** (session_01G2NZj3rXkHwRtW56xPvkxL): tell the BOX runner
  ("Civ V siege lab: PC runner (box)", session_01G2NZj1Snwc6GRS553uhEJb) and the TREEDESKTOP session your own
  session id to report to. This session is elevated (may message the user's PC sessions); a successor may need the
  user to allow it again (confirm_repository_trust card on first refusal). Standing permission: lab upkeep through
  the runner sessions without asking (PLAN.md section 4).
- **Setup switch:** Install-NewAI.ps1 now handles the user's folder-loading pack (files under `Mods\AILab NewAI\`,
  .Civ5Pkg untouched); not yet tried on a PC. Next: have a runner do the single-player check when the queue is
  empty (install ab-455fe7a, the "New AI (test build)" box in Advanced Setup, one turn, AILab_NewAI.csv lists the AI
  civs, -Remove), then the MP smoke test (ai-lab/newai/MP-SMOKE-TEST.md) for the user.
- **The user's goal tonight:** a long overnight test run with little user input. User has set screen and sleep to never.
- **Dashboard:** https://claude.ai/artifact/VuEFfTGtehw613tQvRx9Gj (not updated this session).
- **Next:**
  1. Point the runners at your session id; confirm `ab-455fe7a` published (else rebuild and requeue 17-20).
  2. Read results as they land, in queue order; record M2 iterations 1 and 2, the regression baseline.
  3. Decide iteration 2; propose the M2 bar to the user.
  4. TREEDESKTOP: calibration report, pack hashes, two-PC check; trust it when it passes.
  5. Single-player check of the setup switch via a runner; then the MP smoke test for the user.
  6. Transfer check (PLAN 8a item 4), first for M1. Update the dashboard.

## User rulings

- 2026-11-19 (about 9:30 PM ET): the user is asleep for 8 hours and asks for no approvals meanwhile ("I trust you").
  Overnight the implementer carries on by the plan's rules without asking. Anything that needs a ruling is written
  up for the morning, and the work goes on with whatever doesn't depend on it.

- 2026-11-19 (about 9:30 PM ET): hand over at milestone sign-off or past 650,000 tokens of context, whichever comes
  first (was 300,000). New `ai-lab/CODEMAP.md` (game functions, gates, log lines, script structure), kept current
  every iteration and read by a successor right after PLAN.md. At a milestone's close its entries move to
  `ai-lab/LOG-archive.md` and LOG.md keeps a short "what we learned" (done now for M0 and M1). Scheduled check-ins
  are refused for this session: wait for runs with `ai-lab/runs/wait-for-results.sh` in the background (PLAN.md
  section 7, step 7).

- 2026-11-19 (about 9:10 PM ET): an AI land attack in the classical era waits for 3 melee units
  (`MUFORMATION_SMALL_CITY_ATTACK_FORCE`), and spare ranged units get no slot. Ruling: "both, in order". First
  measure the march with armies that fill the formation (3 melee; a lab change only). Then, as its own iteration, the
  new AI "goes with what it has": when none of its units can fill the missing melee slot, it marches with at least 2
  melee plus its ranged units, and spare ranged units join the army.

- 2026-11-19: standing permission for lab upkeep through the PC runner sessions (BOX's, and TREEDESKTOP's once it
  exists): diagnose and fix a stuck queue, read-only checks, install or remove the lab, without asking first; never
  graphics settings, saved games or the user's own play; tell the user afterwards (PLAN.md section 4). Not chosen:
  quieter updates and unannounced check-ins (so: keep reporting as before).

- 2026-11-19: handover rule addition (PLAN.md section 12): after a successor has read the current state and told the
  user what it will do first, it archives its predecessor session, provided the predecessor has stopped, has pushed
  everything and has no scheduled tasks bound to it. Archive only, never delete; if archiving is refused, tell the
  user the session name.

- 2026-11-19: four additions to the plan (PLAN.md section 8a), in this order after the second-runner work: the
  regression scenario set (capital, coastal, medieval, artillery era; stock and current build; units lost kept as a
  reported column), then the new-AI switch at game setup plus a 30-turn LAN multiplayer smoke test for the user, then
  the transfer check at each milestone close (6 seeded games to turn 100, one aggressive new-AI civ, both PCs;
  informs, doesn't gate). M2 continues between them, one thing at a time. The single "can this army take this city"
  estimate moves from M4 to the start of M3 (PLAN.md updated).

- 2026-11-19: the user asked for their second PC, TREEDESKTOP, as a second lab runner: per-launch claims in the run
  queue so two watchers never run the same launch, the host recorded with every launch, setup steps for them, and a
  same-set stock check on both PCs before TREEDESKTOP's results are trusted. Done as run queue watcher version 2
  (ai-lab/runs/README.md); check queued as 2026-11-19-13-pc2check. Until it passes, requests that count name
  "host": "BOX".

- 2026-11-19: M1 signed off by the user (13 of 14 takeable scenarios meet the bar, melee_only set aside, defender part
  met). The single "can this army take this city" estimate moves to M4.

- 2026-11-19: melee_only_vs_medium (melee-only army, no ranged units) is set aside as an accepted gap for M1, after
  three iterations without a capture (rule 9 stop after M1 iteration 8). The other takeable scenarios carry the bar.

- 2026-11-15: the M0 repeat bar counts close cases. A scenario repeats if at least 5 of its 6 battles land in the
  same class or in two neighbouring classes (for example captured and shot to its last hit points, or never attacked
  and too little damage). Asked because four scenarios sat on a threshold (BASELINE.md).
- 2026-11-18: the user won't play the human reference battles. A scripted attacker in the lab plays them instead,
  and its results take the place of the human column in BASELINE.md (offered as an option on 2026-11-15; the
  user chose it, then briefly said they would play, then confirmed they won't). Consequence: "takeable" now means
  "a simple fixed script takes it", which can be harder or easier than what the user would manage.
- 2026-11-18: "It's completely fine if armies lose their units as long as they are able to capture the city."
  Losses don't count against the AI; the capture does. The M1 bar's "army keeps two thirds of its units" for
  scenarios marked not takeable is dropped (PLAN.md updated), and the scripted attacker no longer protects its units.
- 2026-11-18: M0 signed off by the user, with the takeable list in BASELINE.md as proposed. M1 starts.

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

## M1 (closed 2026-11-19): what we learned
Entries archived in `LOG-archive.md`. Bar met on `ab-78d5e8a`: 13 of 14 takeable scenarios captured 5 of 6 within
1.5x the reference turns (68 of 90 overall; stock 50-57); melee_only_vs_medium set aside by the user; the new AI as
defender doesn't help stock attackers (55 vs 57 of 90).
- Accepted (behind `IsNewAI()`): **it. 3** `DoReconState` no longer turns units within 5 tiles of an enemy city at war
  into explorers (the cause of most "last hit point, not taken": 2-3 melee converted on battle turns 4-8). **It. 7**
  `PlotReinforcementMoves`: melee within 6 tiles close in to ring 2, then ring 1 at half health (rough-ground fix).
- Wrong or reverted: it. 2's heal theory (no unit was sent to heal: read the logs before the code); it. 4, 6, 8 on
  melee-only armies. `ScoreAttackDamage` refuses a lone melee hit that costs more than it deals, scores damage dealt
  minus taken so a costly hit loses to no attack, and `ExecuteCaptureCityMoves`' "too early" estimate (3 melee max,
  wounded dropped, 13 turns) disagrees with it. Two functions judge "can this army take this city": one function is
  M3's first item.
- Seen, not fixed: units the tactical AI doesn't use are "left to homeland" and walk home to garrison; with the city
  tile unseen, an all-ranged group finds no spotter and aborts (ranged_only_vs_weak).
- Method: per-launch noise is about 1 in 6 per scenario (same build, same PC); the AI's own logs settle causes.

## M0 (closed 2026-11-18): what we learned
Entries archived in `LOG-archive.md`. The lab became trustworthy only after five lab leaks were fixed: AI logs
(the pack's DLL never read `[Debug]`; `ApplyConfigLoggingSettings`), battles staged before the attacker's turn (lab
v4), the target tile revealed (v5), research reset so the attacker doesn't climb eras and disband units as "long
obsolete" (v6), and a rough-terrain layout independent of map coordinates (v7). "Engage or walk home" was the lab
(units sent home on a battle's first turn), not the AI. The user doesn't play reference battles: a scripted attacker
(rules v3, aggressive) is the reference; "takeable" = the script or stock takes it in at least 2 of 6.
