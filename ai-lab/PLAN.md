# Plan: a competent Civ V AI, built by short test loops

This is the working plan for the session that implements the AI upgrade. It replaces the earlier
experiment (Nov 2–7, 2026), which was stopped without converging. Read it top to bottom once, then work
from "The loop" and "Milestones". Update it when a milestone closes; don't rewrite goals in the middle of one.

## 1. Goal

The user plays Civilization V multiplayer with a friend, on the Community Patch (not Vox Populi), using
their own mod pack. They want AI opponents that are **competent**:

- a real challenge for two non-expert players, without any advantage a human doesn't have (the pack already
  evens out the King difficulty; keep it that way);
- able to play **different styles depending on the civ**: correct build orders, tech and policy choices,
  diplomacy, and military tactics for that style;
- in particular, an aggressive civ must be able to **take cities** and move toward conquest. This is where
  the earlier experiment failed.

It does not need to beat an expert. "Done" is: every milestone below has met its bar, and the user and
their friend play full games against it and find it a challenge with no obvious blunders.

Two earlier targets are **dropped as goals**: "60% strength share against the stock AI" and "one of our
three styles wins 90% of games". Strength share survives only as a regression check in milestone 7.

## 2. Why the last attempt failed (so this one doesn't repeat it)

1. **Everything was judged through whole 150-turn games.** A city capture happened in 1–4 of 10 games, a
   game took 10–17 minutes, a verdict took half a day, and 1 in 10 versus 3 in 10 is not a difference you
   can see. Tactical fixes were never tested on a battle.
2. **The new logic was a layer of plans and vetoes on top of the stock AI** (about 13,000 lines, 109
   switches). The stock tactical AI still fought every battle and kept undoing the plan; each case was
   answered with another special case citing one game.
3. **Many things changed at once**, goals moved almost daily, and five sessions shared the same files.
4. **The headline number was never measured.**

The rules in section 6 exist because of these four points.

## 3. What exists today

| Thing | Where | State |
|---|---|---|
| The pack's DLL source | repo `treezeegaga/Community-Patch-DLL`, branch `mp-leaderheads` | Community Patch 5.4.8 plus the leader-screen patch. **This is the stock baseline.** `master` is 20 commits newer than 5.4.8 and does not match the pack's database |
| Siege lab | same repo, branch `claude/bold-feynman-cgnhjs`, folder `ai-lab/siege/` | Works in the game. Read its `README.md` |
| First stock results | `ai-lab/siege/results/stock548-*` and `diag1-*` | 30 battles, 1 capture; a 4-battle rerun disagrees with it (section 5) |
| One-command runner | `ai-lab/siege/Run-SiegeLab.ps1` | Written and tested on a short run by the PC runner session |
| PC runner | a Claude session on the user's PC "box", titled "Civ V siege lab: PC runner (box)" | Runs commands on request; kept on a cheap model |
| Earlier experiment's code | branches `ai/war`, `ai/economy`, `ai/diplomacy`, `ai/personality`, `claude/ab-c1` | **Reference only.** Do not merge or build on them |
| Earlier notebook and tools | repo `treezeegaga/Civ5-CP-AnimatedLeaders`, branch `claude/sleepy-hopper-x77moh`, folder `tools/ab/` | `RESULTS.md` (root causes found), `AGENTS.md` (the user's rulings), `HUMAN_GAME.md` (how the user took a capital by turn 54), `STRIKE_PLANS_RESEARCH.md` (per-civ rush plans from human guides), `REVIEW_CHECKLIST.md`, `Run-ABTest.ps1` (installs a DLL build into the pack and restores it), `Run-Games.ps1` (full-game runner) |

Facts worth knowing before touching code:

- The DLL builds with Visual C++ 2008: **no C++11**. Ship the MSVC build; the clang build fails to load.
- The fork's `ai/war` branch has a build workflow that runs on pushes to `ai/**` branches and publishes
  each build as a GitHub release named `ab-<commit>`. Take that workflow file for the new branch.
- This DLL shows engine assertions as a blocking dialog. The runner screenshots and dismisses them; treat
  every new assertion text as a bug to understand.
- The game is 32-bit and crashes near 1.8 GB; long games sit close to that.
- The game's own AI logs (tactical, military, operational, diplomacy) are written when `config.ini` has
  `LoggingEnabled = 1` and `AILog = 1`. They say in words why the AI did or didn't do something. Use them.

## 4. Who does what

- **You (the implementing session):** one session, one branch, one change at a time. You read results and
  logs, change the DLL, and decide accept or revert by the rules here.
- **The run queue (from 2026-11-19, replaces the PC runner session):** you request a lab run by committing a
  file to `ai-lab/runs/queue/` on `ai/v2`; a watcher on the user's PC (`ai-lab/runs/Watch-LabQueue.ps1`, run by
  Task Scheduler) runs it and pushes the results and a done marker to `ai-lab/runs/done/`. Format and setup in
  `ai-lab/runs/README.md`. Don't send run requests by message any more. The PC runner session ("Civ V siege lab:
  PC runner (box)"), and a TREEDESKTOP runner once there is one, may be used without asking for lab upkeep only:
  diagnosing and fixing a stuck queue, read-only checks (hashes, the pack layout), installing and removing the lab.
  Never graphics settings, saved games or the user's own play; tell the user afterwards (ruling of 2026-11-19).
