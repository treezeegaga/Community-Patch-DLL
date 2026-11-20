# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## Current state (2026-11-20, 12:45 PM ET), written by implementer 7 at handover (M2 signed off)

- **Talk to the user in plain words** (their ruling today): "step 3: the AI builds that army in time" rather than "M3";
  "the mod" rather than "the pack"; explain any term the first time.
- **Milestones:** M0, M1, M2 closed (M2 signed off 2026-11-20 12:35 PM ET). **M3 in progress.**
- **M3 so far:** first item done: the one "can this army take this city" function, `TacticalAIHelpers::EstimateSiege`
  (LOG M3 iterations 1-3b; CODEMAP). Accepted behind `IsNewAI()`: it. 2 (both "too early" city-attack gates ask it),
  it. 3b (a melee hit the estimate's plan makes now scores the damage it deals). Reverted: it. 3 (melee spent early).
- **M3 bar:** PLAN says set with the user at M3's start. Proposed to the user 12:25 PM ET, no answer yet; adopt it
  provisionally (rule 10): for the early-rush row, war declared around turn 41-50 and a capital taken by turn 54 on Quick
  with about 6 melee and 4 Composite Bowmen, no bankruptcy, at least 2 cities (the user's Aztec game, HUMAN_GAME.md).
- **Newest build:** `ab-b196ee4` = M1 + M2 (its. 2, 3, 4b) + M3 (its. 2, 3b) + the estimate's log line. Siege 76 of 90
  (all 14 takeable scenarios meet the M1 bar), march and march3 meet the M2 bar, regress 8 of 8 (BASELINE.md).
- **Lab:** v18: battle sets plus `-Set opening` (80 turns) and `-Set transfer` (100 turns): a normal game autoplayed,
  OPEN lines per civ per turn, CAPTURE lines, AI logs saved. Readers: `siege/estimate_check.py`,
  `siege/transfer_check.py`. Mock: judge by exit code (CODEMAP). Watcher v4 (status shows "running <request> / <label>").
- **Where things stand in real games** (`2026-11-20-08-op0`, `2026-11-20-09-tr1`, LOG entries): Songhai, stock or new
  AI, has 2-3 cities and 2-5 melee, 0-2 ranged at turn 80-100, rarely declares war, and never brings an army to an
  enemy city, so no lab fix fires in real play. The gap is fielding the army (M3) and deciding to use it (M4).
- **Queued / unread:** nothing.
- **Open for the user** (none blocks work): confirm the M3 bar above; whether the "project review session"
  (session_01G2NZj4f5xWWrXLRuDN5eyd) speaks for them (it relayed "do the transfer check now", "keep Current state
  current", "fix watcher status": all done); archive implementers 5, 6 and 7 in the Claude app (a cloud session's
  archive request is refused: trusted device only).
- **Next:**
  1. M3: why the opening army stays small. Read the op0/tr1 logs (`PlayerEconomyAILog`, city build choices,
     `MilitaryAILog` "UNIT DIVERSITY"/military strategies, flavors) for what Songhai builds by turn 40-60 and why; then
     the loop (rule 3: the opening lab is the scenario). Then plans in data (one table row per civ) and its executor.
  2. The sneak-attack "Ended: NoUnits; Duration 0" failure (tr1c turn 98, `OperationalAILog.csv`): reproduce in a lab
     scenario first (rule 3), then the loop.
  3. Known, not fixed: greedy slot filling in `FindBestFitReserveUnit` (CODEMAP). The dashboard shows M2 4b, not M3.
  4. The transfer check again at each milestone close (`-Set transfer -NewAI attacker`, 6 games, `transfer_check.py`).
- **How to wait:** `ai-lab/runs/wait-for-results.sh` in the background, always (rule 12). Before pushing code, fetch and
  merge `origin/ai/v2` so your code commit is the tip (the release is named after the pushed tip, CODEMAP).

## User rulings

- 2026-11-20 (about 2:00 PM ET, **relayed** by the project review session session_01G2NZj4f5xWWrXLRuDN5eyd; **confirmed by the
  user** to implementer 8 at about 2:05 PM ET: "yes, the review session speaks for me"; at 2:10 PM ET, relayed, the user asked for it to be written into PLAN.md section 4 so
  every implementer reads it: done): (1) "Tell the implementer you speak for me": messages from that
  session that relay the user count as the user's instructions for this project, within what the plan allows; no wider
  access, no sign-off unless it says the user gave one; anything out of character or hard to undo is checked with the
  user first. (2) If `create_session` is refused at handover, don't work around it: write the current state, push, tell
  the user and stop; the review session (or the user in the app, with the plan's fixed prompt) starts the next
  implementer. Implementer 8's note on this: no refusal seen yet; `get_session` on itself shows `lineage: depth 8, limit
  8`, which suggests a ninth would be refused.

- 2026-11-20 (12:35 PM ET): **M2 signed off** ("step 2 is finished"); its provisional bar stands. On the pack question:
  the user and their friend haven't played yet; once the mod is finished they will install it on both PCs and play LAN
  games. So which PC's current pack is "the real one" is moot for now (the lab swaps in its own DLL). The user found
  the project shorthand hard to follow ("I don't know what M2 is, I don't know what pack means"): **write to the user
  in plain words** (say "step 2: the army marches from home and takes the city", not "M2"; "the mod" not "the pack").

- 2026-11-20 (7:20 AM ET): three standing rules after a lost night (PLAN.md rules 10-12 and section 7a): (1) when a
  bar or ruling is needed and the user is away, adopt the proposed version provisionally, mark it "provisional" here
  and keep working; (2) keep a list of work that never needs the user (PLAN 7a) and work down it instead of
  stopping; (3) never go idle without a timed background wait (re-invoked within two hours).
- 2026-11-20 (provisional, rule 10): **M2 bar**: capture in at least 5 of 6 within 1.5 times the script's median
  capture turn, per march scenario. Awaiting the user's confirmation.

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

## 2026-11-20 M3 iteration 4: lab leak: the opening lab's Songhai played with flat human flavors (lab v19)
Class: opening army too small (Next 1): Songhai at turn 80 in op0/tr1 (10 games): about 2 cities, 3 land melee, might
29, while the ordinary AI civs in the same games average about 3.2 cities, 5.3 melee, might 56. Songhai's builds to
turn 60 hold 0-2 combat units per game (`CityStrategyAIProductionLog.csv`: worker, granary, monument, shrine, settler,
library, water mill, wonders).
Cause: Songhai is the human seat that the lab hands to the AI with autoplay. `CvFlavorManager::Init` (game start, still
human) sets every personality flavor to 5 for a human; `AI_Personality_Log.csv` shows FLAVOR_OFFENSE, EXPANSION,
MILITARY_TRAINING... all 5 for Songhai in all 6 tr1 games (the AI civs: their leader's values, e.g. Aztec offense 10).
The diplomacy AI reloads its leader personality when the seat turns AI (`CvDiplomacyAI` "Are we now an AI player?":
Songhai's War bias 7, conquest pursuit); the flavor manager has no such switch, so builds, research and policies ran on
flat 5s. Starting units are the same for every civ (1 melee each at turn 1). Not an AI failure: a lab leak (PLAN 7).
The siege/march battles also ran with flat flavors for the attacker (the tactsim reads FLAVOR_OFFENSE for its minimum
hp and loss threshold); their bars were met as measured, and they are left as they are (comparable baselines).
Change (lab only, no game behaviour change): `CvFlavorManager::LabReloadLeaderFlavors` and Lua
`Player:LabReloadLeaderFlavors()`: load the leader's flavors (XML, randomized as `Init` does for an AI, the map's
expansion/growth adjustment kept) and pass the difference on as a delta (as `ChangeLeader`), so strategy flavors stay.
Lab v19 calls it in opening/transfer mode right after autoplay starts (`FLAVORS,turn,civ,ok` line in Lua.log). Not
called by the game; battle sets unchanged.
Predicted: Songhai's own personality takes effect: more military in its builds by turn 40-60 and an army at turn 80
nearer the other AI civs' (about 5 melee, might about 50), stock and new AI alike (the new AI's changes are tactical).
If it still lags far behind, the cause is elsewhere (economy, its start) and the next step reads why.
Build and run: below.

## 2026-11-20 The transfer check, first version (PLAN 8a item 4; measurement only)
Asked for now by the project review session, relaying the user (11:35 AM ET), before more M3 work. The earlier
harness (`Civ5-CP-AnimatedLeaders` `tools/ab/Run-Games.ps1`) depends on DLL features `ai/v2` doesn't have (AIABTest.ini
set-up, seeds and result rows written by the DLL), so the minimum port is the lab's opening mode: `-Set transfer` plays
a normal game (the PC's set-up: Tiny, 4 civs, King, Quick) to turn 100 with the new AI on for Songhai (Askia,
aggressive) and stock for the others, and the AI logs are saved with it. Not ported: seeded maps (each game is a fresh
map) and the paired/flipped games; this informs, it doesn't gate (PLAN 8a). Reader: `siege/transfer_check.py`
(Songhai's state at turn 100, captures, and how often each lab fix's log line appears: "siege estimate ... TAKE",
"siege estimate NO", "capture attempt ... executed", "melee/ranged closing in on", "Not exploring with", "Started Army",
"preferred attack target"). Two lab fixes made on the way: `Get-SiegeLabResults.ps1` saved the AI logs only for runs
with battle results (moved to `Save-LaunchLogs`, called for every run), and `test/mock_run.lua`'s end checks had been
failing on every set since lab v16-17 (stale field counts) while printing DONE: fixed, all six sets pass by exit code.
Request: `2026-11-20-09-tr1` (tr1a-f, `ab-b196ee4`, either PC).
Result (6 games, BOX and TREEDESKTOP, all clean, about 5 minutes each; AI logs saved): **no lab fix fires in a real
game yet, because the new AI never brings an army to an enemy city.** Songhai (new AI) at turn 100: 1-4 cities, land
melee 2-5, ranged 0-2; no Songhai unit ever within 4 tiles of an enemy city (0 "siege estimate" lines in all 6), so 0
capture attempts, 0 "melee/ranged closing in". No city changed hands in any of the 10 games (op0 included). Wars:
tr1a Songhai declared on Korea on turn 102 (after the cut) after a WAR approach logged 130 times; tr1b WAR approach to
Greece from turn 39, never declared, was declared on by the Aztecs (61) and Greece (91), started no attack; tr1c-f no war.
Operations: settler escorts only, except tr1c turn 98: several `OP_CITY_ATTACK_LAND_SNEAKY` started with 5 units (Pikeman,
Warrior, Composite Bowman, a Great General) and each "Ended: NoUnits; Duration 0" the same turn: a new failure, to be
reproduced in a scenario before any fix (rule 3). The "preferred attack target" line (M2 it. 2's range) appears 150-211
times per game: targets are chosen, armies are not built or sent.
Reading: the M1/M2 fixes are untested in real play because the step before them fails: fielding an army (M3) and
deciding to use it (M4). That is the plan's own order; it says M3's opening lab is the right next step, with the
army's size and timing as its measure. The transfer check is repeated at each milestone close.

## 2026-11-20 M3: the opening lab, first version (lab v18; measurement only)
PLAN M3's second item. The siege lab add-in gets an opening mode (`-Set opening`): the game as the PC's menus set it up
(normal start, every civ, city-states, barbarians) is autoplayed to turn 80 (Quick); each turn one `OPEN` line per
major civ (cities, population, techs, era, gold and rate, science, policies, happiness; land melee, ranged, siege and
mounted units, naval, upgrade gold for the units that can upgrade, settlers, workers, wars, military might) and a
`CAPTURE` line per city taken. The installer, runner, queue and watchers are unchanged (a launch = one game). Mock
checked (`test/mock_run.lua opening`; the four battle sets still run clean). First request `2026-11-20-08-op0`
(op0a/c stock, op0b/d new AI on Songhai, build `ab-b196ee4`): does it run on the PCs, how long does a game take, what
the stock opening looks like. The M3 bar (user's game: war declared turn 41-50, a capital by turn 54 with about 6
melee and 4 Composite Bowmen, no bankruptcy, 2+ cities) is read against these lines once they exist.
Result (op0a, TREEDESKTOP, op0b, BOX; clean, about 2.5 minutes of play for 80 turns; 4 majors: Songhai, Greece,
Morocco, the Inca): it runs. No AI logs were saved (fixed since, above). Stock Songhai (op0a): 1 city until turn 79, land
melee 2/3/4/5 and ranged 0/0/1/1 at turns 40/50/60/80, never at war; far from the user's reference (6 melee + 4 CB, a
capital by 54). All four (80 turns): Songhai at turn 80, stock (op0a/c): 2-3 cities, melee 3-5, ranged 1; new AI
(op0b/d): 2-3 cities, melee 2-4, ranged 0-1, siege 0-1; never at war in any. The new AI changes nothing here (its
changes are tactical). Against the reference (about 6 melee and 4 CB, war by 41-50, a capital by 54) the army is a
third to a half the size and no war is declared.

## 2026-11-20 M3 iteration 3: a melee hit on a city we can take scores what it deals
Class: melee_only_vs_medium, takeable (script 6/6 by turn 5), new AI 1 of 6 (m3f2). Bar: 5 of 6 by turn 7.
Cause (m3f2a rep 1, game turns 55-60, `PlayerTacticalAILog.csv`): the estimate says TAKE, both gates let the attack
through, every Swordsman is "allowed: damage 15-17, received 30-34", but the city stays at 250: the attack's score is
dealt - taken (about -17; end of `ScoreAttackDamage`), so the tactsim picks moves without it.
Change (`0410152`, behind `IsNewAI()`): at the end of `ScoreAttackDamage`, for a melee attack on a city whose turn
estimate says TAKE, the damage taken is left out of the score (the hit point limit, "damage not worth it" and suicide
checks still apply before it). Only cities the army can take, so small_vs_strong (estimate NO) is unaffected.
Predicted (new AI, 6 battles each): melee_only_vs_medium from 1/6 to at least 4/6, most by turn 7, losses allowed.
Mixed armies may send melee in earlier: capture turns the same or earlier, units lost may rise a little (losses don't
count, 2026-11-18 ruling). No siege scenario's captures drop by more than 1 in 6 from m3f2 (71 of 90); small_vs_strong
unchanged. March and march3 keep the M2 bar; regress 8 of 8.
Build: `ab-0410152`. Run: queue request `2026-11-20-06-m3f3` (m3f3a-c siege, m3g3a-c march, m3h3a-c march3, m3x3a regress).
Result (all 10 launches clean): melee_only_vs_medium **6/6**, turns 5-8 (5 of 6 by turn 7: meets the M1 bar). Siege
72 of 90 (m3f2 71), but **large_vs_strong 3/6** (m3f2 6/6, turns 5-6) and march **d16 4/6** (6/6; under the M2 bar),
d8 5/6 and slower (10-13). Regress 8 of 8. Cause (m3f3a large_vs_strong rep 1, game turns 48-53): Spearmen hit the city
at 163 HP (7-15 dealt, 59-69 taken), so when the ranged units had it at 21 HP they were at 5-46 HP: every finishing hit
"refused (suicide)", the estimate turned NO. The exemption applied whenever the estimate said TAKE, not only to the
hits its own plan makes (it holds melee while the ranged units can do the work): melee spent early, as in section 5.
Decision: reverted in favour of 3b (two scenarios drop by 2-3 in 6). Learned: the score must follow the estimate's
plan, not just its verdict.

## 2026-11-20 M3 iteration 3b: the score exemption only for the hits the estimate's plan makes now
Class and cause: as iteration 3 (melee_only 1/6) plus its regression (large_vs_strong, march d16: melee spent early).
Change (`b196ee4`, behind `IsNewAI()`): the estimate records `bMeleeNow`: its plan has melee attack on turn 1 (the
city is taken this turn, or the melee grind because the ranged units alone can't bring it within their reach before
the limit). The score exemption (damage taken left out) applies only when the estimate says TAKE and `bMeleeNow`. The
log line adds "melee now 0/1".
Predicted: melee_only_vs_medium stays at least 5/6 by turn 7 (no ranged: always grind, bMeleeNow 1). large_vs_strong
back to at least 5/6, turns 5-6 (ranged suffice: melee hold, stock scoring until the city is low). March d16 and d8
back to 6/6 within the bar. Every other scenario within 1 in 6 of m3f2.
Build: `ab-b196ee4`. Run: queue request `2026-11-20-07-m3f4` (m3f4a-c siege, m3g4a-c march, m3h4a-c march3, m3x4a regress).
Result (all 10 launches clean, BOX and TREEDESKTOP): siege **76 of 90** (m3f2 71): melee_only_vs_medium **6/6**,
turns 5-7 (all within the M1 bar of 7); large_vs_strong 6/6, turns 5-7; no scenario drops; ref_cb4 a turn earlier (4);
losses 0.0-0.3 except small_vs_strong 1.3 (was 2.3). March 41 of 42 and march3 24 of 24; every scenario meets the M2 bar
(d8 5/6 and march3 d8 5/6 within it, the rest 6/6). Regress 8 of 8, a turn earlier.
Decision: **accepted**. With it, all 14 takeable siege scenarios meet the M1 bar (melee_only_vs_medium was set aside
at M1). Learned: the score has to follow the estimate's plan (hold melee while the ranged units can do the work).

## 2026-11-20 M3 iteration 2: the new AI's two "too early" gates ask the one estimate
Class: melee_only_vs_medium, takeable (script 6/6), new AI 0-1 of 6 (m3e1 0/6; one capture in m3e2b): the "never
attacks enough" class, set aside for M1. Two gates disagree and refuse (M1 it. 8): `ExecuteCaptureCityMoves`' "too
early" (expected damage 0 with five Swordsmen adjacent, because a costly melee hit counts as nothing) and
`ScoreAttackDamage`'s per-unit "too early" (one Swordsman is worn down before the city). Estimate v2 says TAKE for it
on battle turn 1 (turn 2-3).
Cause: the two gates (above); M3's first item is to make them one function.
Change (`7aa945f`, behind `IsNewAI()`): `CvTacticalAI::UpdateSiegeEstimates` keeps the turn's estimates
(computed at the start of `Update`, for the new AI or when logging; not saved). In `ExecuteCaptureCityMoves`, when the
units in reach can't finish the city this turn, the new AI attempts the capture if the estimate says TAKE, else logs
"too early for attacking <city> (siege estimate NO)". In `ScoreAttackDamage`, a melee attack on a city is "too
early" for the new AI only if the estimate says NO ("refused (too early, siege estimate NO)"). Scoring and the other
checks (hit point limit, damage not worth it, suicide) unchanged.
Predicted (new AI, 6 battles each): melee_only_vs_medium from 0-1/6 to at least 4/6 by turn 7 (the M1 bar), or, if not,
the log shows which later check refuses (expected: "damage not worth it"). small_vs_strong (estimate NO) stays 0/6
with no more units lost. Every other siege scenario within 1 in 6 of m3e1 (69 of 90), capture turns the same or
earlier. March and march3 keep the M2 bar (5 of 6 within 1.5x the script); regress 8 of 8.
Build: `ab-7aa945f`. Run: queue request `2026-11-20-05-m3f2` (m3f2a-c siege, m3g2a-c march, m3h2a-c march3, m3x2a regress).
Result (first launch, m3f2a, BOX, clean, 23 of 30): melee_only_vs_medium 0/2, 29% most damage (m3e1: 0). The log
(rep 1, game turns 55-60): estimate TAKE (turn 3-4) every turn, "attempting capture ... executed: 6 units in reach, 6
melee" every turn, and every Swordsman "allowed: city hp 250, damage 15-17, received 30-34" by `ScoreAttackDamage`, yet
the city stays at 250. Both gates are open; the attack is allowed but never chosen: its score is damage dealt minus
damage taken (`result->SetScore(0, bonus, dealt - taken)`, end of `ScoreAttackDamage`), -17, so the tactsim prefers
moves without it. This is the second block of M1 it. 8. Not "damage not worth it", as predicted: a score, not a refusal.
Full result (`2026-11-20-05-m3f2`, 10 launches on BOX and TREEDESKTOP, all clean): siege 71 of 90 (m3e1 69): no
scenario drops, losses the same or lower, capture turns the same or earlier; melee_only_vs_medium 1/6 (turn 11; was
0/6); small_vs_strong 0/6, 2.3 lost (2.2). March 42 of 42 and march3 24 of 24, every battle within the M2 bar (11 of
11 scenarios 6/6; 4b: 65 of 66). Regress 8 of 8 (2 each).
Decision: accepted (the plan's "one function" item: both gates now ask the same estimate; nothing got worse, the
march bar holds with the estimate driving attacks). The prediction for melee_only missed: what still holds it back is
the attack score, not a gate (above). That is iteration 3.

## 2026-11-20 M3 iteration 1: one siege estimate, logging only
Class: none yet (M3's first item, PLAN 8 M3): two functions judge "can this army take this city" and disagree (M1
it. 8). Reading them again: `ComputeTotalExpectedDamage` caps melee at 3 only for **unit** targets; for a city it drops
every melee hit that costs more than it deals (dealt < received, unless dealt x2 > received and the unit stays above
half health), and only counts units that can strike this turn. So a melee-only army facing a strong city counts 0,
which is the "expected max damage per turn 0 with five Swordsmen adjacent" of M1 it. 8. `ScoreAttackDamage` judges one
unit at a time ("too early" when the unit wears down before the city). Neither plays the siege over turns.
Change (`69b9d72`, logging only, not behind the switch, for both AIs): `TacticalAIHelpers::EstimateSiege(attacker,
city, range, max turns, out)` counts our land combat units within 4 tiles and plays the siege forward up to 13 turns:
ranged units shoot (never below 1 HP), melee attack strongest first while they survive the hit and take the city when
one hit covers what is left, idle melee heal 10, the city strikes the weakest melee unit and heals 20; travel from
distance and moves. `CvTacticalAI::LogSiegeEstimates`, called once a turn in `Update` after the target list, writes
"siege estimate for <city>: TAKE|NO, turn t, melee left m, city hp at end h; city hp, heal, strike; within 4: melee,
ranged; ranged damage per turn, melee damage per attack, received" (`PlayerTacticalAILog.csv`). Read-only, no random
numbers (damage without the random part), only what a human sees.
Predicted: no behaviour change: siege set new AI within 1 in 6 per scenario of m2gb (69 of 90). The estimate on battle
turn 1 says TAKE for the scenarios the script or the new AI takes (11 of 15 incl. melee_only_vs_medium, which the
script takes 6/6) and NO for small_vs_strong, medium_vs_strong (2/6 now: either) and ranged_only_vs_weak (no melee):
agreement in at least 13 of 15. March: NO while the army is still home (beyond 4 tiles: no line), TAKE on arrival.
Build: `ab-69b9d72`. Run: queue request `2026-11-20-02-m3e1` (m3e1a/b/c siege new AI, m3n1a march, m3c1a march3,
m3s1a siege stock).
Result (v1, m3e1a, BOX, 23 of 30, clean): first-turn estimate agrees with the outcome in 12 of 15 scenarios; it says NO
for small_vs_medium, medium_vs_medium and large_vs_strong, all captured. Two model errors, from `turns.csv` and the
estimate's own per-turn hp: (1) the garrison: while it lives each hit is weak (small_vs_medium: 37 a turn, net 17 after
the 20 heal), once it dies on battle turn 3 the damage nearly triples (66, net 46); v1 kept the garrison-alive figures.
(2) v1 sent melee in from turn 1 (Spearmen deal 13, take 38) so they were spent on a full-health city; the AI and the
script hold them until the city is low. Estimate v2 (`a0f7eff`, built as `ab-7dd3e85` after a merge): tracks garrison
hp and switches to the bare-city figures once it dies; melee hold and heal while the ranged units alone can bring the
city within the melee's reach before the limit, otherwise grind; taken when the melee able to attack this turn cover
what is left. Run `2026-11-20-04-m3e2b` (m3e2b, m3s2b, m3n2b, m3c2b). m3e1b/c still run on v1: the no-change check.
v1, march (m3n1a 14/14, m3c1a 8/8, TREEDESKTOP, clean): the first estimate comes with the first units near the city and
says NO (as predicted); it turns TAKE as the army arrives, 0-4 battle turns before the capture, and the capture turn it
then predicts is the real one or up to 3 turns early (22 of 22 battles).
No-change check met: m3e1a/b/c (BOX, clean) 69 of 90, as m2gb (69); every scenario within 1 in 6 except
medium_vs_strong 4/6 (m2gb 2/6, earlier 1-2/6: up, not down). v1 over the 6 battles: agrees in 11 of 15 (the three
above plus medium_vs_strong 4/6 captured, estimate NO): misses the bar of 13.
v2 (`ab-7dd3e85`): m3e2b (BOX, clean, 24 of 30) agrees in 13 of 15; the two "misses" are TAKE for medium_vs_strong
(1/2 here, 4/6 in m3e1) and melee_only_vs_medium (1/2 here, the first capture on this build; script 6/6), so against
the best of the AI and the script it is right in 15 of 15. Bar met. Its turns are optimistic (predicted capture 0-3
turns early). March m3n2b (14/14): TAKE comes 2-7 turns before the capture, sometimes with part of the army; the
predicted turn is up to 4 early (one d10 battle fell on turn 25, logging-only build: noise, but a miss of that
scenario's turn-12 bar). The risk for iteration 2 is a partial army attacking early on the march.
Stock attacker, v2 (m3s2b, TREEDESKTOP, clean): agrees in 10 of 15; all five misses are TAKE where stock falls short:
melee_only (0/2), medium_vs_medium_rough (0/2), large_vs_strong (1/2) and siege_train_vs_strong (1/2) are stock's known
failures that the script takes 5-6 of 6 and M1 fixed for the new AI, so there the estimate is right. medium_vs_strong
(1/2) is an estimate error: neither stock nor the script took it in the baseline. A TAKE the AI doesn't deliver mostly
marks an AI failure.

## M2 (closed 2026-11-20): what we learned
Entries archived in `LOG-archive.md`. Signed off by the user on 2026-11-20 (12:35 PM ET). Bar (provisional, then accepted
with the sign-off): in each march scenario, capture in at least 5 of 6 within 1.5 times the scripted attacker's median
capture turn. Met in 11 of 11 on `ab-9238090` (65 of 66; stock 0 of 66) and still on `ab-b196ee4` (41/42 + 24/24).
- Accepted (behind `IsNewAI()`): **it. 2** `GetArmyPathsFromCity`: army path budget x2 (the stock cap is the land trade
  route range, 10 on every map size, so a city 12 tiles away was never a target). **It. 3**
  `IsSpareRangedForOptionalSlot`: a spare land ranged unit may fill an optional slot, so a classical army that can't fill
  its third melee slot still leaves recruiting (user ruling "go with what it has"). **It. 4b** `PlotReinforcementMoves`:
  idle ranged units close in to firing range, but only once our melee are within 3 tiles (a lone bowman ahead of the
  army stalled it in 4).
- Causes found: stock never leaves home at 10+ tiles (no path within the budget); at 8 tiles the army waits in
  recruiting for a third melee unit no one builds (`HaveEnoughUnits` allows 0 missing, though the op started allowing
  1-3); greedy slot filling can strand a Spearman (ATTACK slots need ATTACK units; not fixed). Army paths only cross
  seen tiles (the lab now shows the corridor).
- Lab: march, march3 (a Swordsman fills the formation) and rough sets; era-grouped battles; a second lab PC
  (TREEDESKTOP) passed the two-PC check; the run queue (watcher v3, v4 since 2026-11-20).
- Transfer check at the close (`2026-11-20-09-tr1`, see its entry): in 6 normal games the new AI never brings an army
  to an enemy city, so no M1/M2 fix shows in real play yet; the step before (fielding and using an army) is M3/M4.

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
