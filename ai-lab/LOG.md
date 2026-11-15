# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## User rulings

(none yet)

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
