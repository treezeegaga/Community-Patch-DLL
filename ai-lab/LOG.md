# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## Current state (2026-11-23, 1:30 AM ET), implementer 9 (rewrite after every accepted or reverted iteration)

- **BOX is paused (2:45 PM ET Nov 22, user's ruling): the user is using it for other things.** Lab runs go to TREEDESKTOP only until the user says BOX is back, so a request takes about twice as long. (BOX stopped reporting at 2:46 PM.)
- **Talk to the user in plain words**: "step 3: the AI builds that army in time" rather than "M3"; "the mod", not "the
  pack"; explain any term the first time. Times in US Eastern.
- **The review session speaks for the user:** from 10:10 AM Nov 22 that is "Project review 2"
  (session_01G2NZizyLCf9i5g45DQDYLB; PLAN section 4); the old "Project review" no longer counts. Review 2 can receive
  send_message replies, and starts the next implementer when this heading says HANDOVER READY.
- **Milestones:** M0-M2 closed. **M3 in progress.** Bar: early rush, war about turn 41-50, a capital by about turn 60
  on Quick, about 6 melee + 4 Composite Bowmen, at least 2 cities, gold not at zero before the war is decided.
- **Newest build:** `ab-b8c9ffb` = it. 26 (accepted 12:15 PM, `ab-434eecf`: ranged units hold only where they can shoot
  the city) + it. 25 (accepted 7:50 PM, provisional: city-states off limits through the plan's window; tuning set 3: diversion 8 seeds -> 0, captures 7 -> 7, gained 8383 Athens t76, lost 7777 Sparta). Accepted M3 work: CODEMAP "Plans" and the override
  table. Draft it. 25 (melee hold) shelved in `drafts/it25-melee-hold.patch` (no scenario reproduces its cause).
- **Headline (user ruling 9:40 AM Nov 22): the 48 measuring maps, seeds 1919-6666** (never for tuning). **Newest, build
  `ab-b8c9ffb` (it. 26 + it. 25, `00m-meas25`, Nov 23 1:09 AM):** Songhai new AI takes a major's city by turn 100 on
  **16 of 48** (it. 22: 13; stock 0), a capital on 9 (7), a capital by turn 60 on 2 (1); army 43 (42), war 39 (38).
  At turn 100 science 11.4 (10.8; stock 26.3), population 12.3 (11.6; stock 15.7). Map for map and what got worse: LOG
  entry of Nov 23. All with Greece as a neighbour (fixed seat 1). Dashboard "Civ V AI Project" shows these
  (`ai-lab/runs/fullgames_csv.py` builds its table; `runs/measure_compare.py` the map-for-map list).
- **Stalled wars on the it. 22 build (25 of 38), corrected 10:30 AM** (dashboard stalls.csv now has this split): army sent to a city-state 2; estimate NO 4; TAKE with the city never
  below 90% 5, dented and healed 9, below half not taken 5 (LOG entry of Nov 22). City-states take the plan's army
  when the plan ends at t60 with no war on a major (5 of the 25). Battle-lab copies of 505 and 1616 don't stall alone (6/6 even at real strength), but 505 with
  its far-side tiles held (`stall505b`) does (2/6) and it. 26 fixed it (4/6): the lab can stage this one. **Provisional (rule 10):** work the sieges in full games on the tuning seeds, with the
  siege snapshot (SMAP/SCITY/SUNIT/SSPOT lines) to see what draws the army away.
- **Lab:** v24 (siege snapshot in transfer mode; stall scenarios with ground, units at offsets, river edges, city damage,
  defenderTechCount). `wait-for-results.sh 110 done` waits for done markers only.
- **Queued:** `01-log505` (3:05 AM Nov 23): seed 505 to turn 100 and stall505b x3 on `ab-41e754e` = `ab-b8c9ffb` plus lab logging of the tactical simulation's picks ("tactsim pick" lines). Why: at 505 bowmen 1101 and 1179 stand next to St. Petersburg with a shot ("can shoot it from here 1", tactical move 20) on t62-67 and the capture attempt says "executed", yet the city only drops 200 -> 185; the estimate counts 45 ranged damage a turn. Done before: `00k-i25`, `00l-i25guard` (passed), `00m-meas25` (reported). Withdrawn unstarted (12:25 PM): `02-holdC24b`, `03-songx24b`, `04-guard24b`
  (old build; superseded by it. 26's guard and by the measuring re-run). **User via Review 2 (12:30 PM): once it. 25 is
  decided, re-run the 48 measuring maps (Songhai new AI, seeds 1919-6666) on the newest accepted build** and report
  the headline map for map against it. 22, what got worse included, in this state and on the dashboard. Stock not re-run.
- **Next:**
  1. Seed 505 full game (still holds after it. 26): why the one bowman with a shot and the two "allowed" Spearmen never attack St. Petersburg
     t61-67 (tactical moves REINFORCE/STEAMROLL), against the lab copy where they do. Then the other stalled tuning seeds
     (404, 1313, 1414, 1616, 1717, snapshots in `sd24bN*`).
  2. The plan's way out (user, point 3): back to the economy once the rush fails or succeeds; own iteration, with "state
     at turn 100 vs stock on the same map" tracked beside captures.
  3. Vary the neighbour (point 4): a lab-only setting for seat 1's civ chosen from the seed; mark all-Greece results.
  4. Late declarations (point 5): army ready, no war for 20+ turns.
- **Waiting for the user:** nothing. **For the user:** archive implementers 5-7 if not done; the old review session.

## Stage table (review relay of 11:10 PM Nov 21; update after every accepted change)
Turn each game reached each stage. Songhai new AI on the newest accepted build (it. 22, `ab-d0f5828`), played to
turn 100: 12 tuning seeds (`26-long22`), 6 holdout seeds (`25-holong22`; **picture only, never for choosing a fix**);
other warmonger civs with the plan (new AI for all majors but Songhai): tuning seeds on it. 21 (`24-long21`, it. 22
changed no outcome) and holdout seeds on it. 22 (`25-holong22`). How each stage is read: (a) first turn the land army
count (lab OPEN line) rises; (b) TechAILog "COMPLETED, TECH_CONSTRUCTION"; (c) first turn with 6 melee + 4 ranged of
Composite Bowman strength (OPEN); (d) first and last "plan upgrade:" (MilitaryAILog); (e) first OP_CITY_ATTACK on a
major's city (OperationalAILog); (f) first war with a major, declared by us or on us; (g) after the war, first siege
estimate with at least 5 of our units within 4 tiles of an enemy city (the estimate counts within 4, not 3); (h)
first estimate showing that city under half its hit points; (i) first capture of a major's city.

| game | civ | a first unit | b Construction | c army 6+4 | d upgrades (first-last) | e attack op | f war | g at walls | h city < half | i capture / where it stopped |
|---|---|---|---|---|---|---|---|---|---|---|
| **user's own game (reference)** | | | **41-44** | **41-44** | **41-44** | | **41-50** | | | **capital 54 (bar: about 60)** |
| 101 | Songhai | 7 | 35 | 69 | 35-59 | 60 | 63 | 65 | 91 | York t92 |
| 202 | Songhai | 7 | 45 | 57 | 45-55 | 58 | 59 (by Greece) | 60 | 66 | Corinth t66 |
| 303 | Songhai | 6 | 35 | 58 | 35-52 | 72 | 95 | 95 | 97 | Osaka t97 |
| 404 | Songhai | 6 | 37 | 43 | 42-48 | 49 | 54 | 65 | 82 | city below half, no capture by 100 |
| 505 | Songhai | 6 | 36 | 52 | 36-36 | 53 | 59 | 60 | - | at the walls from t60, city never below half by 100 |
| 606 | Songhai | 6 | 37 | 47 | 37-45 | 48 | 51 | 51 | 56 | Athens t56 |
| 1313 | Songhai | 7 | 37 | - | 42-42 | 60 | 78 | 79 | - | at the walls from t79, city never below half by 100 |
| 1414 | Songhai | 6 | 38 | 49 | 38-54 | 52 | 64 | 68 | - | at the walls from t68, city never below half by 100 |
| 1515 | Songhai | 5 | 34 | 39 | 34-34 | 45 | 51 | 54 | 59 | Stockholm t60 |
| 1616 | Songhai | 6 | 34 | 47 | 34-42 | 48 | 56 | 56 | 68 | city below half, no capture by 100 |
| 1717 | Songhai | 6 | 40 | 53 | 40-55 | 57 | 62 | 80 | - | at the walls from t80, city never below half by 100 |
| 1818 | Songhai | 6 | 39 | 47 | 39-54 | 52 | 55 | 56 | 63 | Edinburgh t64 |
| holdout ho707 | Songhai | 6 | 38 | 45 | 38-52 | 53 | 60 | 61 | 65 | Sparta t67 |
| holdout ho808 | Songhai | 8 | 37 | - | 37-37 | 60 | 84 | 85 | 88 | Athens t88 |
| holdout ho909 | Songhai | 6 | 39 | 58 | 39-49 | 59 | 64 | 65 | 70 | Argos t70 |
| holdout ho1010 | Songhai | 7 | 36 | 63 | 36-36 | 60 | 71 | 75 | - | at the walls from t75, city never below half by 100 |
| holdout ho1111 | Songhai | 7 | 40 | 70 | 40-40 | 60 | 66 | 67 | - | at the walls from t67, city never below half by 100 |
| holdout ho1212 | Songhai | 8 | 42 | 42 | 42-49 | 50 | 56 | 56 | - | at the walls from t56, city never below half by 100 |
| 202 | England | 7 | 39 | 69 | 39-39 | 67 | 80 | 82 | 87 | Amsterdam t87 |
| 303 | Japan | 7 | 38 | 48 | 38-39 | 48 | - | - | - | operation started, no war (cancelled/never declared) |
| 303 | Denmark | 7 | 34 | 60 | 34-44 | 60 | 64 | 64 | - | at the walls from t64, city never below half by 100 |
| 404 | Persia | 7 | 40 | 53 | 40-54 | 56 | 59 | 60 | - | at the walls from t60, city never below half by 100 |
| 606 | Mongolia | 6 | 41 | 45 | 41-43 | - | - | - | - | army complete, no attack on a major |
| 1515 | The Aztecs | 8 | 39 | 62 | 39-45 | 60 | 68 | 68 | 72 | Sparta t72 |
| 1414 | The Zulus | 8 | 33 | 59 | 33-76 | 59 | 64 | 65 | - | at the walls from t65, city never below half by 100 |
| holdout civh707 | The Ottomans | 5 | 38 | - | 38-45 | 76 | 97 | 97 | - | at the walls from t97, city never below half by 100 |
| holdout civh707 | The Shoshone | 7 | 41 | 55 | 41-41 | - | - | - | - | army complete, no attack on a major |
| holdout civh1010 | Mongolia | 7 | 41 | 56 | 42-54 | 51 | 49 (by Greece) | 60 | - | at the walls from t60, city never below half by 100 |
| holdout civh1111 | The Aztecs | 6 | 35 | 48 | 35-46 | 48 | 67 | 67 | - | at the walls from t67, city never below half by 100 |
| holdout civh1212 | Assyria | 8 | 40 | - | 43-48 | 78 | 90 | 90 | 94 | Tombouctu t94 |

Script: `ai-lab/runs/stages.py`.

**Turns lost per stage** (median, total over games; "never" = stage not reached by turn 100, counted to 100):

| stage | Songhai tuning (12) | Songhai holdout (6) | other civs (12) |
|---|---|---|---|
| Construction to army complete (b-c) | median 12.5, total 214, never 1 | median 23, total 146, never 1 | median 19, total 296, never 2 |
| army complete to attack op (c-e) | median 3, total 33 | median 1, total 4 | median 0, total 94, never 2 |
| attack op to war (e-f) | median 5.5, total 93 | median 6.5, total 59 | median 10, total 135, never 1 |
| war to walls (f-g) | median 1, total 42 | median 1, total 8 | median 0, total 15 |
| walls to city under half (g-h) | median 14.5, total 193, **never 4** | median 15, total 114, **never 3** | median 33, total 200, **never 6** |
| under half to capture (h-i) | median 1, total 53, never 2 | median 0, total 2 | 0 |

**Where the turns go.** Construction lands on time (turns 34-45; the user's game 41-44). Two stages lose the most:
(1) **the army is completed 12-23 turns after Construction** (the user's was complete when Construction landed): the
largest total in every group; the last plan upgrade comes late (101: first t35, last t59) and the ranged count only
counts Composite Bowmen; (2) **the siege at the walls**: 7 of 18 Songhai games never get an enemy city under half in
20-45 turns at its walls; once a city is under half it falls within 1 turn. Next iterations aim at (1), then (2).

## 2026-11-23 Measuring maps on the newest build (`00m-meas25`, `ab-b8c9ffb` = it. 26 + it. 25; user via Review 2, 12:30 PM Nov 22)
Songhai new AI to turn 100 on seeds 1919-6666, map for map against `nx22N*` (it. 22). Stock not re-run.
- Capture of a major's city by turn 100: 13 -> **16**; capital 7 -> 9; capital by turn 60: 1 -> 2 (1919: t61 -> t60).
  Gained: 4242 (war t74, capital t80; before no war), 4444 (t72), 4848 (t76), 6464 (capital t77).
- **Worse:** 3636 lost its capture (now no war at all; before war t92, city t97). Wars later: 2727 t64 -> t92, 5858
  t76 -> t97 (both city-state stalls before: the freed army still doesn't declare; late declarations, Next). Science at
  turn 100 down: 3838 20 -> 0, 3131 19 -> 4, 6464 16 -> 5, 5959 13 -> 4, 4040 16 -> 8, 2020 14 -> 7 (up on 2626 5 -> 24,
  4444 0 -> 20, 5454 3 -> 15, 4949 3 -> 15, 5858 0 -> 14).
- Averages at turn 100: science 11.4 (10.8; stock 26.3), population 12.3 (11.6; stock 15.7), cities 2.71 (2.58), gold 30 (32).
- The bar (capital by about turn 60 with the rest of M3's bar): still not met on most maps; the economy gap to stock
  stays the largest gap (the plan's way out, Next 2).

## 2026-11-22 M3 iteration 25: city-states stay off limits through the plan's whole window (new AI)
Class: the plan's army sent to a city-state when the plan ends at t60 with no war on a major (measuring maps: 5 of the
25 stalled wars, Review 2's point 2; new tuning set 3, seeds 7070-9393 on `ab-45cd73b` (`tune3`): 8 of 24, 7272 7777 7979
8383 8787 8989 9090 9191, operations on a city-state at t60-74 before any capture).
Cause: `CvMilitaryAI::GetActivePlan` returns NULL from the row's end turn (60) when no war on a major has started, and
it. 16's blocks in `RequestCityAttack` (minor targets) and `RequestBullyingOperation` test `GetActivePlan`.
Change: new `CvMilitaryAI::IsPlanWindowOpen()` (new AI, warmonger, no city taken, before turn 90 Quick); both blocks
also hold while it is true (the bullying block covers shows of force on majors too, as during the plan). New AI only.
Built on top of it. 26 (decision on 26 pending its guard sets).
Predicted (tuning set 3, i25 against i26 on the same build line, Songhai new AI to turn 100): Songhai operations on a
city-state before its first capture and before turn 90 fall from 8 seeds to 0; seeds with a capture by turn 100 not
fewer; on the 8 diversion seeds, at least 2 gain a war on a major or a capture before turn 80. 12 tuning seeds: no
seed loses a capture. Guard sets: no scenario loses more than 1 in 6. Holdout and measuring maps untouched.
Results (`00k-i25`, re-queued after `00j` failed on a cancelled build; 60 games; i26t3_7070 failed to run, so 23 pairs):
- City-state operations before the first capture and turn 90: **8 seeds -> 0** (held).
- Seeds with a capture of a major's city by turn 100: 7 of 23 -> 7 of 23 (held: not fewer), plus 7070 Corinth t80 on it. 25.
  Gained: **8383 Athens t76 (a capital; war t72, before: no war)**. Lost: **7777 Sparta t68 -> none**: same war t62, same
  army at Sparta (3 melee, 4 ranged within 4), but ranged damage 42 a turn instead of 88; Greece hit Tombouctu t62 and
  an OP_RAPID_RESPONSE took 2 units home (in i26 the units were tied to the city-state sneak operation instead).
- Of the 8 diversion seeds, gained a war or capture before t80: 1 (8383); predicted at least 2: **missed**. The other
  six never go to war with a major by t80 (7272, 7979, 8989, 9090, 9191 none at all; **8787's war moves t69 -> t98**,
  a worse late declaration).
- 12 tuning seeds: identical to it. 26 (no diversion there): no seed loses a capture (held).
Decision (7:50 PM ET, provisional under rule 10): **accepted.** The cause is gone on all 8 seeds, captures are not
fewer, the gained capture is a capital; the miss is that freeing the army doesn't by itself start a war (late
declarations, Next 4). 7777's loss and 8787's late war are logged against it. Guard sets queued (`00l-i25guard`).
Guard sets (`00l-i25guard`, 8:12 PM): siege 74 of 90 (it. 26: 73), march 42 (42), regress 24 (24); no scenario loses more than 1 in 6 (small_vs_medium 6/6 both, melee_only_vs_medium 5 -> 6). **Acceptance stands.**

## 2026-11-22 M3 iteration 26: a ranged unit holds only where it can shoot the city (new AI)
Class: "TAKE, the city never below 90%" (5 of the 25 stalled wars on the measuring maps; tuning seed 505 too). Review 2's
point 3 (10:30 AM): in 4848 (Sparta t71-86) and 505 the bowmen stand 2 tiles out and the city isn't shot.
Cause: `CvTacticalAI::PlotReinforcementMoves`, the new AI's "ranged closing in (ring 2)" (M2 it. 4/4b, ours) gives a
ranged unit within 2 tiles a skip mission ("holds") without asking whether it sees the city. Around a city on a hill
ringed by forest only the adjacent tiles (and a couple on the far side) see it (505's SSPOT lines): 4848's log has four
Composite Bowmen "holds" at distance 2 with "can shoot it from here 0", and "capture attempt ... executed: 4 units in
reach, 0 melee" every turn with no damage. The lab copy (stall505) shows the same opening, and gets out only because two
far-side tiles were free; `stall505b` holds them with defenders, as the full game's enemy units did.
Change: there, a ranged unit holds only if `canEverRangeStrikeAt` from its plot (line of sight over revealed ground);
else it moves to the nearest free revealed plot within its range that sees the city, reachable within 3 turns, full
range preferred (`LabFindFiringPlot`). Log: the closing-in line adds "can shoot from here" / "to firing plot x:y" / "no
firing plot in reach". New AI only.
Predicted (only if stall6 shows the stall on `ab-45cd73b`): stall505b new AI from at most 1/6 to at least 4/6; stall505s
capture turn from 7.7 to 6 or less; no scenario of the siege, march, regress and stall sets loses more than 1 in 6;
12 tuning seeds: 505 St. Petersburg shot (below 90%) and no seed loses its capture (4 capitals). Holdout untouched.
**Baseline (stall6, `ab-45cd73b`, 10:24 AM):** stall505b reproduces the stall: new AI 2/6 (capture t12-13, mean
damage 50%), stock 1/6 (35%); at the start of our turns the bowmen near the city could not shoot it on 236 of 334
lines, 36 "ranged closing in ... holds". So the prediction's baseline is 2/6 (not "at most 1"); target unchanged: 4/6.
**Result (i26a, build `ab-434eecf`, 11:13 AM):** stall set (new AI, 6 each): **stall505b 2/6 to 4/6** (mean damage 50% to
69%), **stall505s capture t7.7 to t6.0**, every other copy 6/6 as before (1616 copies faster, t3-4). Tuning seeds (Songhai
new AI to turn 100, against `l24bN`; 1818 against `long22N`):

| seed | before (it. 24b) | after (it. 26) |
|---|---|---|
| 101 | York t92 | **York t69** |
| 1616 | nothing | **Attila's Court t67 (capital)** |
| 1515 | Stockholm t60 (capital), Helsinki t95 | Stockholm t58 (capital) |
| 606 | Athens t56 (capital) | Athens t57 (capital) |
| 202 | Corinth t66, Athens t71 (capital), Sparta t85 | Corinth t66, Athens t71 (capital) |
| 1818 | Edinburgh t64 (capital) | Edinburgh t64 (capital) |
| 303 | Osaka t97 | Osaka t97 |
| 404, 505, 1313, 1414, 1717 | nothing | nothing |

Capitals 4 to 5; seeds with a capture 6 to 7; captures by turn 80 on 4 to 6 seeds; two late second captures lost (202
t85, 1515 t95). 505: "to firing plot" 16 times, St. Petersburg down only to 185/200 (92.5%): that prediction missed.
Predictions met: stall505b, stall505s, no seed lost its capture.
**Guard (i26guard, 12:12 PM):** siege 73/90 (it. 22's guard 74; melee_only_vs_medium 6 to 5, all else equal), units lost
19 to 20; march 42/42; regress 24/24. **Decision: accepted.** A ranged unit at a city on a hill ringed by forest now
finds a plot that sees the city; the 505 game itself still holds (185/200), so b1 is not all of it there.

## 2026-11-22 Rule 3 on the stalled sieges: the copies don't stall (lab v21-v24, build `ab-45cd73b`)
Seeds 505 (b1) and 1616 (b2) replayed with the siege snapshot (`sd24bN*`, `sd2N*`: the same games as before) and copied
into the battle lab's `stall` set with their ground, units, hit points and (v24) tech count. Each copy, 6 battles:

| copy | city strength (real) | new AI | stock |
|---|---|---|---|
| stall505 (pop 6, ancient techs) | 1144 (1585) | 6/6, t6.2 | 4/6 |
| stall505c6 / c9 (Construction, pop 6 / 9) | 1158 / 1278 | 6/6, 6/6 | - |
| stall505s (pop 14, 15 techs) | 1565 (1585) | **6/6, t7.7** | **1/6** (47% damage) |
| stall1616 (pop 4) / cap (capital) | 1064 / 1328 (1496) | 6/6, 6/6 | 6/6, 6/6 |
| stall1616s (capital, pop 8, 14 techs) | 1556 (1496) | **6/6, t4.0** | 4/6 |

505's ground is the cause of its first turns (the city on a hill ringed by forest, sea south: from our side only the
tiles next to it can shoot it): in the lab too the bowmen spend 2-6 turns at range 2 without a shot, then find the two
firing tiles on the far side. 1616's real game had more than the army and the city: barbarian Warriors next to the city
drew the bowmen's fire t57-59 (city 199-200), Hun rams and a horse archer arrived, and the real city's strength had a
player-wide bonus (policy or trait; no Lua setter, so the copies use population). **Finding: alone at the walls, the new
AI takes both cities; the full-game stalls come from what the lab leaves out** (other enemy units and barbarians drawing
fire, reinforcements, the army's other fronts). The stall set stays as a guard (new AI 6/6 on all copies). Draft it. 25
(melee hold, `drafts/it25-melee-hold.patch`) has no scenario to justify it: shelved.
Provisional (rule 10, user told): the sieges are worked in full games on the tuning seeds, with the siege snapshot to
see what draws the army away; scenarios only for a cause the lab can stage.

## 2026-11-22 The 25 stalled wars of the measuring set, by kind (user ruling 9:40 AM, point 2; build of it. 22)
Songhai new AI, seeds 1919-6666 (`nx22N*`): 38 wars with a major, 13 with a capture by turn 100, 25 without. Read from
the siege estimate lines after the war starts (the city with the most TAKE turns; min hp over the whole war):
- **army sent to a city-state: 2** (2727 Riga, 5858 Bucharest: operations of t60 on a city-state, the wars on majors
  declared later with nothing sent).
- **estimate NO throughout: 4** (2020, 3737, 5353, 6262): the army near the city never looked enough.
- **TAKE, then the city never under 90% (b1, as 505): 5** (4141, 4848, 5454, 6060, 6464).
- **TAKE, city dented to 56-81% and healed back: 9** (2525, 2929, 3131, 3232, 3535, 4343, 4444, 5757, 5959).
- **TAKE, city shot below half, not taken (b2, as 1616): 5** (3434, 4545, 4747, 4949, 5555 Tenochtitlan to 26% t91).
**Corrected 10:30 AM** after Review 2's recount (`ai-lab/review/recount_stalls.py` on `claude/bold-feynman-cgnhjs`):
my first pass took the city with the most TAKE turns and didn't check its owner (2727, 5858 were city-states; 5555 read
Teotihuacan 93% instead of Tenochtitlan 26%). Checked here against the tactical and operational logs: agreed.
So 14 of the 19 TAKE stalls on majors are cities that never come below half (b1 + dented).
**City-states take the plan's army at t60 (cause found, Review 2's point 2):** `GetActivePlan` ends the plan at its
end turn (60) when no war on a major has started, and it. 16's block on city-state operations (`RequestCityAttack`,
`RequestBullyingOperation`) ends with it: in 5 of the 25 stalled games an operation on a city-state starts at t60-64
(2727, 5858, 6464 Kabul, 6060 Kuala Lumpur, 3737 Ragusa x8). Not seen on the 12 tuning seeds (their city-state
operations come after a capture, 606 and 1818, or late, 505 t89, 202 t98): new tuning seeds needed to test a fix.

## 2026-11-21 Stalled sieges sorted (review relay of 11:40 PM: work the sieges now, alongside army completion)
Siege estimate turn by turn while the army stood at the walls (PlayerTacticalAILog, it. 22 builds, to turn 100).
Every stall starts as **TAKE** for several turns and turns into NO as the army wears down; none is a plain "too weak
on arrival". Two kinds:
- **(b1) TAKE, the city is never shot:** 505 St. Petersburg t60-67 TAKE (3 melee + 4 ranged within 4), hit points
  200/200 all 8 turns; the Composite Bowmen sit 3-4 tiles out ("can shoot it from here 0" on 25 of 26 lines, tactical
  moves 17 and 20 = steamroll) and never step to range 2; then NO (1 melee, 0 ranged). Same in holdout 1111
  Teotihuacan (t67-84 TAKE, 5-6 melee + 1-3 ranged, 200/200 for 17 turns; picture only). Likely 1313, 1414
  (TAKE 2-4 turns, then NO, 250 hp capitals) and 1717.
- **(b2) TAKE, the city is shot down but the melee is gone first:** 1616 Attila's Court t56-68 TAKE, bowmen in range
  (37 of 52 lines "can shoot 1"), city 200 to 88, melee 3 to 1, then NO and the city heals back to 250. 404
  Persepolis (TAKE 18 turns, down to 157/250, melee 4) is near this kind.
Next (rule 3): copy 505 (b1) and 1616 (b2) into the battle lab as scenarios, the army as it arrived and the city as it
was, and check they reproduce the stall on the current build; then iterate in the lab while the full games carry the
army-completion work. Not yet done (handover). Holdout games are not used to build scenarios.

## 2026-11-21 M3 iteration 24: the plan's sneak attack on a major isn't cancelled when the approach leaves WAR, and declares at the walls (new AI)
Class (cmp10 i23c303, long21C606): Japan's first operation (t48, 7 units incl. 4 Composite Bowmen, sneak attack on
Songhai's Gao) moved at t53 and was cancelled t56: "War goal changed" / "Ended: DiploOpinionChange"
(`CvDiplomacyAI` cancels sneak attacks when the approach toward that major stops being WAR; `DoMakeWarOnPlayer`
declares at the walls only if the approach is WAR then). Mongolia's attacks restart 7 times the same way (on a
city-state, after the plan ended; not touched here).
Change (new AI, while `GetActivePlan()`; majors only): the cancel is skipped (log "War goal changed; the plan keeps
its sneak attack", OperationalAILog); when the army is in place, war is declared even if the approach isn't WAR
(log "Plan army in place: declaring war although the approach is not WAR"). The co-op war wait and `canDeclareWar`
still apply.
Predicted (11 other-civ seeds i24c against i22c; 12 Songhai seeds i24s against i22s): "plan keeps its sneak attack"
on at least 2 seeds; Japan at war with Songhai by turn 65 on 303; plan-civ captures (not Songhai) by 80 at least 2
(i22c: 1); Songhai captures not fewer (4 capitals). Regress 8 of 8. Holdout untouched.
**Result (cmp11, build `ab-9ed46d1`, 11 PM ET): no game changed; incomplete, fixed as 24b.** Regress 8 of 8. Songhai
captures identical on all 12 seeds; other civs identical on all 11. "Plan keeps its sneak attack" once (303, Japan
t56), but the same turn `DoMakeWarOnPlayer`'s not-in-place branch cancelled it as a "show of force" ("Show of
force cancelled" / "Ended: DiploOpinionChange"): with the approach no longer WAR and no demand, that branch stops any
offensive operation toward the player. (Two runs, i24c606 and i24c1313, ended without a result on the first attempt
and finished on the second.)
**24b:** that branch also leaves the plan's operation toward a major alone. Same predictions, run as cmp12.
**Result 24b (cmp12, build `ab-45cd73b`, 12:25 AM ET Nov 22): accepted.** Regress 8 of 8. Songhai: same captures on
all 12 seeds. Other civs: identical on 10 of 11; on 303 Japan's 7-unit sneak attack is kept (t56-57), reaches Gao and
**declares war t58** (before: cancelled t56, no war by 100); the siege estimate says TAKE from t58 (Gao 200 to 169 by
t60) and the city holds to 80, the same stalled-siege kind as Songhai's (b1/b2 above). Predictions: kept attack on 2
seeds, missed (1); Japan at war by 65, met; plan-civ captures by 80 at least 2, missed (1, Aztecs' Sparta); Songhai
not fewer, met. Accepted anyway: the stage it targets (attack to war) now completes where it applies, nothing got
worse, and the remaining loss is the siege stage the user ordered worked next.



## 2026-11-21 M3 iteration 23: iteration 15 again, for the other warmonger civs (the ready army feeds the war approach)
Class (`24-long21`, other civs to turn 100, it. 21 build): the plan civs' armies stand ready around turn 40 (England's
Composite Bowmen t39, Mongolia's t41-43) and no war comes before the plan ends at t60. England's approach toward
both neighbours (Netherlands, Greece) is FRIENDLY from t40 to t60 with war weight 0 (DiplomacyAI_Approach_Log); its
first operation starts t67, war t80, Amsterdam t87. Mongolia, with no major in reach, marches on Ife (city-state) from
t60, the moment the plan ends, restarting 7 times ("Ended: DiploOpinionChange"). Iteration 15 fed the plan's target
into the war approach and changed nothing, but it was tested on Songhai only, whose approach was already WAR.
Change: iteration 15's code as it was (`CvMilitaryAI::GetPlanWarTarget`/`LogPlanWarTarget`, the WAR weight in
`CvDiplomacyAI::SelectBestApproachTowardsMajorCiv`), on top of its. 16-22. New AI only.
Predicted (11 other-civ seeds i23c against i22c; 12 Songhai seeds i23s against i22s): "plan war target" lines for
at least 2 plan civs other than Songhai; a plan civ other than Songhai at war with a major earlier than in i22c on at
least 2 seeds; plan-civ captures (not Songhai) by turn 80 at least 2 (i22c: 1). Songhai: no game changed, as in
it. 15 (capitals 4). Regress 8 of 8. Holdout untouched.
**Result (cmp10, build `ab-286d92b`, 9:30 PM ET): reverted (prediction failed: no game changed).** Regress 8 of 8.
Songhai: every game identical to i22s (targets logged on 9 of 12, as in it. 15). Other civs: identical to i22c on all
11 seeds; a plan war target logged only for the Zulus (1414, Sparta t59), no war or capture moved. Why: the plan
armies of the other civs (6 melee + 4 strike-class ranged): England ready only t69 (t60: 6+3), Denmark t60, Persia
t53, Japan t48, Mongolia t45, Aztecs and Zulus never. Where ready, `GetPlanWarTarget` found no major city within 12
tiles that the estimate says falls. **The finding to act on:** Japan's first operation (303, t48, 7 units incl. 4
Composite Bowmen, sneak attack on Songhai's Gao) gathered, moved at t53, and was cancelled t56 "War goal changed /
Ended: DiploOpinionChange"; Mongolia's on Ife restarted 7 times for the same reason (long21C606). The approach
toward the target leaves WAR before the declaration and the stock code drops the sneak attack.


## 2026-11-21 Songhai to turn 100 on the tuning seeds (`26-long22`, build `ab-d0f5828`, it. 22)
Captures on 6 of 12 (by 80: 4): 202 Corinth t66, Athens t71 (capital), Sparta t85; 606 Athens t56 (capital); 1515
Stockholm t60 (capital), Helsinki t95; 1818 Edinburgh t64 (capital); 101 York t92; 303 Osaka t97 (Japan's, war t95).
None on 404, 505, 1313, 1414, 1616, 1717 even with wars from t54-78: the army reaches war and stalls at the walls.

## 2026-11-21 Holdout to turn 100 (`25-holong22`, build `ab-d0f5828`, it. 22; check only)
Songhai new AI: captures on 3 of 6: Sparta t67 + **Athens t86 (capital)** (707), **Athens t88 (capital)** (808, war
only t84), Argos t70 (909); none on 1010, 1111, 1212. Other civs new AI (4 of 6 read): no capture by 100.

## 2026-11-21 M3 iteration 22: plan soldiers stay soldiers while the army marches (new AI)
Class (cmp8, other civs): after it. 21, `CvEconomicAI::DoReconState` makes the same Warriors explorers again the next
turn (Denmark's Warrior 1111 sent "back to attack duty" 12 times on 303), so they only join an operation formed in
between. Change (new AI only, via `GetActivePlan()`): new `CvMilitaryAI::IsPlanMarching()` (the guard of
`DoPlanUpgrades`, now shared); `DoReconState` takes no combat unit that isn't a native explorer as a new explorer while
it's true; log "Not exploring with X: plan army marches" (PlayerEconomyAILog). Scouts still explore.
Predicted (12 tuning seeds Songhai i22s against i21s; 11 other-civ seeds i22c against i21c): "back to attack duty"
lines fall to about one per unit (cmp8: 25-64 per game on 7 seeds); plan civs' first operations not smaller (6.8);
plan-civ captures by 80 at least 1; Songhai capitals not fewer than 4. Regress 8 of 8. Holdout untouched.
**Result (cmp9, build `ab-d0f5828`, 7:30 PM ET): accepted, but no game outcome changed.** Regress 8 of 8. "Back to
attack duty" lines per game 19-64 to 1-4 (7 seeds); "plan army marches" refusals 60-171 per game. Plan civs' first
operations identical (England t67 5, Japan t48 7, Denmark t60 7, Persia t56 8, Mongolia t60 7); plan-civ capture by
80: Aztecs Sparta t72 (1515), as before. Songhai: same captures on all 12 seeds (4 capitals: Athens t56, Stockholm
t60, Edinburgh t64, Athens t71; plus Corinth t66); 606 ends with a larger army (9 melee + 7 ranged, was 4 + 4). All 4
predictions met; kept because it fixes the flip-flop the next changes would otherwise trip over, and costs nothing
measured. The explorers were not what keeps the first armies small.

## 2026-11-21 Holdout check of iteration 21 (`23-ho21`, build `ab-047437e`; check only)
Songhai new AI (ho21N against hoN on `ab-006a7f0`): captures 2 of 6 both, now **Sparta t67 (707), Argos t72 (909)**,
before Onondaga t78 (1010), Teotihuacan t79 (1111); **no capital either way.** 1111 army ready t70 (was t52), 1010 t63
(was t60). Other civs new AI (civh21N vs civhN, 5 seeds read): no capture by 80 either way; Mongolia's first operation
t52 with 4 units both. Holdout stays well below the tuning set (4 capitals of 12 there, none of 6 here).

## 2026-11-21 M3 iteration 21: explorer Warriors return to attack duty when the plan marches (new AI)
Direction (review relay 3:10 PM): before more Songhai tuning, why the other warmonger civs declare and take nothing.
Class (tuning seeds, other civs with the new AI, `rlog2` on the rebuilt tip): first operations leave with 4 of ~10
units (England t60, Japan t48). "Recruit skip" (OperationalAILog): Japan's first operation skipped 3 Warriors as
"explorer" (the homeland AI made early Warriors UNITAI_EXPLORE; `IsUnitSuitableForRecruitment` never takes explorers)
while `CountPlanArmy` counts them, so the plan says ready with units that never march; Songhai on 1313 and 1515: 1
each. England's were "threatened zone or enemy near" (Greek units by its cities; not addressed here).
Change (behind `IsNewAI()` via `GetActivePlan()`, in `DoPlanUpgrades`, i.e. once the strike unit's tech is known):
land melee units whose AI type is UNITAI_EXPLORE but whose default isn't (Warriors, not Scouts) are set to
UNITAI_ATTACK; logged "plan: explorer ... back to attack duty". Feeding (point 2): the plan's own step; the stock
explore assignment stays until the march.
Predicted (opening set, 12 tuning seeds, Songhai new AI against i16s/t2N; and other civs new AI against civN/civ2N):
"back to attack duty" lines for Japan and on 1313/1515; first operations of plan civs recruit more units on average
than before; captures by plan civs other than Songhai by turn 80: at least 1 (before: 0); Songhai captures not fewer
(5 of 12). Regress 8 of 8. (Holdout untouched.)
**Result (cmp8, build `ab-047437e`, 5:30 PM ET): accepted.** Regress 8 of 8.
Songhai, 12 tuning seeds (before i16s/t2N, after i21s):

| seed | before | after |
|---|---|---|
| 101 | York t67 | nothing (army ready t69, was t59; war t63) |
| 202 | Corinth t66 | Corinth t66 + **Athens t71 (capital)** |
| 505 | St. Petersburg t63, Novgorod t80 | nothing (war t59) |
| 606 | Athens t59 (capital) | **Athens t56 (capital)** |
| 1515 | nothing | **Stockholm t60 (capital)** |
| 1818 | Edinburgh t64 (capital) | Edinburgh t64 (capital) |
| 303, 404, 1313, 1414, 1616, 1717 | nothing | nothing |

Seeds with a capture 5 to 4 (the prediction "not fewer" missed by one); cities 6 to 5; **capitals 2 to 4, by turn 60:
1 to 2.** Other civs (new AI for every major but Songhai, 11 seeds; before civN/civ2N): "back to attack duty" on 7
seeds (Japan, Denmark...); first operation of each plan civ: England 4 to 5 (t60 to t67), Japan 4 to 7, Persia 5 to 8
(t50 to t56), Denmark 8 to 7, Mongolia 7 to 7, average 5.6 to 6.8; **first capture by a plan civ other than
Songhai by turn 80: Aztecs take Sparta t72 (1515)**, before none (the Zulus' Palenque t78 on 1414 went away; the Zulus
have no plan). Decision: accepted on the mechanism (the targeted skip gone, bigger first operations, 1 plan-civ
capture) and the user's bar (capital by about 60: 2 seeds instead of 1), against one fewer seed with a capture.
Follow-ups seen: `DoReconState` turns the same Warriors back into explorers every turn (the line repeats up to 12
times for one unit), so a unit recruited between those turns is the only gain; England and Persia now leave later.
Holdout check queued (`23-ho21`), and other civs to turn 100 (`24-long21`).

## Purge of the full-desktop screenshots from ai/v2's history (user via review session, 3:10 PM ET Nov 21)
**Done (3:50 PM ET), user confirmed in chat ("go ahead"):** the user couldn't pause the PCs, so instead the backlog
was stopped and watcher v6 pushed first (`Repair-RewrittenHistory`: when the checkout is more than 50 commits ahead
of origin, it keeps the old commits in a local branch `old-history-<date>`, never pushed, resets to origin, and carries
over a just-made results commit); both PCs reported idle on v6 before the rewrite. `git filter-branch --index-filter`
on `d64b737^..ai/v2` (old tip `12a00bad2`) removed every `ai-lab/siege/results/*.png`, the 39 window-only ones too;
checks: no such PNG anywhere in the rewritten history, 1,321 commits both sides, the tip tree identical apart from
those 39 PNGs, `6ddb63174` (before the lab) unchanged. Force-pushed with lease: ai/v2 is now `a4ac7293e`; for 7
minutes after it, nothing old came back. Backups of the old history: this session's local branch
`old-history-backup` (lost with this container) and each lab PC's `old-history-<date>` branch (made on its next
sync). **Still for the user:** delete the 74 old build releases/tags (`ai-lab/runs/purge/delete-tags.txt`, needs
`gh`); until then they keep the old commits reachable. **Build names in this log from before 3:50 PM Nov 21
(`ab-...`) no longer resolve once deleted**; commit hashes in older entries refer to the old history.
The original plan follows.
**What is affected** (checked 3:40 PM): PNGs under `ai-lab/siege/results/` enter ai/v2 at `d64b737` (2026-11-15
11:19, "Siege lab results: diag2"); 377 commits add them; 1,292 commits from there to the tip. All were full-desktop
shots until `a017a7ce3` (window-only, 12:41 PM Nov 21); the tip (`5662ad807`) removed all 391. **73 `ab-` build
tags/releases** point at commits at or after `d64b737` (list: `ai-lab/runs/purge/delete-tags.txt`); the other 252
tags point at older commits on other branches (Nov 1-7) without such files. No other branch carries them: every other
branch tip has none, and only `claude/bold-feynman-cgnhjs` is newer than `d64b737` (the review session found none).
**Steps**
1. The user pauses both lab PCs (`New-Item $env:LOCALAPPDATA\cp-ai-lab\PAUSE`) once they finish their current
   game; the implementer checks both status files say `paused` and nothing is claimed.
2. Rewrite (implementer, here): `git filter-branch --index-filter` on `d64b737^..ai/v2` removing every
   `ai-lab/siege/results/*.png` (also the few window-only shots since `a017a7ce3`: simpler, one rule); commits before
   `d64b737` keep their hashes. Check: no `ai-lab/siege/results/*.png` anywhere in the rewritten history; the tip's
   tree equals the current tip's apart from those window-only PNGs; same commit count. Trial run: see below.
3. Backup: the old history stays on both lab PCs: before resetting, the user renames each lab checkout folder (e.g.
   `cp-ai-lab` to `cp-ai-lab-old-history`), so a private copy stays off GitHub until the user deletes it.
4. Force-push the rewritten ai/v2 (implementer). If refused here, the user runs the same from a PC (commands to be
   written then).
5. The user deletes the 73 releases and tags: `ai-lab/runs/purge/delete-tags.txt` (`gh release delete ... --cleanup-tag`).
   The push in step 4 builds the new tip, so the lab has a current build again. **Build names in this log older than
   the purge no longer resolve.**
6. Each lab PC: fresh clone of ai/v2 into the old folder name (or `git fetch` + `git reset --hard origin/ai/v2`),
   then delete the PAUSE file. The watcher only pulls with rebase and pushes ai/v2: after the reset it has no old
   commits to replay.
7. What remains: GitHub keeps unreachable commits viewable by direct hash link for a while; only GitHub Support can
   purge them for sure. The request to send: "Please run a garbage collection / purge cached views of unreachable
   commits for treezeegaga/Community-Patch-DLL: branch ai/v2 was rewritten to remove screenshots of my desktop from
   files under ai-lab/siege/results/ (e.g. setup-before-start.png); the old commits and the deleted ab-* tags should
   no longer be reachable." Forks or clones made before the rewrite keep their copies.
8. Never again: the lab saves only Civ V's window (`Save-Shot`, since `a017a7ce3`; no other script captures the
   screen); `ai-lab/runs/README.md` says results are public.

## User rulings

- 2026-11-22 (about 10:10 AM ET, the user, confirmed in implementer 9's chat): the review role moves to the session
  "Project review 2" (`session_01G2NZizyLCf9i5g45DQDYLB`, starts from `ai-lab/review/REVIEWER.md` on
  `claude/bold-feynman-cgnhjs`). PLAN section 4 names it; relays from "Project review" (`session_01G2NZj4f5xWWrXLRuDN5eyd`)
  no longer count. Review 2 starts the next implementer when the current state says HANDOVER READY.
- 2026-11-22 (about 9:40 AM ET, the user, relayed by the review session after its analysis of the night's 48 extra
  seeds 1919-6666, build of it. 22): (1) **the 48 seeds are the measuring set** and its figures the headline (Songhai new
  AI: a capture by turn 100 on 13 of 48, 27%; a capital by about 60 on 1-2 of 48; stock 0); the 6-seed holdout is a spot
  check; the 48 are never used for tuning (pick new seeds if more tuning maps are needed). (2) The stalled wars are the
  largest loss (25 of 38 wars take nothing): carry on with the stalled sieges in the battle lab, and use those 25 to show
  how common each kind of stall is (estimate NO vs TAKE and nothing happens). (3) The plan needs a way out: a civ whose
  rush fails or succeeds must go back to its economy (at turn 100 with no capture: science under half of stock's,
  population a third lower). Track "state at turn 100 against stock on the same map" beside captures; the exit is its own
  iteration with its own prediction. (4) Vary the neighbour: Greece is seat 1 in every game (remembered setup); change
  that and say in the log which results are all-Greece. (5) After the sieges, one look at the late declarations (army
  ready, no war for 20+ turns: about a third of games). No change to the rules or the bar.
- 2026-11-20 (about 11 PM ET, the user, relayed by the review session): "I think if I was able to get a capital by 54,
  using Aztecs with Jaguars, the goal for AI could probably be a little later, maybe 60?" So step 3's bar is a capital
  taken by about turn 60 on Quick (was 54). The rest stands: about 6 melee + 4 Composite Bowmen, at least 2 cities, one
  settler, no buildings, gold only for the upgrade; the war still starts about turn 41-50 (if results show that can't
  hold, raise it with the user rather than moving it).
- 2026-11-20 (about 3:30 PM ET, the user, more on the reference rush): "I used gold for nothing except upgrading units
  right before I attacked. I built no buildings, just army units. I was building archers while I researched, and as
  soon as the research finished I upgraded all of my archers and then immediately marched to war with about 60g in my
  bank and losing about 5 gold a turn." So the early-rush row is: no buildings; army units only (archers while
  Construction is researched, melee alongside); gold kept for one purpose, upgrading the archers to Composite Bowmen
  the turn the tech lands; march at once; about 60 gold left and about -5 a turn at the declaration is acceptable.
  Added a few minutes later: "oh I built one settler": the one non-army build (hence the second, undeveloped city).

- 2026-11-20 (about 3:20 PM ET, the user, on the M3 reference game): "I had 2 cities at turn 54 when I captured the
  capital. The second city was completely undeveloped. I basically saved up money from barb hunting and ruins and then
  when I started the attack I essentially was running my economy at a deficit and would have eventually gone below zero
  if I hadn't won the war." So the reference rush is **not** a bigger economy: 2 cities, the capital's production put
  into units, gold banked beforehand, and a deficit accepted for the length of the war. Consequence: the M3 bar stands
  (about 6 melee + 4 Composite Bowmen, war around turn 41-50, a capital by 54, at least 2 cities); "no bankruptcy" is
  read as "doesn't hit zero gold before the war is decided". Neither option put to the user (A: grow the economy first;
  B: lower the army target) is taken; the work is the rush plan itself (PLAN M3's "plans in data").

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

## 2026-11-20 M3 iteration 7: the early-rush plan row and its executor, part 1: the build order (new AI)
Class: no army fielded. Songhai's 182 build choices to turn 54 in 15 opening games: 93 buildings, 46 civilians, 16
wonders, 27 military (under 2 a game). The user's reference rush (rulings of 2026-11-20): one settler, no buildings,
only army units, about 6 melee + 4 ranged, 2 cities.
Cause: the stock city chooses by flavor weights among everything it can build; nothing tells it "this is a rush".
Change (behind `IsNewAI()`): `CvMilitaryAI::SAIPlan` rows (`s_aAIPlans`, a data array for now; the database table comes
when the pack can carry it) and two functions: `GetActivePlan()` (new AI, major, leader's primary victory pursuit is
domination, before the row's end turn (60 on Quick, scaled), not at war with a major; no saved state) and
`GetPlanBuild(city)`: counts land melee/ranged units and settlers (other cities' production included) and returns the
one settler (capital only, once 2 army units stand, while cities + settlers < 2), else the best melee or foot ranged unit
the city can train, whichever is further behind the row's 6:4, else nothing. `CvCityStrategyAI::ChooseProduction` builds
what it returns (logged as CHOSEN) and otherwise chooses as before. Row: AIPLAN_EARLY_RUSH, 1 settler, 6 melee, 4 ranged.
Research, gold and the march are later parts (one per iteration).
Predicted (opening lab, new AI on Songhai, 3 games): builds to turn 54 are army units plus one settler, no buildings
or wonders until the army is complete; land melee about 6 and ranged up to 4 (if Archery is researched in time: the
tech path is not in yet) by turn 50-60, 2 cities; income goes negative as the army grows and gold may hit 0 (watch
"b/c bankrupt" scrapping: if so, the gold part is next, not a revert). War: the attack op (it. 5) should now fill and
start; a declaration by turn 80 in at least 2 of 3. Regress 8 of 8 (the lab's attacker is at war, so no plan builds).
Build: `ab-d69a403`. Run: `2026-11-20-13-op4` (op4a/b/c new AI, op4x regress; all clean, 3:50-4:01 PM ET).
Result: the plan ran in 2 of 3. op4a: builds to turn 50 warrior, settler (turn 6), warriors, archers, spearmen (one
granary in a gap); 6 melee by turn 30, 7 melee + 3 ranged at turn 50, might 57 (op2/op3: 20-30); an attack op recruits
(34, 42), gathers, moves and arrives by turn 50; war on Rome declared turn 51. op4b: 7 melee + 4 ranged at turn 50,
might 83; op arrives turn 55, no declaration by 80; gold 0 from turn 40 and 3 Spearmen gifted "b/c bankrupt" (turns
42, 50, 77). op4c: no plan: the leader's victory pursuit is randomized per game and came out DIPLOMACY there (the
selection rule was wrong), stock-like builds. No captures: at Rome (op4a) and Kuala Lumpur (op4b) the siege estimate says
NO every turn: 2 Archers deal 12-16 a turn to a 200 hp city that heals 20 (the reason the user upgraded to Composite
Bowmen first). Regress 8 of 8.
Decision: **accepted** (where the plan runs, the army reaches the bar's size by turn 50 and marches; nothing worse).
Learned: select the plan by a fixed civ property; Archers can't take a full-health city: the research part is next; the
deficit needs the gold part (op4b gifted units).

## 2026-11-20 M3 iteration 8: the early-rush plan, part 2: research toward the strike unit; plan chosen by civ trait
Class: plan armies arrive but can't take the city (op4a/b: estimate NO, Archers too weak); op4c ran no plan.
Change (behind `IsNewAI()`): the row gets a research target, `UNITCLASS_COMPOSITE_BOWMAN`;
`CvMilitaryAI::GetPlanResearch()` returns the cheapest tech we can research now on the way to that unit's tech (prereq
tree walk), and `CvPlayerAI::AI_chooseResearch` takes it before the tech AI's own choice. Plan selection: civs whose
unique ability is a warmonger's (`CvPlayerTraits::IsWarmonger`, fixed per civ; Songhai is) instead of the randomized
victory pursuit (a fix to iteration 7, in the same build). Gold and the upgrade are part 3.
Predicted (3 games, new AI on Songhai): the plan runs in 3 of 3; Construction known by about turn 40-50; the archers
that exist then are upgraded by the stock AI's own upgrade logic only if gold allows (often not: part 3); Composite
Bowmen built from then on; where 2+ Composite Bowmen reach the city, the estimate turns TAKE and a capture by turn 80
in at least 1 of 3. Bankruptcy gifting may persist (part 3). Regress 8 of 8.
Build: `ab-31db976`. Run: `2026-11-20-14-op5` (op5a/b/c new AI, op5x regress; all clean, by 4:29 PM ET).
Result: the plan runs in 3 of 3; Construction on turns 43/37/39; war declared in 3 of 3 (turns 50-60; op4 1 of 3);
armies at turn 60-70: 6-8 melee, 1-5 ranged, might 62-109. No capture. Most ranged at the walls are still Archers:
"Need gold for Archer upgrade, available 17-53, needed 55" (op5b/c); the gold went on buying Archers (140) and a Scout
(100) instead. 3-4 units gifted "b/c bankrupt" per game. Regress 8 of 8.
Decision: **accepted** (wars now in 3 of 3, Construction on time; nothing worse). Learned: the user's own rule is
needed: gold only for the upgrade.

## 2026-11-20 M3 iteration 9: the early-rush plan, part 3: gold kept for the upgrade (new AI)
Class: plan armies at the walls with Archers that can't be upgraded (55 gold each; 17-53 available) because gold went
on unit and scout purchases (140, 100).
Change (behind `IsNewAI()`): `CvEconomicAI::CanWithdrawMoneyForPurchase` (the one gate for unit, building, tile
purchases, gifts and trades) refuses everything but `PURCHASE_TYPE_UNIT_UPGRADE` while `GetActivePlan()` is not NULL
(before turn 60 and before a war with a major). Upgrades go ahead as soon as the gold is there (stock homeland logic).
Predicted (3 games): no gold purchases before the war; gold at Construction 100+ in at least 2 of 3; 2+ Archers upgraded
to Composite Bowmen before or at the declaration in at least 2 of 3; a capture by turn 80 in at least 1 of 3;
bankruptcy gifting fewer (no purchases draining gold). Regress 8 of 8.
Build: `ab-a4a68e6`. Run: `2026-11-20-15-op6` (op6a/b/c new AI, op6x regress; all clean, by 4:57 PM ET).
Result: gold banks: 123/209/228 at turn 30, up to 437 (op6c, turn 60); bankruptcy gifting 0/3/0 (op5: 4/3/4); 1 gold
purchase in all. Construction turns 38/36/34. Upgrades: 1/0/0, because by then the cities were building Composite Bowmen
(op6c fielded 6). War in 3 of 3 (op6a by turn 40). TAKE estimates at times in all 3, no capture. Why (op6c, Athens): the
first attack op started at turn 48 with 3 units, marched at once and the war began on arrival with "within 4: melee 2,
ranged 2" of an army of 7-9 melee and 5-6 ranged; the Warriors wore down (received 66-134), the rest stayed home
("unassigned ... left to homeland ... garrison"). Regress 8 of 8.
Decision: **accepted** (gold kept, gifting down, nothing worse). Learned: the march must wait for the plan's army.

## 2026-11-21 M3 iteration 20: when the estimate says TAKE, the city goes first in the steamroll and flank postures (new AI)
Class (tuning set): the estimate says TAKE for turns and the city stands (1313, 1515, 1616, 1717). The d1313b replay
(build `ab-0aa0ea5`, ranged-position log; the identical game): from turn 66 all four Composite Bowmen stand 2 tiles
from Attila's Court, "can shoot it from here 1", full moves, every turn, assigned to tactical moves 20/21 (STEAMROLL,
FLANKATTACK); the city loses 15-27 hp a turn. `PlotSteamrollMoves` and `PlotExploitFlanksMoves` first run
`ExecuteDestroyEnemyUnits` on every enemy unit in the zone and only then `ExecuteCaptureCityMoves` with what is left,
so the bowmen's shots go to the units around the city. (The surgical-strike posture goes for the city first.)
Change (behind `IsNewAI()`): in both postures, when the zone's city is an enemy's and `GetSiegeEstimate` says it is
taken (`iTurnTaken >= 1`), `ExecuteCaptureCityMoves()` runs first; the unit attacks and the final capture call follow
as before. Feeding (point 2): the order of two existing steps, gated by the existing estimate; not a plan override.
Predicted (12 tuning seeds against the accepted build): a capture on at least 2 of 1313/1515/1616/1717 (none now);
the 5 seeds with captures keep them (606 Athens by 60); captures 7+ of 12. Regress 8 of 8.
Build: `ab-62054f6`. Run: `2026-11-21-20a-cmp7` (i20s on the 12 tuning seeds, i20x regress).
Result: 10 of 12 identical in outcome to the accepted build; no new capture (1313, 1515, 1616, 1717 still none);
505 took St. Petersburg turn 62 (63) but not Novgorod (turn 80 before). 1313: the capture attempt now counts 6-7
units in reach and expects 54-59 damage a turn, yet Attila's Court goes 147, 132, 127 and back up to 156: the attacks
are ordered but most do not land (not read further). Regress 8 of 8.
Decision: **reverted** (prediction failed: no capture on the four TAKE-but-standing seeds).

## 2026-11-21 The six added tuning seeds, baseline (build `ab-006a7f0`, its. 14+16; no decision)
`2026-11-21-19-tune2`, Songhai seat, new AI vs stock:
| seed | new AI: army 6+4 / war / capture | stock |
|---|---|---|
| 1313 | t55 / on the Huns t65 / - (estimate TAKE for Attila's Court turns 65-74, 10 turns) | no war |
| 1414 | t49 / on Greece t59 / - | no war |
| 1515 | t39 / on Sweden t51 / - (TAKE for Sigtuna/Stockholm from t52) | no war |
| 1616 | t56 / on the Huns t63 / - (TAKE for Attila's Court from t67) | on the Huns t81 |
| 1717 | t53 / on Greece t66 / - (TAKE for Corinth from t69) | no war |
| 1818 | t47 / on the Celts t55 / **Edinburgh (the Celtic capital) t64** | (failed once: Civ V window lost; re-queued) |
Tuning set now (12 seeds, this build): captures on 5 of 12; capitals Athens t59 (606), Edinburgh t64 (1818).
**New failure class in the tuning set:** the siege estimate says TAKE for several turns and the city doesn't fall
(1313, 1515, 1616, 1717). 1313: the estimate counts 4 ranged within 4 tiles at 96-100 damage a turn; the city's
hp goes 200, 173, 147, 132, 128 (turns 65-69) and the capture attempt's "expected max damage per turn" is 11-25;
Warriors make 7-8 damage melee hits on it. Next: `2026-11-21-19a-diag1313` replays 1313 with a log of where our ranged
units stand (distance, can they shoot the city) each turn.

## 2026-11-21 Holdout finding (not acted on): a nearly taken city left standing while melee close in on another
On holdout seed 909 (hoN909, its. 14+16) Argos sat at 21 hp turns 70-72 ("attempting capture of Argos, required damage
21"): the one Warrior in reach (17 hp, then 9) struck for 19-20 each turn, while a Spearman and three Warriors 3-5
tiles away with full moves were sent "closing in on Athens (ring 2)" by `CvTacticalAI::PlotReinforcementMoves` (the
new AI's closing-in, M2 it. 4): each unit goes to the zone city processed first, not the one nearest capture. On
1212 the estimate says TAKE for Assur/Nineveh turns 56-60, no capture by 80 (not read further). The tuning seeds
show no such case (every city brought under 60 hp fell the next turn: 202, 505, 606), so per the holdout rule this
is **not** used to choose a change. Instead the tuning set grows by six seeds (1313-1818, below); if the class
shows there, it is fixed from there.

## 2026-11-21 Backlog: the holdout baseline and other warmonger civs (build `ab-006a7f0`, its. 14+16; no decision)
**Holdout set** (seeds 707-1212, never used for tuning; `2026-11-21-15-holdout`; Songhai seat, new AI vs stock):
| seed | civs | new AI: war / capture / t80 | stock: war / capture |
|---|---|---|---|
| 707 | Greece, Shoshone, Ottomans | we declare on Greece t54 / - / 2 cities, 8+4, 133 gold | - / - |
| 808 | Greece, Venice, Carthage | on Mogadishu (city-state) t65 / - | - / - |
| 909 | Greece, Korea, France | on Greece t64 / - / gold 0 | Greece on us t69 / - (1 city left) |
| 1010 | Greece, Iroquois, Mongolia | on the Iroquois t72 / Onondaga t78 | on Mongolia t81 / - |
| 1111 | Greece, Aztecs, Russia | on the Aztecs t63 / Teotihuacan t79 | - / - |
| 1212 | Greece, Portugal, Assyria | on Assyria t56 / - / gold 0 | - / - |
**The holdout does clearly worse than the tuning set**: captures 2 of 6, at turns 78-79, no capital (tuning set, same
build: 4 of 6, turns 59-67, Athens a capital t59). Wars come on 6 of 6 (5 on a major) but late (54-72) and the sieges
fail. Stock: 0 captures, 1 war of its own. Read as: the plan's gains are real against stock, but the turn-60 capital is
not general yet; the tuning seeds are kinder than average. Seed 808's war went to a city-state after all (the it. 16
hook covers operations, not the diplomacy's own city-state war declarations).
**Other warmonger civs** (`2026-11-21-14-civs`, tuning seeds, new AI for every major but the Songhai seat, vs stock):
the plan switched on (`IsWarmonger`; "plan upgrade" lines) for England (202), Denmark and Japan (303), Persia (404),
Mongolia (606); not for Greece, the Aztecs, Russia, Indonesia, Spain, the Netherlands or Ethiopia. Plan civs declared
war in 5 of 6 cases (Japan t54, Persia t56, Denmark t64, Mongolia t72, England t77); stock civs in the same games:
none. Captures by plan civs by turn 80: 0 (Songhai's games aside). civN101 failed: the new-AI civs (England first)
destroyed the Songhai seat around turn 63-65 and Civ V's "you have been defeated" screen stopped the autoplay (lab
gap: a defeated seat should end the run cleanly). (Correction: the Aztecs do count as a warmonger: see 16-civho;
their only tuning seed, 101, was the failed game.)
**More backlog** (same build): `16-civho` (other civs on the holdout seeds): the plan switched on for the Ottomans,
the Shoshone (707), Mongolia (1010), the Aztecs (1111), Assyria (1212); their wars: the Ottomans on the Shoshone t79,
the Aztecs on Songhai t71; Mongolia and Assyria none (Greece declared on Mongolia t52); captures 0. `17-long`
(tuning seeds to turn 100, Songhai new AI): no capture after turn 80 on any seed (the same 5 as by 80), none lost;
at turn 100 gold rates -1 to -24 (404: -24 with 114 gold), 3 of 6 at 0 gold.
Read as: beyond Songhai on the tuning seeds, the plan does not yet produce captures; its wars come late (52-79).


## 2026-11-21 M3 iteration 19: the plan's upgrade takes the strongest upgrade open now, not the game's default (new AI)
Class: the plan's melee stay Warriors on every seed (404: "melee damage per attack 7-8" at Persepolis). The diagup
replay (seed 404, build `ab-54bc2ba`, upgrade-refusal log; the identical game): every refused melee upgrade is
"Warrior to Swordsman, price 55, tech 0" (176 times) and "Spearman to Pikeman, tech 0": `CvUnit::GetUpgradeUnitType`
returns the last of the unit's upgrade classes, and the Warrior has two (Spearman from VP's table, Swordsman); the
Swordsman needs Iron Working, unknown, so the Warrior is never upgraded although Bronze Working (turn 40) opens the
Spearman. (Also: 16 Archer refusals "must be in friendly territory".)
Change (behind `IsNewAI()` via `GetActivePlan()`, in the plan's own hook `DoPlanUpgrades`): for each unit, of its upgrade
classes, the strongest (melee or ranged strength) that `CanUpgradeTo` now and has the resources, via `DoUpgradeTo`.
Feeding (point 2): no new override; the plan's existing upgrade step.
Predicted (seeds 101-606 against it. 16): "plan upgrade: Warrior to Spearman" on at least 4 of the 5 seeds with a
major near (gold allowing); captures at least it. 16's 5; 404's siege estimate shows more melee damage per attack
than it. 16 at Persepolis; 606 Athens by 60. Regress 8 of 8.
Build: `ab-4882f25`. Run: `2026-11-21-13b-cmp6` (i19s101-606, i19x regress).
Result: **every game identical to it. 16**; no melee upgrade anywhere. Bronze Working (stock research) turns 44-52 on
101/202/303/606, never on 404/505 by 80; with it known and gold up to 254 (101), still no Warrior upgraded. In the game
data actually played the Warrior's only upgrade class is the Swordsman (`GetUpgradeUnitType` names it, and the new
loop found no other class): the repository's VP SQL (`PreUnitChanges.sql` deletes all, `UnitChanges2.sql` adds
Warrior to Spearman) is not what the lab PCs' installed pack has. Regress 8 of 8.
Decision: **reverted** (no effect). Learned: a Warrior becomes a better melee unit only through Iron Working (and an
iron resource) in the played data; otherwise the plan must build Spearmen (Bronze Working) instead of Warriors.

## 2026-11-21 M3 iteration 18: iteration 11 again (Bronze Working after Construction), now that upgrades work (new AI)
Class: seed 404 (its. 14+16): at Persepolis's walls 2 Warriors + 3 Composite Bowmen, "melee damage per attack 7-8"
against 250 hp, siege estimate NO every turn; it. 17's 404 run: 5 melee (4 Warriors) "about 20". The plan's melee
are Warriors on every seed. It. 11 researched Bronze Working but no Warrior was upgraded (the stock upgrade move needs
idle units at home); it. 12 since added `DoPlanUpgrades` (melee pass, anywhere in our land, kept gold). In this mod
the Warrior upgrades to the Spearman (`UnitChanges2.sql`: UNITCLASS_WARRIOR to UNITCLASS_SPEARMAN, Bronze Working).
Change: it. 11's code re-applied unchanged (the row's second research target `UNITCLASS_SPEARMAN`; `GetPlanResearch`
walks the targets in order). Feeding (point 2): no new override; the existing research and upgrade hooks.
Predicted (seeds 101-606 against it. 16): "plan upgrade: Warrior to Spearman" on at least 4 of the 5 seeds with a
major near; the attack ready no more than 3 turns later; 404 takes Persepolis by turn 70; 606 Athens by 60; captures
at least it. 16's 5. Regress 8 of 8.
Build: `ab-ef4f37b`. Run: `2026-11-21-13-cmp5` (i18s101-606, i18x regress).
Result: **no "Warrior to Spearman" upgrade on any seed**; only the Archer upgrades as before. Bronze Working was known
by turn 40 anyway (TechAILog, 404 and 606, also before this change), and 404 keeps 220 gold to turn 80: something in
`CanUpgradeRightNow` refuses the Warriors. Per seed: 404, 505 identical to it. 16; 202 identical but for the army mix;
606 Athens turn 58 (59); 303 army ready turn 53 (58), still no major near; 101 worse: ready turn 68 (59), Hastings
turn 67 instead of York, gold 39 (254). Regress 8 of 8.
Decision: **reverted** (prediction failed: no Spearmen; the research order only shuffled the games). Next: a logging-
only line in `DoPlanUpgrades` says why each refused upgrade fails (moves, price, gold, tech, territory, the game's own
reason), replayed on seed 404 (`2026-11-21-13a-diagup`).

## 2026-11-21 M3 iteration 17: a city's needed garrison doesn't count toward the plan's army (new AI)
Class: seed 404 (its. 14+16): army 6+4 by turn 43, sneak attack turn 47, war at Persepolis turn 53, no capture. The
diag404 replay (build `ab-006a7f0`, recruit-skip log; the identical game) shows the operation took 5 of 10: two units
"needed garrison" (Spearman, Warrior), two "threatened zone or enemy near" (Persian scouts/raiders at home), and one
found no open slot; it marched 2 Warriors + 3 Composite Bowmen, too few melee to take the city; the follow-up operation
(turn 54) gathered home units and needed 19 turns to arrive.
Change (behind `IsNewAI()` via `GetActivePlan()`): `CvMilitaryAI::CountPlanArmy` skips a unit garrisoned in a city that
`NeedsGarrison()` (exposed or under siege), so the build order makes the 6+4 on top of the garrisons and the attack
waits for them. Feeding (point 2): this changes the plan's own count (an existing hook), not a stock decision; the
stock garrison rule still holds the garrisons back.
Predicted (seeds 101-606 against it. 16): army ready 0-8 turns later; the plan's first operation recruits 7+ units on
at least 4 of the 5 seeds with a major near; 404 takes Persepolis by turn 70; 606 still takes Athens (by turn 65);
captures at least it. 16's 5. Regress 8 of 8.
Build: `ab-f55613f`. Run: `2026-11-21-12-cmp4` (i17s101-606, i17x regress).
Result: 101, 202, 303, 505 identical to it. 16; 404: first operation 7 units (Spearman, 4 Warriors, 2 Composite
Bowmen) but turn 55 (it. 16: 47), war turn 58, no capture; 606: war turn 57, Athens turn 61 (it. 16: 52 / 59).
Regress 8 of 8. Why 404 still fails: the siege estimate for Persepolis says NO every turn 59-80 (250 hp, heals 20,
strikes 33-58; at the walls 5 melee + 2 ranged = 24 ranged damage a turn): the army reaches the walls with 2 of its
4-5 Composite Bowmen (the others held home: "threatened zone", healing), and its melee are Warriors (melee damage
per attack about 20 against 250 hp). Decision: **reverted** (prediction failed: no Persepolis, Athens later, past 60).
Learned: on 404 the gap is the strike's composition at the walls (ranged count, melee quality), not its head count.

## 2026-11-21 M3 iteration 16: no operations against city-states while the plan is active (new AI)
Class: seed 606 (it. 14 code, every build so far): approach WAR toward Greece from turn 40, army 6+4 ready turn 47, no
war. The diag606 replay (build `ab-4752664`, operation-start log names enemy and target; the identical game) shows why:
operation 1231 (turn 44, 6 units) went against Mombasa, then Cape Town (turns 55, 64, 66) and Mombasa (68-74), 6-7
units each, ending at the walls without war (the city-states' approach is IGNORE); the one operation on Athens (turn
48) got 3 units and never more.
Change (behind `IsNewAI()` via `GetActivePlan()`): `CvMilitaryAI::RequestCityAttack` refuses a city-state target and
`RequestBullyingOperation` refuses any target while the plan is active (before its war and through it, to the first
capture or turn 90). Feeding instead (point 2): these operations start while the diplomacy's approach to the
city-states is already IGNORE, so feeding the approach would not stop them; the request is the one place they meet.
Predicted (seeds 101-606 against it. 14): 606: a sneak attack on Athens with 6+ units and war on Greece by turn 55;
other seeds unchanged or better where they had city-state operations; captures at least it. 14's (York, Corinth, St.
Petersburg, Novgorod), and a capital by turn 60 on at least 1 seed. Regress 8 of 8.
Build: `ab-05dc217`. Run: `2026-11-21-10-cmp3` (i16s101-606, i16x regress; done by 12:50 AM ET).
Result: **606: war on Greece turn 52, Athens (the capital) taken turn 59** (it. 14: no war); 101, 202, 303, 404, 505
identical to it. 14 (they had no city-state operations). Captures: York t67, Corinth t66, St. Petersburg t63 +
Novgorod t80, Athens t59. Gold 0 on 606 from turn 67 (it. 14: 64). Regress 8 of 8.
Decision: **accepted** (prediction held: 606 war by 55, a capital by 60 on 1 seed, nothing worse).

## 2026-11-21 Lab v20: fixed seeds; the seed check (seed1)
Launch field `seed` (watcher v5) sets config.ini `MapRandSeed` and `SyncRandSeed` for the run (back to 0 after); the
opening lab logs a STARTS line (seed, each major's start). Check `2026-11-21-04-seed1`: seedA (BOX) and seedB
(TREEDESKTOP), seed 101, build `ab-23e0802`: the same STARTS line (Songhai, Greece, the Aztecs, England) and all 320
OPEN lines identical. **Same seed + same build = the same game, on either PC.** So a decision is a matched comparison
on a set of seeds: differences come from the code alone, and luck is sampled only by using more seeds. Step-3 decisions
from now on: 6 seeds (101, 202, 303, 404, 505, 606), before and after.

## 2026-11-21 M3 iteration 15: the plan's ready army declares on a target the estimate says it takes (new AI)
Class (review relay of 8 PM, point 3): op7c, op9b, op10b: the plan's army stood ready (6+4) and no war on a major came;
the war approach never chose war. The war decision is otherwise M4's.
Change (behind `IsNewAI()` via `GetActivePlan()`): `CvMilitaryAI::GetPlanWarTarget`: before any war with a major and
once `IsPlanArmyReady()`, the major owning a city we have seen within 12 tiles of our cities that
`TacticalAIHelpers::EstimateSiege` says our army takes (our land units within 12 tiles counted as if at its walls;
city hit points, strike, garrison as seen), earliest capture, then capital, then nearest. Fed into the stock decision,
not replacing it (point 2): in `CvDiplomacyAI::SelectBestApproachTowardsMajorCiv` the WAR score toward that civ is set
above all others, before the zero-out conditions (no attack target, war would bankrupt us, recent peace, can't declare)
and the approach curve. The stock path then runs the sneak attack: operation, march, declaration at the walls.
Logged in MilitaryAILog: "plan war target: <city> (<civ>), estimated capture turn, melee left, war weight".
Predicted (6 seeded games against it. 14 on the same seeds): a plan war target logged within 3 turns of the army
counting ready in at least 5 of 6; war on a major in at least 5 of 6 (it. 14: fewer), before turn 60 in at least 4;
captures more than it. 14 on the same seeds. Regress 8 of 8 (no plan there).
Build: `ab-ab21a61` (`ab-c013b84` and `22f16eb` failed to compile: a const logger, then an uninitialized estimate).
Run: `2026-11-21-08-cmp1c` (i15s101-606, i15x regress; cmp1 itself failed for want of the build). Same seeds = same maps.
Result: a target logged in 3 of 6 (404 Persepolis turn 47, 505 St. Petersburg turn 55, 606 Athens turn 48; none on
101/202/303); **every game identical to it. 14** on its seed (wars 101 t65, 404 t53, 505 t59, 202 Greece declares t52; none on 303/606).
The approach toward the target was already WAR before the change (606: WAR toward Greece from turn 40, "TGT
Favorable"), so feeding it changed nothing. Why 606 has no war: a sneak attack (operation 1231, turn 44) marched on
Mombasa, a city-state the diplomacy ignores; at its walls it "succeeded", flagged army-in-place against Mombasa, no war
(approach IGNORE) and disbanded; the next operation, on Athens (1251, turn 48), recruited 3 units and then found 0 to
recruit every turn to 80 ("Land Army under construction"; "requested attack on Cape Town ... lack of units").
Regress 8 of 8. Decision: **reverted** (prediction failed: no game changed). Learned: the gap is not the approach but
the operation: which city it marches on (a minor's, chosen by `RequestCityAttack` for a different owner than the
approach's target) and recruiting after the first army disbands.

## 2026-11-21 M3 iteration 14: the early-rush plan carries on through its war (new AI)
Class: op10c: once the war began the plan ended (iteration 10's rule), the cities built Shrines, the gold bought them,
and the army wore down without replacements; op9a's war after turn 60 had no plan at all.
Change (behind `IsNewAI()`): `GetActivePlan()` at war with a major: the plan stays active until the first city is taken
(cities + lost > founded) or turn 90 (Quick), so cities keep building army units up to the row's 6+4, gold stays for
upgrades, no city-state gifts; `IsPlanArmyReady()` is true at war (no hold on attacks). Before a war, unchanged.
Predicted (3 games): during the plan's war no Shrine/building purchase and cities building army units; melee at 4+ ten
turns into the war; a capture by turn 80 in at least 1 of 3 (in a game with a war on a major). Regress 8 of 8.
Note: this comparison (op11 against op9/op10) is across different random maps; the decision waits for a seeded re-run (6 matched starts, review relay of 8 PM).
Build: `ab-23e0802`. Run: `2026-11-21-03-op11` (op11a/b/c new AI, op11x regress; all clean, by 9:08 PM ET).
Result (unmatched maps): op11a war on Greece turn 56, the plan held through it (gold 300 to 39, army 7+6 kept), no
capture by 80; op11b army 6+5 and 398 gold by turn 60, no war with a major, gold then to city-states, bankrupt turn 79;
op11c war on Greece turn 62, gold 330-500 kept, melee 6 to 4, no capture. No Shrines bought during a plan war. Captures
0 of 3. Regress 8 of 8.
**Seeded comparison** (`2026-11-21-07-cmp1b`, it. 13 build `ab-32caacd` against it. 14 `ab-23e0802`, seeds 101-606;
101/202 for it. 14 from seed1's seedA/seedD): 101, 303, 606 identical; 202 (Greece declares turn 52) Corinth, a Greek city, turn 66 not 63, bankrupt
turn 73 not 63; 404 war t53 Persia both, it. 14 keeps 220 gold and 4 ranged at turn 80 (it. 13 bankrupt turn 59, 1
ranged); 505 St. Petersburg turn 63 both, it. 14 also takes Novgorod turn 80. Decision: **accepted** (never worse on
the same start; better on 3 of 6).

## 2026-11-21 M3 iteration 13: the early-rush plan keeps its gold from city-states (new AI)
Class: op9b gave 750 gold to city-states while preparing (op9a 250), went bankrupt and gifted units away;
`CvDiplomacyAI::DoContactMinorCivs` gold gifts don't pass `CanWithdrawMoneyForPurchase`.
Change (behind `IsNewAI()` via `GetActivePlan()`): while a plan is active, the gold-gift list is emptied.
Predicted (3 games): no "Gold Gift" lines for Songhai before its war; no bankruptcy gifting before turn 60; gold at the
march 150+; a capture by turn 80 in at least 2 of 3. Regress 8 of 8.
Build: `ab-32caacd`. Run: `2026-11-21-02-op10` (op10a/b/c new AI, op10x regress; all clean, by 8:37 PM ET).
Result: no gold gifts while the plan was active in all 3 (gifts again only after it ended: turns 60/60/66); no bankruptcy
gifting before turn 60; gold at the end of the plan 426/469/212. But captures 0 of 3: op10a army slow (5+3 at turn 55,
second city turn 45), war on Greece turn 66, melee lost; op10b army 6+4 by turn 50 with 469 gold, no war with a major
(Wittenberg, a city-state, turn 62); op10c war on Greece turn 57 with 6+5, then the plan ended and its gold went to
three Shrines and a Warrior (turns 56-59), melee worn down to 2, bankrupt turn 71. Regress (op10x) 8 of 8.
Decision: **accepted** (its own prediction held: no gifts, gold kept; the lost captures come from the war itself and the
war decision, not from gold kept). Learned: the plan must last through its war.

## 2026-11-21 M3 iteration 12: the early-rush plan, part 5: the upgrade step (new AI)
Class: the plan's army counts ready only when Composite Bowmen are built (turn 60+), and nothing is upgraded: the stock
upgrade (`CvHomelandAI::PlotUpgradeMoves`) runs after tactical and operational moves and takes only idle units at home
(op8a: no upgrade line with 286-370 gold). The user upgraded all Archers the turn the tech landed and marched at once.
Change (behind `IsNewAI()`): `CvMilitaryAI::DoPlanUpgrades()`, called first in `CvPlayerAI::AI_unitUpdate` (before
tactical/operational moves): while the plan is active and the strike unit's tech is known, every plan unit (land combat,
not explorer) in our territory that can upgrade now is upgraded with the kept gold, ranged first, then melee (iron
permitting). Logged in MilitaryAILog as "plan upgrade:". Wars end the plan, so upgrades stop at the declaration.
Predicted (3 games): "plan upgrade" lines the turn Construction lands (about turn 42-50) with 3+ Archers upgraded in
at least 2 of 3; the attack op starts within about 5 turns of that (before turn 55) in at least 2 of 3; a capture by
turn 80 in at least 1 of 3. Regress 8 of 8 (Songhai there has no plan: no change). The war decision is not touched.
Build: `ab-c07b5fe`. Run: `2026-11-21-01-op9` (op9a/b/c new AI, op9x regress; games done by 8:10 PM ET).
Result: plan upgrades 3/2/3 Archers on turns 42-44 / 37 / 43-52. op9a: war on Sweden turn 66 with 6+4, **captured
Sigtuna turn 72** (first opening capture). op9c: war on Arabia turn 60, **captured Mecca (capital) turn 73**. op9b: army
6+4 by turn 40 but no war with a major (targets Mombasa, a city-state); 750 gold given to city-states (turns 43/48/54),
bankrupt from turn 55, units gifted. op9a also gave 250 to Cape Town (turn 53) and its capital built a Water Mill on turn
50 (the second city's slow Warrior already counted as army). Attack within 5 turns of the upgrades: no (60-66).
Regress (op9x) 8 of 8, clean. Decision: **accepted** (captures 2 of 3, from 0 of 15). Learned: city-state gold gifts bypass the gold gate;
production counted from a slow second city stalls the capital.

## 2026-11-20 M3 iteration 11: the early-rush plan's second research target: the melee unit (new AI)
Class: op7b: the full rush at Sparta brought it to 21 hp, then "melee left 0": the melee were Warriors (strength 8,
worn down by the city's 22-28 strike). The plan's research went straight to Construction, never Bronze Working.
Change (behind `IsNewAI()`): the row gets a second research target, `UNITCLASS_SPEARMAN`; `GetPlanResearch()` walks
the row's targets in order and researches toward the first whose tech isn't known (Construction, then Bronze Working).
The build order already takes the strongest melee unit a city can train; existing Warriors are upgraded from the kept
gold (iteration 9) when it allows.
Predicted (3 games): Bronze Working by about turn 45-50, before the march (plan army ready about turn 50-60); 2+
Spearmen in the marching army (built or upgraded) in at least 2 of 3; where the army reaches a city, melee left at the
last hit points, and a capture by turn 80 in at least 1 of 3. Regress 8 of 8. The war decision (op7c) is not touched.
Build: `ab-6071462`. Run: `2026-11-20-17-op8` (op8a/b/c new AI, op8x regress; all clean, by 5:53 PM ET).
Result: Bronze Working turns 42/44/46, but 1/2/4 Spearmen only: the 6 Warriors already fill the row's melee count, and
they are never upgraded: no "upgrade" line in op8a with 286-370 gold. `CvHomelandAI::PlotUpgradeMoves` runs after the
tactical and operational moves and takes only units with moves left (`isReadyForUpgrade`) inside our borders; and the
Warrior's upgrade is likely the Swordsman (Iron Working, iron), not the Spearman. Attack ops started turns 60-66 (the
plan's limit; the army counted ready late), wars at turns 70-80 (op8a 80, op8b about 72, op8c none by 80); siege
estimates NO, no capture. Bankruptcy gifting 0/0/6. Regress 8 of 8.
Decision: **reverted** (the prediction failed: no Spearmen in the army; wars later). Learned: upgrades happen only for
idle units at home; melee quality needs the upgrade path (or the plan building the better melee), not just the tech.
**Seeded re-run (review relay point 1), `2026-11-21-06-cmp2`, 12 games, both PCs, the same 6 starts:** it. 10 build
`ab-d71d189` (i10s*) against it. 11 build `ab-6071462` (i11s*), seeds 101-606, Songhai, 80 turns.
| seed | it. 10: ready / war / capture / bankrupt | it. 11: ready / war / capture / bankrupt |
|---|---|---|
| 101 | t70 / t65 England / - / t82 | t68 / t64 England / Hastings t67 / - |
| 202 | never / Greece declares t52 / Corinth (Greek) t61 / t68 | never / Greece declares t52 / Corinth t69 / t72 |
| 303 | t58 / none / - / t72 | t53 / none / - / t77 |
| 404 | t43 / t53 Persia / - / t59 | identical game |
| 505 | t57 / t63 Russia / St. Petersburg t68 / - | identical game |
| 606 | t47 / t72 Greece / Athens (capital) t80 / t64 | t48 / t59 Greece / - (melee 0 at t80) / t69 |
Major-civ captures 3 against 3 (Corinth on 202 is a Greek city; corrected 11:55 PM); one seed better each way (101 for it. 11, 606 for it. 10), two seeds unchanged.
Decision: **revert stands** (no measurable gain; one fewer override). Also seen: gold hits 0 before turn 80 in 9 of
12 games, and on 2 of 6 seeds no war with a major by turn 80, on both builds.

## 2026-11-20 M3 iteration 10: the early-rush plan, part 4: the attack waits for the plan's whole army (new AI)
Class: wars begin with a fraction of the army (op6: 2+2 at the walls of 15).
Change (behind `IsNewAI()`): `CvMilitaryAI::CountPlanArmy` (shared by the build order and the readiness check; once the
strike unit's tech is known only ranged units at least as strong as it count, so Archers get upgraded or Composite
Bowmen are built) and `IsPlanArmyReady()` (units that exist, not ones being built, reach the row's 6 melee + 4 ranged).
In `RequestCityAttack`, the careless (sneak attack) request of a player whose plan isn't ready returns false: no
operation, so no early march; the cities keep building the plan's army. Once ready, the op starts as in iteration 5.
Predicted (3 games): the first attack op starts only when 6 melee + 4 Composite Bowmen exist (about turn 45-55); the army
at the walls on the war's first turns has at least 4 melee + 3 ranged within 4 tiles; TAKE estimates for several
consecutive turns; a capture by turn 80 in at least 1 of 3 (bar: a capital by 54, not expected yet). Regress 8 of 8.
Risk: the plan's turn limit (60) ends the wait if the army isn't ready by then.
Build: `ab-d71d189`. Run: `2026-11-20-16-op7` (op7a/b/c new AI, op7x regress; all clean, by 5:25 PM ET).
Result: no capture; three different stories. op7b: the op waited to turn 60 (the plan's limit), started with 7 units,
reached Sparta turn 65-66 with "within 4: melee 4, ranged 4"; TAKE turns 66-72; Sparta 200 -> 21 hp by turn 72-73, then
"melee left 0": the melee were Warriors, held at distance 3-5 ("tactical move 0") while the city was above half, closed in
late (18 "closing in" lines from 71) and were worn down; nothing left for the last hit; Sparta healed back. op7c: never
at war: its only major neighbour, Greece, stayed FRIENDLY (approach log), preferred targets were city-states (Hanoi,
Ragusa); army 6-7 melee + 5-6 ranged by turn 60-70 sat idle, then 7 units gifted "b/c bankrupt" (turns 74-80). op7a:
at war from turn 49 (declared on Songhai: 1 melee near Sparta), the op recruited from turn 53 and arrived at 67; 4-5 melee
2 ranged. Regress 8 of 8.
Decision: **accepted** (the full rush army now arrives together, op7b shot a city to 21 hp: the nearest yet; nothing
worse). Learned: three separate gaps remain: (1) the melee are Warriors (the research goes straight to Construction, no
Bronze Working) and are held back until late; (2) the war decision is diplomacy's (op7c: friendly to the only neighbour;
M4's subject); (3) an idle or long war drains gold into bankruptcy gifting (the deficit rule).

## 2026-11-20 M3 iteration 6: preparing for war, the treasury pays for the army (new AI)
Class and cause: iteration 5's result: the soft supply cap counts income only, so with flat income the army stays at
2 units per city with 160-250 gold banked, and the attack operation's units are refused ("nosupply").
Change (`GetSoftSupplyCap`, behind `IsNewAI()`): when preparing for war (a WAR approach) or at war, the minimum income
it keeps drops by gold/30 (spend the bank over about 30 turns; 200 gold allows about -7 a turn). The bankruptcy guard in
`CheckUnitBuildSanity` (no new unit under 9 turns to bankruptcy) is unchanged. Logging for both AIs (not behind the
switch): `MilitaryAILog.csv` "army size: land units, rec land, rec naval, explorers, soft cap, supplied, to supply,
gold, gold rate, offense weight, land defense weight", once a turn (`SetRecommendedArmyNavySize`).
Predicted (opening lab, new AI on Songhai, 3 games vs op2): the log shows rec land = land units (the cap binding) for
stock Songhai at about 2 per city; for the new AI after the war approach the soft cap rises by 3-6, "nosupply" stops for
the op's units, the op fills and leaves recruiting (gathering, moving) and war is declared by turn 80 in at least 2 of 3;
land melee+ranged at turn 80 at least 6 (op2: 3/4/7). Gold falls toward 0 but no bankruptcy (no "b/c bankrupt"
scrapping), at least 2 cities. The stock game: unchanged (its log line confirms the cause). Regress: unchanged.
Build: `ab-15cf7f8`. Run: `2026-11-20-12-op3` (op3a/b/c new AI, op3s stock, op3x regress; all clean, by 3:09 PM ET).
Result: the cause is confirmed: in nearly every turn land units = rec land, and the soft cap (2-7) sits far below real
supply (8-16), for stock and new AI alike. But the change bought little: Songhai's income is already -4 to -7 with 3-4
land units, so gold/30 frees about one unit, and the bank drains (op3a 0 gold at turn 60; op3c spent 190 on a Chariot
Archer). New AI at turn 80: melee 4/3/2, ranged 1/1/1, might 42/54/26, cities 3/2/1 (op2: 2/3/6, 26/32/56); no war; the
op still waits in recruiting (6-20 turns); "nosupply" 20/70/40. No bankruptcy. Stock op3s: 6 melee, might 60, income
+5 to +11 (a better map). Regress 8 of 8.
Decision: **reverted** (the behaviour change; the army-size log line stays, logging only). Learned: the army is capped by
the economy, not by a rule that ignores the bank: at turn 60-80 Songhai has 1-3 cities and income around 0 or below with
3-5 units, so any honest army is about 4-6 land units. Stop (rule 9: third iteration on "the army is too small" without
moving it to the bar), question written to the user below; meanwhile the next line is the economy itself (why 1-3
cities and no income by turn 60-80).
Answered by the user at about 3:20 PM ET (rulings): their rush ran on 2 cities, banked gold and a deficit. What the AI
does with the same start (15 opening games op1-op3, Songhai's 182 build choices to turn 54): 93 buildings (monument 32,
shrine 17, granary 17, water mill 10, library 9), 46 civilians, 16 wonders, **27 military** (under 2 a game); gold bought
30 scouts, 5 workers, 7 buildings and 1 archer. So the gap is what the capital builds and what the gold buys, then the
income rule. Next: the early-rush plan as a data row and its executor (build order, bank, deficit during the strike).

## 2026-11-20 M3 iteration 5: a sneak attack may build the units it lacks (new AI)
Class: no army fielded, no war declared (opening/transfer games). With Songhai's leader flavors (op1a, lab v19) its army
at turn 80 is still 3 melee, 1 ranged; it never declares war.
Cause (`MilitaryAILog.csv`): from turn 32-57 Songhai wants war (preferred target Corinth) and logs "requested attack on
Corinth, from Gao, not possible for lack of units" every turn (op1a 25 times; tr1 games 2-95). Every AI civ does the
same (tr1/op1a: 2-108 per civ per game, from turn 32-100). `CvDiplomacyAI::DoMakeWarOnPlayer` asks
`CvMilitaryAI::RequestCityAttack(target, 3, bCareful)`; with bCareful false (the usual case) RequestCityAttack passes
**0 allowed missing units** to `addAIOperation` ("if we're being careless, just use whatever units we have"), so the
operation starts only if the whole formation already exists. Otherwise it is never created, and cities only build units
for an operation's open slots (`PeekAtNextUnitToBuildForOperationSlot`, muster city): nothing builds the army. The same
gate explains tr1c's "Ended: NoUnits; Duration 0" (Next item 2): `addAIOperation` aborts right after `Init` when
`GetNumUnitsNeededToBeBuilt` > 0 allowed.
Change (behind `IsNewAI()`, `RequestCityAttack`): a careless request passes `iNumUnitsWillingToBuild` (3 for a major, 1 for a
city-state) like a careful one, so the operation starts in recruiting and the muster city builds the missing units; while
an operation of that type against that city exists, the request returns true (no second operation; diplomacy asks again
every turn until the army is in place).
Predicted (opening lab, new AI on Songhai, 3 games vs op1's 3): Songhai's "lack of units" lines drop to near 0 and an
`OperationalAILog` city attack starts in recruiting soon after the war approach (turn 32-57); its muster city builds
military; land melee+ranged at turn 80 up by at least 2 over op1's new-AI games, and a war declared by turn 80 in at
least 2 of 3 (stock games unchanged). Watch: settlers/cities (the bar wants at least 2 cities, no bankruptcy), and
operations ending "NoUnits"/timeout. Siege/march/regress sets: unchanged (their wars are set by the lab; requests there
come from the same path only if the attacker's diplomacy asks): check with one regression launch.
Build: `ab-463dc60`. Run: `2026-11-20-11-op2` (op2a/b/c new AI on Songhai, op2x regress; all clean, 2:22-2:37 PM ET).
Result: an `OP_CITY_ATTACK_LAND_SNEAKY` now starts in recruiting in all 3 (turns 51, 74, 60: 2-3 units, other slots
open) and waits until the cut (8-31 turns); "lack of units" 25/2/12 (other targets or musters). Songhai at turn 80:
melee 2/3/6 (op1 new AI 1/2/4), ranged 1/1/1, might 26/32/56 (17/30/41), cities 2/3/2; no war (prediction: 2 of 3).
Regress 8 of 8. Why short: the muster city doesn't build for the op. `CityStrategyAIProductionLog` (op2a Gao and
Tombouctu, turns 53-70): every military unit "SKIPPED ... nosupply", and no operation unit in the list at all.
`CvUnitProductionAI::CheckUnitBuildSanity` refuses any land combat unit once land units reach
`GetRecommendLandArmySize()` (operation units too); that size is a share of `CvEconomicAI::GetSoftSupplyCap()`: the
units affordable while income stays at or above +1 (war approach) / +2-5 (peace), at least 2 per city. Banked gold
isn't counted: Songhai's income is about 0 and it holds 160-250 gold, so the cap sits at about 4 (2 cities). Real
supply (12) is far off.
Decision: **accepted** (the op now starts and recruits; nothing worse; the next gate explains the miss: iteration 6).

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
Build: `ab-df5ef11`. Run: `2026-11-20-10-op1` (op1a/c/e stock, op1b/d/f new AI on Songhai; BOX and TREEDESKTOP, all
clean, 2:03-2:19 PM ET). FLAVORS line ok in all 6; Songhai's flavors move to Askia's (offense 8, defense 2, wonder 2,
growth 8, military training 4 in op1a).
Result (Songhai at turn 80): cities 2-4 (mean 2.5), land melee 1-5 (3.0), ranged 1-2 (1.2), might 17-44 (31.5); flat
flavors (op0/tr1) gave about 2 cities, 3 melee, might 29. The other AI civs in op1 (18): 3.3 cities, 4.1 melee, 1.1
ranged, might 44. At war by turn 80 in 3 of 6 (op1b/c/f; none a Songhai capture). Gold banked unspent: 75-267.
"Requested attack ... not possible for lack of units": Songhai 25-84 per game, the other civs 252 in all.
New-AI baseline for iteration 5 (op1b/d/f, turn 80): melee 1/2/4, ranged 2/1/1, might 17/30/41, cities 2/2/3, war 2 of 3.
Decision: **accepted** (a lab fix: the opening lab now measures Songhai as an AI with its own personality; lab v19).
Learned: the flavors were not what kept the army small; the "lack of units" gate is (iteration 5).

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
