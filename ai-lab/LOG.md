# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## User rulings

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
Result: pending (build, then sw0 and sw1).

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