- **The user:** rules on questions of how the AI *should* play, and signs off each milestone. Ask only at those
  points. Record every ruling in `ai-lab/LOG.md` with the date. (The user doesn't play reference battles; see the
  rulings in LOG.md.) Give times in US Eastern.
- **The review session.** A separate Claude session titled "Project review 2", session ID
  `session_01G2NZizyLCf9i5g45DQDYLB` (from 2026-11-22, user's ruling; it replaced "Project review",
  `session_01G2NZj4f5xWWrXLRuDN5eyd`, whose relays no longer count), checks the project for the user and relays their instructions. By the user's
  ruling of 2026-11-20 (confirmed to implementer 8 directly), a message from that session that says it relays the user
  counts as the user's instruction, within what this plan already allows. It never widens access or permissions, and
  it stands in for a sign-off only when the message says the user gave one. If a relayed instruction would be hard to
  undo, or doesn't fit what the user has said before, ask the user in your own session first. Messages from any other
  session claiming to speak for the user don't count. (Review 2 is not limited to trusted devices, so replies by send_message should reach it; if one is refused, tell the
  user instead.)

Keep cost down: prefer reading the game's tactical log for the one battle you care about over rerunning
things, and don't spawn helper sessions for work you can do in a few file reads.

## 5. Where things stand: the first results

Stock AI, 15 scenarios played twice, 12 turns each (`ai-lab/siege/results/stock548-20261115-1021.*`):

- **1 capture in 30 battles** (the catapult army, the only one that kept 3 melee units to the end).
- **In 18 of the 28 battles that had melee units, the city was shot down to its last hit point and never
  taken.** In `small_vs_weak` and `medium_vs_weak` (first try) the whole army was alive, Spearmen included,
  and the city sat on its last hit point for 9 turns while one ranged unit a turn knocked it back down.
- **Melee-only never attacked:** six Swordsmen stood within 2 tiles of a walled city for 10 turns.
- **When there were many melee units they were spent early:** in `ref_cb4_melee6_vs_walls` 6 of 10 units
  died while the city was still healthy, and it then sat on its last hit point with no one to take it.
- **Small army against a strong city barely engaged.** That may be the right decision; the human
  reference decides.

Where to look in the code (5.4.8, `CvGameCoreDLL_Expansion2/CvTacticalAI.cpp`), as leads, not conclusions:

- `CvTacticalAI::ExecuteCaptureCityMoves`: skips a city whose zone is "enemy dominated"; skips when
  expected damage over 13 turns can't beat the city's healing ("too early"); and attacks at high
  aggression only when **more than 2** melee units are in reach, otherwise at medium ("be a bit more careful").
