# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## User rulings

(none yet)

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
Predicted: a run on `ab-03b00ce` writes `PlayerTacticalAILog.csv`, `MilitaryAILog.csv`, `OperationalAILog.csv` and
the diplomacy logs. The battle results should match stock (no behaviour change).
Result: pending.
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
