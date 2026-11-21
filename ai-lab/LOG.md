# AI lab log

Newest first. Format and rules: `PLAN.md`, sections 7 and 10.

## Current state (2026-11-21, 11:55 AM ET), implementer 8 (rewrite after every accepted or reverted iteration)

- **Talk to the user in plain words**: "step 3: the AI builds that army in time" rather than "M3"; "the mod", not "the
  pack"; explain any term the first time. Times in US Eastern.
- **The review session speaks for the user** (session_01G2NZj4f5xWWrXLRuDN5eyd; PLAN section 4); replies to it by
  send_message are refused here (untrusted device), so answer in chat.
- **Milestones:** M0-M2 closed. **M3 in progress.** Bar: early rush, war about turn 41-50, a capital taken by about
  turn 60 on Quick (ruling of 11 PM; was 54), about 6 melee + 4 Composite Bowmen, at least 2 cities, gold not at zero
  before the war is decided. One settler, no buildings, gold only for upgrades right before the attack.
- **M3 so far:** the siege estimate (its. 1-3b); sneak attacks may build missing units (it. 5); lab flavor leak fixed
  (it. 4, lab v19); the early-rush plan for warmonger civs (`CvMilitaryAI::GetActivePlan`, CODEMAP "Plans" and the
  override table): build order (7), research to Composite Bowmen (8), gold kept for upgrades (9), attack waits for the
  plan's army (10), upgrades the turn the tech lands (12), no city-state gifts (13), plan carries on through its war
  (14), no city-state operations during the plan (16). Reverted: 18 (Bronze Working again: no Warrior upgraded), 17 (garrisons not counted: 606 later), 3, 6, 11 (re-run seeded: no gain), 15 (war target fed to the approach: no game changed).
- **How we measure now:** seeds hold (lab v20 / watcher v5): the same seed + build is the identical game on either PC.
  Every decision: seeds 101, 202, 303, 404, 505, 606 before and after (`summ.sh`-style per-seed table in the LOG).
- **Where the newest code stands (its. 14 + 16, build `ab-05dc217`), seeds 101-606:** army 6+4 ready turn 43-59; war
  with a major on 5 of 6 (ours turns 52, 53, 59, 65; Greece's on us t52); captures: Athens t59 (606, **a capital by
  60**), St. Petersburg t63 + Novgorod t80 (505), Corinth t66 (202), York t67 (101); 404 war t53 no capture; 303 no
  major near. Gold 0 by turn 80 on 4 of 6 (turns 64-73, after the war starts).
- **Gaps now:** 404: Persepolis (250 hp, heals 20) needs the 4 Composite Bowmen at its walls; only 2 arrive and the
  melee are Warriors (it. 17 log). 101: army ready only turn 59. Gold runs out during the war on 4 of 6 (units then
  gifted/scrapped "b/c bankrupt"). Lab logs added (no behaviour change): operation start names enemy and target;
  "Recruit skip" says why a unit stayed out of an operation (OperationalAILog).
- **Review relay of 10:05 AM Nov 21 (user):** (1) never leave the PCs idle: before going quiet, queue several hours
  from the standing backlog (PLAN 7a); (2) holdout seeds 707-1212, check only, never for tuning (PLAN 7a, BASELINE);
  (3) when ready to hand over: finish/revert the iteration, write this section, put HANDOVER READY on the first line
  of this heading's text, push, stop (the review session starts implementer 9).
- **Queued:** `13a-diagup` (seed 404 replay: why Warriors aren't upgraded), `14-civs` (backlog a: other warmonger civs, new AI and stock),
  `15-holdout` (backlog b: holdout baseline, new AI and stock), all but cmp5 on build `ab-006a7f0`.
- **Review relay of 8 PM (user):** (1) 6 seeded games per decision: done. (2) CODEMAP override list: done, kept up.
  (3) war when the army is ready and the estimate says a target can be taken: it. 15 (fed the approach) no effect;
  it. 16 (army not spent on city-states) gave 606 its war and Athens. (4) this section kept current. Later: the plan seen working on
  two other warmonger civs before M3 closes.
- **Open for the user:** archive implementers 5, 6 and 7 in the Claude app (trusted device only).
- **Next:** the plan's operation takes its ranged units first (404), or the war-time treasury (no scrapping/gifting
  of the plan's units during its war); then 101's slow army; then two other warmonger civs.
- **How to wait:** `ai-lab/runs/wait-for-results.sh` in the background, always (rule 12). Before pushing code, fetch and
  merge `origin/ai/v2` so your code commit is the tip (the release is named after the pushed tip, CODEMAP).

## User rulings

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