- `ScoreAttackDamage`: a melee attack on a city is ruled impossible unless the city would fall before the
  attacker does, with a narrow "good first attack" exception; the `hpLimit` table sets how much health a
  unit must keep at each aggression level.
- `CvTacticalAnalysisMap::CalculateMilitaryStrengths`: counts the city's own strength as enemy ranged
  strength when judging who dominates the zone.

Three caveats, which make M0 unfinished:

- **Income drifted during that run** (unit upkeep rises with the turn count; the attacker went from 0 to
  -62 gold a turn). The lab now levels income at +10 a turn (commit `f05cbd2`), and a later short run
  (`diag1`) confirms it: `AttackerGoldRate` reads 10 in every battle.
- **The same scenario does not behave the same way twice.** In `diag1` (4 battles, stock AI) the
  `small_vs_weak` and `medium_vs_weak` armies turned round on turn 1 and walked home without firing a
  shot (average distance to the city rose from 4 to 12 tiles), where in the first run the same armies
  shot the city down to its last hit point. One battle of the first run (`medium_vs_weak`, second try)
  showed the same walk-away. Until it is known what decides between "engage" and "walk home", no result
  from the lab can be trusted as a baseline. It may be something the lab leaves behind between battles or
  fails to set up (what the AI thinks its war goal, target or army is), or a real property of the stock AI.
- **No AI logs have been saved yet.** The results reader looks for files with "Tactical" in the name in the
  game's `Logs` folder and found none in `diag1`, although the runner was asked to switch `AILog` on.
  Find out what the game actually writes there (file names, and whether `AILog = 1` took effect) and fix
  the reader. Without the tactical log, step 3 of the loop is guesswork.

## 6. Rules

1. **Test at the lowest rung that exercises the change.** A change to how units fight is judged in the
   siege lab, never in full games. Move up a rung only when the rung below meets its bar.
2. **One change per iteration.** One idea, one build, one run, one decision.
3. **Every failure becomes a scenario before it becomes a fix.** If a battle or a game shows a new way to
   fail, add a scenario (or a lab) that reproduces it, see it fail, then fix it. No fix may cite a single
   game as its reason.
