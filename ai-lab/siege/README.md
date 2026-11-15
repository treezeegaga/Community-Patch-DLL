# Siege lab

A test for one question: **given an army at an enemy city's walls, does the AI take the city?**

It stages short battles inside a running game (an attacking army, a defended city, already at war), lets
the AI play both sides for about 12 turns each, and logs what happened. A run of 30 battles is one game
launch and should take minutes, instead of judging the same thing through whole 150-turn games.

This is rung 1 of the redesigned AI experiment. The same script also runs rung 2 (the march lab: the army
starts at home, 12 tiles away) with `-Set march`.

**Status: not yet run in the game.** The script's own logic passes a mock run outside the game
(`lua5.1 test/mock_run.lua`), and every engine call was checked against the DLL's Lua bindings, but the two
PowerShell scripts and the behaviour inside Civ V are untested. The first run is there to prove the lab
itself; see "First run".

## Files

| File | What |
|---|---|
| `SiegeLab.lua` | The in-game script: builds the arena, stages each battle, scores each turn |
| `SiegeLabScenarios.lua` | The battles: armies, cities, techs, terrain. Edit this to add or change scenarios |
| `Install-SiegeLab.ps1` | Puts the lab into the installed pack, or removes it (`-Remove`) |
| `Get-SiegeLabResults.ps1` | Reads the results out of `Lua.log` into CSV files and prints a summary |
| `test/mock_run.lua` | Runs the script against a fake game, to check changes to the lab without Civ V |

Nothing in the DLL or the database changes. The lab works with any DLL build, so the same scenarios can be
run on stock Community Patch and on every candidate.

## Running it

On a PC with the "CP with Animated Leaders" pack installed and Civ V closed:

```
powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 -EnableLogging
```

Then start Civ V and a new single-player game:

- 2 or more civs (2 is fastest; everyone but you and the next civ is removed at the start anyway)
- any map of Tiny size or larger, King, Quick pace
- **Quick Combat** and **Quick Movement** on
- click through to the map ("Begin Your Journey")

The lab takes over from there: the AI takes over your civ and the battles play themselves. When it is
done (a `[SiegeLab] DONE` line in `Logs\Lua.log`) the game hands your civ back to you a few turns later.
Close Civ V, then:

```
powershell -ExecutionPolicy Bypass -File ai-lab\siege\Get-SiegeLabResults.ps1 -Label stock548
powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 -Remove
```

Read the results **before** starting Civ V again (the game overwrites `Lua.log` on launch), and **remove
the lab before playing**: while it is installed, every new single-player game is taken over. It refuses to
start in multiplayer, on a loaded save, or after turn 0.

Options for the installer: `-Set march`, `-Only small_vs_weak,large_vs_medium`, `-Repeats 3`, and `-Human`
(you play the attacker; end turn 0 and the first battle appears).

## What a battle looks like

- The lab flattens a 25 by 13 rectangle of grassland where the map has the most land.
- The attacker (your civ's slot) has a home city at its west end. The target city is 12 tiles east.
- The defender (the next civ) also has a reserve capital far away, ringed with mountains. It keeps the
  defender alive when the target falls, and stops a capture counting toward a domination victory.
- Siege scenarios place the army 3 tiles from the city, melee in front, marked as "just delivered by an
  operation": the state in which the stock AI hands an army over to its tactical AI.
- Each turn, both sides are held still outside the battle: treasury reset to 60 gold, culture and faith to
  0, city production to 0, and any unit that wasn't staged is removed. So no reinforcements, purchases,
  upgrades or policies.
- A battle ends when the city falls, the attackers are all dead, or its turns run out. Then everything is
  wiped and the next one is staged.

## What is logged

`Get-SiegeLabResults.ps1` writes two files into `results\`.

`*.results.csv`, one row per battle:

| Column | Meaning |
|---|---|
| `Captured`, `CaptureTurn` | 1 if the city fell, and on which turn of the battle |
| `CityMaxHP`, `MaxDamage`, `FinalDamage` | The city's hit points, the most damage it carried, and its damage at the end |
| `CityStrength` | The city's combat strength at the start, times 100 |
| `AttackersStart`, `AttackersAlive`, `AttackersKilledByEnemy` | Army size, survivors, and deaths the game attributed to the defender |
| `DefendersStart`, `DefendersAlive`, `DefendersKilledByEnemy` | The same for the garrison and field units |
| `EngagedTurns`, `IdleTurns` | Turns on which an attacking unit gained experience or the city ended more damaged, and turns on which neither happened |
| `MinMeanDistance`, `FinalMeanDistance` | The army's average distance to the city at its closest, and at the end (rising = it walked away) |
| `AttackerGoldRate` | The attacker's gold per turn at the start (to see whether upkeep could be distorting it) |

`*.turns.csv`, one row per battle turn: the city's damage before the attacker moved and after (after the
city's own heal), attackers alive, attackers within 2 tiles, average distance, attackers that gained
experience that turn, defenders alive. `Late = 1` marks a turn that could only be scored a turn later, so
its numbers also include the defender's moves.

## First run: proving the lab

Before any result means anything, check on a stock build:

1. **No `ERROR` or `LUA ERROR` lines**, and a `DONE` line.
2. **Unit counts.** `AttackersStart` and `DefendersStart` match the scenario list.
3. **It looks right.** Watch the first two battles: a flat arena, the army 3 tiles west of the city, both
   sides at war, units moving.
4. **Repeats agree roughly.** Each scenario is played twice; wildly different outcomes for the same
   scenario mean something leaks between battles.
5. **The human reference.** Run `-Human -Only ref_cb4_melee6_vs_walls,medium_vs_medium,small_vs_medium -Repeats 1`
   and play them. What you can take, the AI should take; what you can't, it shouldn't be blamed for.

The scenario list is a first grid, not tuned values, and it has no pass bars yet. The bars come from this
baseline and the human reference.

## Known limits and things to watch

- **Long runs drift.** A full run is about 360 game turns in one game. Techs are reset at every battle, but
  a side can research an early tech during one, and war weariness may build up (each side gets +20
  happiness per city to cover it). If repeats disagree, shorten runs with `-Only`.
- **The city in a siege scenario is never the defender's capital**, so it has no Palace.
- **Upkeep.** Both sides have one small city, so a large army runs a deficit; the treasury reset covers
  it, but the AI may still react to a negative income. `AttackerGoldRate` shows how negative.
- **Removed units.** A Great General earned in a battle is removed like any other unstaged unit.
- **Scoring uses experience gains** to tell whether units fought. If a unit can fight without gaining
  experience, `EngagedTurns` undercounts; `MaxDamage` doesn't depend on it.
- **Game length.** The lab extends the game's turn limit and switches every victory off, so the game
  can't end mid-run.