4. **Fix causes inside the stock functions.** No parallel planners and no veto layers that override stock
   decisions from outside. If two places estimate the same thing (for example "can this army take this
   city"), make them one function.
5. **One switch, not many.** All new behaviour sits behind a single per-player test "uses the new AI"
   (milestone 1 adds it), so stock and new can be compared in the same build and mixed in one game. No
   per-idea flags. An accepted change stays behind the switch; a rejected change is deleted, not disabled.
6. **Bars are set before the run, from human play and the stock baseline,** and are frozen for the
   milestone. The user's rulings on how the AI should play are applied at milestone boundaries.
7. **No cheating and multiplayer-safe.** Only information a human could see; no state that differs between
   machines; no randomness outside the game's seeded generator; every new saved field is serialized with a
   version. Go through `REVIEW_CHECKLIST.md` from the earlier experiment before a build goes to the PC.
8. **The PC is the user's gaming machine.** Restore the stock pack DLL and remove the lab after every run.
   Never leave Civ V running. Don't touch graphics settings or saved games.
9. **Stop and write to the user** when: three iterations in a row on the same failure move nothing; a bar
   looks wrong or unreachable; a ruling is needed; or a change would break rule 4 or 7 to work. "Stop" means stop
   that line of work and write the question, never stop working (rules 10-12).
10. **Provisional rulings** (user's rule, 2026-11-20). When a bar or a ruling is needed and the user is away, adopt
   your own proposed version provisionally, mark it "provisional" in LOG.md (and in this plan, where it is a bar),
   and keep working on it. The user confirms or changes it later; if they change it, re-judge what depended on it.
11. **When blocked on the user, work down the list in section 7a** instead of waiting.
12. **Never go idle without a wake-up** (user's rule, 2026-11-20). Before ending any turn, leave a timed background
   wait running (`ai-lab/runs/wait-for-results.sh`, at most 110 minutes), even when the queue is empty, so the
   session is re-invoked within two hours.

## 7. The loop

Each iteration is one entry in `ai-lab/LOG.md` (template in section 10) and follows these steps.

1. **Observe.** Take the latest lab results for the current build. Sort battles into failure classes by
   what the turn data shows (for the siege lab: never approached; approached but never attacked; attacked
   but too little damage; city on its last hit point and not taken, with melee alive; same, with no melee
   left; captured). Count them.
2. **Pick the largest class** that isn't a correct decision by the human reference.
3. **Find the cause.** Open the game's tactical log for one battle of that class and find the line where
   the AI decided not to act; then the code that wrote it. If the log doesn't say, add a log line (logging
   only, no behaviour change), rebuild, rerun that one scenario.
4. **Predict.** Write down the change, which scenarios it should move and by how much, and which must not
   get worse. If you can't predict, you don't understand the cause yet: go back to step 3.
5. **Change** the stock function, behind the one switch. Keep the diff small enough to review in one read.
6. **Review and build.** Checklist, push, wait for the build, note the release name.
7. **Run** the affected scenarios first (a few minutes), then the whole set. Ask the runner; don't wait
   idle, read code for the next class meanwhile.
   **How to wait for runs (user's rule, 2026-11-19):** scheduled check-ins (`send_later`, scheduled tasks) are
   refused for this session; don't rely on them. Start `ai-lab/runs/wait-for-results.sh` as a background command
   (Bash `run_in_background`, timeout about 7,000,000 ms). It polls `origin/ai/v2` every 2 minutes and exits when a
   new done marker or launch result lands, or after about 110 minutes. When it exits: pull, read what arrived,
   record it, queue what's next, and start it again. Keep one running at all times, queue empty or not (rule 12): a
   timeout with nothing new is the cue to take the next item from section 7a.
8. **Decide** by the rule below. Accept: record the new table as the baseline. Reject: revert the commit,
   and write one line on what the result taught you. Then bring `ai-lab/CODEMAP.md` up to date: any gate or limit
   found, log line added, function changed (accepted) or attempt reverted, script changed.

**Decision rule for lab runs.** Play each scenario at least 6 times per build (three launches of the set
at 2 repeats). A change is accepted when the scenarios it targeted move from "mostly fails" to "mostly
succeeds" (for captures: from at most 1 in 6 to at least 4 in 6), and no other scenario's capture count
drops by more than 1 in 6 or its losses rise by more than a third. An effect too small to see in 6 battles
is not what this stage is after; don't chase it with more repeats. When repeats of the *same build*
disagree wildly, the lab has a leak: fix the lab first.

## 7a. Work that never needs the user (when blocked on the user, work down this list)

In this order, unless an item is already in hand or can't move:
1. **The next iteration on the largest failing class** of the current milestone, under the provisional bar (rule 10).
2. **CODEMAP upkeep**: anything learned but not yet in `ai-lab/CODEMAP.md`; the out-of-date `siege/test/mock_run.lua`.
3. **The transfer-check harness** (section 8a item 4): port the minimum of the earlier full-game harness (seeded maps,
   seat assignment, results file) to `ai/v2`, then run it for the closed milestones.
4. **The next milestone's lab**: for M3, the single "can this army take this city" estimate (logging first, checked
   against the lab's results) and the opening lab (a normal start cut at turn 60-80).
5. **Lab hygiene**: known stock bugs worth a scenario (CODEMAP "known, not fixed"), lab leaks, faster runs.

**Standing run backlog (user via review session, 2026-11-21 10:05 AM ET): never let the lab PCs sit idle.** Before going
quiet for any reason (waiting on a result, on the user, the end of an iteration at night), queue enough runs from this
list to fill several hours on both PCs; they need no decision. Top the queue up from it; tick off what ran.
- (a) Other warmonger civs: the current accepted build on seeds 101-606 with `newAI: defender` (the new AI for every
  major but the Songhai seat: Greece on every seed, Aztecs, England, Netherlands, Denmark, Japan, Persia, Ethiopia,
  Russia, Indonesia, Spain, Mongolia) and the same with `newAI: none` (stock), to see whether the plan's selection
  rule (`IsWarmonger`) and the plan itself work beyond Songhai. First queued: `2026-11-21-14-civs` (build `ab-006a7f0`).
- (b) The holdout set (below): stock and new AI at the current accepted build, now and before M3 closes.
- (c) Repeat (a)/(b) after every accepted iteration.
- (d) Ran 2026-11-21: `14-civs`, `15-holdout` (LOG backlog entry). Ran `16-civho`, `17-long`; ran `18-long2`, `19-tune2`;
  queued `20-tune2more` (seeds 1313-1818 to turn 100, and other civs on them). Lab gap to fix: a defeated Songhai seat stalls the run (civN101).

**Measuring set (user, 2026-11-22): seeds 1919-6666, step 101 (48 maps).** The headline figures come from these (new AI
and stock on the same maps, captures and the state at turn 100); never used to choose or tune a change.
**Holdout set (2026-11-21): seeds 707, 808, 909, 1010, 1111, 1212.** Never used to choose or tune a change; run only as
a check (stock and new AI), now as a baseline and again before M3 closes. If it does clearly worse than the tuning set
(101-606), LOG.md says so plainly. Tuning set: seeds 101, 202, 303, 404, 505, 606, and from 2026-11-21 12:35 PM ET also 1313, 1414, 1515, 1616, 1717, 1818
(widened because a failure class showed on the holdout only; the holdout itself is never used to choose).

Anything that changes the user's PC beyond the lab (installing the setup switch in the pack, graphics, saves) or
needs the user at the PC (the multiplayer smoke test) is not on this list.

## 8. Milestones

Each milestone has a question, what to build, how it is measured, and the bar. Bars marked "set at M0" or
"proposed" are fixed with the user before work on that milestone starts.

### M0. Trust the lab

- Get the game's AI logs saved with every run (third caveat in section 5).
- Explain the "engage or walk home" split (second caveat in section 5) from those logs, and fix its cause
  if it is the lab's. Then show that repeats of the same build agree: the full set three times over, with
  each scenario landing in the same failure class in at least 5 of its 6 battles.
- **Human reference:** the user plays `ref_cb4_melee6_vs_walls`, `medium_vs_medium`, `small_vs_medium`,
  `large_vs_strong` and `melee_only_vs_medium` once each (`Install-SiegeLab.ps1 -Human -Only ... -Repeats 1`).
  Their results mark each scenario "takeable" or "not takeable", and how many turns and losses it took.
- Add scenarios the reference suggests are missing (for example a capital with a Palace, which the lab
  currently never stages).
- Write `ai-lab/BASELINE.md`: one row per scenario with the stock result and the human result.

Bar: the lab runs clean, repeats agree, and the user has agreed the "takeable" list.

**Closed 2026-11-18** (signed off by the user; the reference is the scripted attacker, see LOG.md rulings).

### M1. The assault: an army at the walls takes the city

- Base branch: create `ai/v2` from `mp-leaderheads`; bring over `ai-lab/` and the build workflow.
- Plumbing first, as its own iteration with no behaviour change: the per-player "uses the new AI" switch,
  saved with the game, settable from Lua, and a lab option to choose it for the attacker and the defender
  separately. Check that the stock results are unchanged with it off.
- Then run the loop on the failure classes in section 5, largest first.
- Make the "can this army take this city" estimate one function, checked against the lab's own results:
  it should say yes for scenarios the lab shows are won and no for those that are lost.

Bar (proposed; amended 2026-11-18 by the user's rulings in LOG.md: the reference is the scripted attacker or
the stock AI, whichever does better, and losses don't count): in scenarios marked takeable, capture in at least
5 of 6 battles, within 1.5 times the reference's turns. Losing units is acceptable if the city falls. New AI as
defender must not make stock attackers *more* successful than the stock defender does.

**Closed 2026-11-19** (signed off by the user): 13 of 14 takeable scenarios meet the bar on `ab-78d5e8a`;
melee_only_vs_medium set aside by the user; defender part met. The "one estimate" item is moved to M4.

### M2. The march: an army at home arrives ready to fight

- Use the `march` scenario set (army starts at its home city, 12 tiles away). Extend it with a longer
  distance and with rough terrain.
- The stock operation delivers the army to within 3 tiles and then dissolves it; units then drift home to
  heal or get recruited elsewhere. The earlier notebook lists these hand-over failures. Fix them where
  they occur.

Bar (adopted 2026-11-20 under rule 10, confirmed by the user's sign-off): in each march scenario, capture in at
least 5 of 6 battles within 1.5 times the scripted attacker's median capture turn. (Proposed first: 80% of the army
within 3 tiles within 2 turns of each other, then capture at the M1 rate; replaced because the script itself misses
the "together" part; LOG.md M2 iteration 3.)

**Closed 2026-11-20** (signed off by the user): 11 of 11 march scenarios meet the bar on `ab-9238090` (65 of 66) and on
`ab-b196ee4`. Transfer check at the close: the fixes don't show in real games yet (no army reaches an enemy city).

### M3. The army: an honest economy fields that army on time

- First (moved here from M4 by the user, 2026-11-19): make "can this army take this city" one function. Today
  `ExecuteCaptureCityMoves` (`ComputeTotalExpectedDamage`: at most 3 melee counted, wounded dropped, 13-turn rule) and
  `ScoreAttackDamage` (per unit "too early") disagree; see LOG.md M1 iteration 8. Check it against the lab's results
  (yes for scenarios won, no for those lost). M3 uses it to say how big an army must be; M4 uses it for target choice.
- Build an **opening lab**: a normal game start, one civ's plan, cut at turn 60–80 (Quick pace). It
  measures by which turn the civ has the army that M1 showed is sufficient, with the upgrade gold, and
  what its economy looks like at that turn. Minutes per game, no fighting needed.
- Put plans in **data**: one database table with a row per civ (style, opening builds, tech path, policy
  order, strike unit and timing, target victory) and one executor in the DLL that follows a row. Civs
  without their own row get the default row for their style. No per-civ C++.
- Seed the rows from `STRIKE_PLANS_RESEARCH.md` and the user's own Aztec opening in `HUMAN_GAME.md`.

Bar (set with the user at the start of M3): for the early-rush row, the reference is the user's game:
war declared around turn 41–50 and a capital taken by about turn 60 on Quick (the user's own Aztec game took one by
turn 54, but with the Jaguar's early strength; ruling of 2026-11-20, 11 PM ET), with about 6 melee and 4
Composite Bowmen. The economy must not collapse to get there (no bankruptcy, at least 2 cities).

### M4. The decision: who to attack, when, and when to stop

- Use M3's single "can this army take this city" estimate for target choice.
- A **decision lab** from saved mid-game positions (saves must load across builds: version every new
  saved field). It asks: does the civ pick the target M1's estimate says it can take, declare when the
  army is ready and in place, stay on that target, and make peace when the war is no longer winnable.
- Then the first end-to-end check, in full games cut at turn 100: an aggressive civ against ordinary
  neighbours.

Bar (proposed): in end-to-end games, the aggressive civ takes at least one city by turn 80 in at least
half the games, and holds what it takes.

### M5. The other styles

- Rows and benchmarks for the user's styles: greedy wide (expand, then grow and trade), greedy tall (3–4
  cities, tech and policy), balanced, aggressive early, aggressive late.
- Measured in the opening lab and in games cut at turn 100–150 against the stock AI in the same seat:
  cities, population, techs and gold at fixed turns, and for defenders, whether they hold against an M1–M4
  aggressor in proportion to what they invested in defence.

Bar (set with the user): each style beats the stock AI in its own seat on that style's own measures, and
none is helpless against the others.

### M6. A lively world: diplomacy toward humans

This was the user's original complaint: in multiplayer the AI rarely proposes trades or declares war.

- Measure events per civ per 10 turns toward human players and toward other AIs: trade proposals,
  declarations, denouncements, peace offers. Start by measuring stock; the earlier notebook found deal
  counts toward AIs were already reasonable and that the stock "victory rivalry" logic only starts late.
- Fix what is specific to human players and to multiplayer first.

Bar (set with the user after the stock measurement).

### M7. Whole games

- Port the minimum of the earlier full-game harness to `ai/v2`: seeded maps, seat assignment, the results
  file, paired games with the groups swapped. Use full games for **regression and integration only**:
  nothing from M1–M6 got worse, turn times are acceptable (at most 1.25 times stock), no crashes or
  assertions, strength share against stock is not below 50%.
- Then the real test: the user and their friend play. Keep a blunder log; each blunder becomes a scenario
  in the right lab (rule 3) and goes back through the loop.

## 8a. Additions of 2026-11-19 (user), in this order, with M2 carried on between them one thing at a time

1. **Second lab PC** (TREEDESKTOP): run queue claims, setup steps, the two-PC check (`ai-lab/runs/README.md`).
   Finish it first.
2. **Regression scenario set** in the siege lab: a capital (with its Palace), a coastal city, a medieval siege and an
   artillery-era siege. Baseline them on the stock AI and on the current build (6 battles each), and keep them in
   BASELINE.md. "Units lost" stays a reported column in every results table (losses don't count against the bars,
   per the 2026-11-18 ruling, but they are shown).
3. **The new-AI switch at game setup**: settable when a game is set up (not only from the lab's Lua), so the user can
   turn it on for chosen AI players in their own games, multiplayer included; and a 30-turn LAN multiplayer smoke test
   on two PCs, written up for the user to run (desyncs, assertions, crashes, turn times).
4. **Transfer check at each milestone close** (informs, does not gate): port the minimum of the earlier full-game
   harness (seeded maps, seat assignment, results file) to `ai/v2`; at each milestone close run 6 seeded games to turn
   100 with the new AI on for one aggressive civ, split across both lab PCs; record captures, assertions, and whether
   each lab fix shows up in the game's logs (e.g. M1 iteration 7's "melee closing in on" lines). Done first for M1
   (closed) once built, then at every later close.

## 9. First session checklist

1. Read `ai-lab/siege/README.md`, then this plan's sections 5–7.
2. Look at `ai-lab/siege/results/diag1-*` next to `stock548-*`: the same scenarios, opposite behaviour.
   Get the AI logs saved (ask the runner what is in the game's `Logs` folder after a run), rerun
   `small_vs_weak` alone a few times, and find what decides between engaging and walking home.
3. Finish M0: repeats agree, the user plays the human reference battles, write `BASELINE.md`. Only then
   is "why don't the Spearmen take a city on its last hit point" worth chasing.
4. Create `ai/v2` from `mp-leaderheads`, carry over `ai-lab/` and the workflow, confirm a build of the
   untouched branch installs on the PC and reproduces the stock baseline.
5. Start M1 with the plumbing iteration.

## 10. Records

`ai-lab/LOG.md`, newest first, one entry per iteration:

```
## <date> M<n> iteration <k>: <short name>
Class: <failure class and its count, e.g. "last hit not taken, melee alive: 11 of 30">
Cause: <the log line and the function that refused, in one or two sentences>
Change: <what, where; commit>
Predicted: <scenarios that should move, and to what; scenarios that must not move>
Result: <build, runs, the counts>
Decision: accepted | reverted, and one line on what was learned
```

`ai-lab/BASELINE.md`: the current table per scenario (stock, human, current build).
User rulings: dated lines at the top of `LOG.md`.
`ai-lab/CODEMAP.md`: the game-code functions the project depends on (what each decides, the gates and limits found,
the log lines each writes) and the structure of the lab and watcher scripts. Kept current at every iteration's end.
**At a milestone's close** (user's rule, 2026-11-19): move that milestone's iteration and process entries from
`LOG.md` to `ai-lab/LOG-archive.md` (newest first, unchanged), and leave in `LOG.md` a short "M<n>: what we learned"
summary (what was accepted, the causes found, what failed and why, the numbers that closed it). Rulings stay in
`LOG.md`.

## 11. Known risks

- A fixed assault may still need a bigger army than an honest economy fields by the user's timings. M1's
  results table is what tells M3 how big the army must be; if the two can't meet, raise it with the user
  rather than adding advantages.
- The lab stages battles on flat open ground with a small empire behind each side. Results that depend on
  terrain, supply or a real economy need their own scenarios or the higher rungs.
- Click-through automation on the PC is fragile (fixed window size and positions). If it breaks, the
  runner fixes the script; don't work around it by changing the user's settings.
- The second PC ("TREEDESKTOP") is being set up as a second lab runner (user's request, 2026-11-19; LOG.md rulings).
  Its results count only after the two-PC check in `ai-lab/runs/README.md` passes.

## 12. Handover between implementing sessions

A session's context fills up over a long project; a fresh session that starts from the written record is cheaper
and loses less than one that runs on a compacted memory. So:

**When:** at each milestone sign-off by the user, or as soon as your context passes 650,000 tokens, whichever
comes first (user's rule, 2026-11-19; it was 300,000). Check with `get_session` on your own session
(`external_metadata.context_usage.used_tokens`), at least at the end of every iteration. Finish or revert the iteration in hand first; never hand over with an unexplained change on the
branch.

**What:**
1. Write a `## Current state (<date, time ET>)` entry at the top of `ai-lab/LOG.md`, above the rulings, replacing
   the previous one: milestone and iteration in hand; the newest DLL build and what it contains; the baseline in
   use; queued runs not yet done and results not yet read; open questions for the user; anything the user must do;
   and a numbered "Next" list. Everything the successor needs that isn't already in PLAN.md, BASELINE.md or the
   log entries. Make sure `ai-lab/CODEMAP.md` is current. Commit and push it.
2. Create the successor with `create_session`: model `claude-opus-5-5`, title `Civ V AI upgrade: implementer <n>`
   (n one more than yours; you are 1 if your title has no number), tag `civ5-ai-lab`, source this repository at
   `ai/v2`, `outcome_branch` `ai/v2`, and exactly the prompt below.
3. Tell the user its name in one short message, and stop: no further work, no further runs, no messages to the
   successor.

**The successor, at its start** (user's rule, 2026-11-19): once it has read the current state and told the user
what it will do first, it archives its predecessor session (`archive_session`), provided the predecessor has
stopped (its last turn has ended), has pushed everything (its handover commit is on `origin/ai/v2` and nothing
newer is pending from it), and has no scheduled tasks bound to it (`list_triggers`: none with its session id as
`persistent_session_id`, and none it set for itself still pending). Archive only, never delete. If any condition
fails, leave it and tell the user why. If archiving is refused, tell the user the session's name so they can
archive it themselves.

The successor's prompt, word for word:

> You are the implementing session for the Civ V AI project in the repository treezeegaga/Community-Patch-DLL.
> Work only on the branch ai/v2: check it out from origin, commit there, push only there. Read ai-lab/PLAN.md
> in full first, then ai-lab/CODEMAP.md, then the "Current state" entry at the top of ai-lab/LOG.md and the log
> entries it points to, then ai-lab/BASELINE.md and ai-lab/runs/README.md. Carry on from the current state's "Next" list, following
> the plan's rules, loop and handover rule (sections 6, 7 and 12). The user follows this session and is the
> person you report to and ask for rulings; give times in US Eastern. Start by telling the user in a few lines
> what you will do first.
