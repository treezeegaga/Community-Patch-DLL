-- Scenario list for the siege lab (see README.md next to this file).
--
-- Each scenario stages one battle: an attacking army and a defended city, already at war, on flat open
-- ground the lab clears for itself. The AI plays both sides for `turns` turns and the lab logs what happened.
--
-- Fields:
--   name            unique, no commas
--   turns           how many turns the battle runs before it is scored as "not captured"
--   startDistance   tiles between the army's front row and the city (3 = where a stock operation hands the
--                   army over to the tactical AI). Ignored when atHome is set.
--   atHome          true = the army starts around the attacker's home city, 12 tiles from the target, with no
--                   "just delivered by an operation" mark (the march lab)
--   homeDistance    with atHome: tiles from the attacker's home city to the target (default 12; 4 to 19)
--   attackerTechs / defenderTechs
--                   techs each side has; their prerequisites are added automatically. Everything else is removed.
--   army            { { unit type, count [, xp] }, ... } for the attacker
--   city            population, buildings, hill (city on a hill), garrison (unit type inside the city),
--                   capital (lab v12: the defender's palace moves to it for the battle),
--                   coast (lab v12: sea on its north side out to 4 tiles)
--   defenders       { { unit type, count }, ... } placed next to the city
--   terrain         "open" (default) or "rough" (forest and hills on the approach)
--   era             lab v15: the attacker's era group (default 0). Battles run lowest era first, all repeats of
--                   a group before the next, because an era once reached never goes back down.
--
-- The numbers below are a first grid, not tuned values. Pass bars are set after the stock AI's baseline and
-- the human reference games (README.md, "First run").

local CB = "UNIT_COMPOSITE_BOWMAN"
local SPEAR = "UNIT_SPEARMAN"
local WARRIOR = "UNIT_WARRIOR"
local ARCHER = "UNIT_ARCHER"
local SWORD = "UNIT_SWORDSMAN"
local CATAPULT = "UNIT_CATAPULT"
local TREBUCHET = "UNIT_TREBUCHET"
local CROSSBOW = "UNIT_CROSSBOWMAN"
local LONGSWORD = "UNIT_LONGSWORDSMAN"
local PIKE = "UNIT_PIKEMAN"
local ARTILLERY = "UNIT_ARTILLERY"
local RIFLE = "UNIT_RIFLEMAN"

-- What a Composite Bowman timing attack has researched, and what its victim has.
local A_CLASSICAL = { "TECH_CONSTRUCTION", "TECH_BRONZE_WORKING" }
local A_IRON = { "TECH_CONSTRUCTION", "TECH_BRONZE_WORKING", "TECH_IRON_WORKING", "TECH_MATHEMATICS" }
local A_ANCIENT = { "TECH_ARCHERY", "TECH_BRONZE_WORKING" }
local D_ANCIENT = { "TECH_ARCHERY", "TECH_MASONRY", "TECH_BRONZE_WORKING" }
local D_CLASSICAL = { "TECH_CONSTRUCTION", "TECH_BRONZE_WORKING", "TECH_MASONRY" }

local ARMY_SMALL = { { CB, 3 }, { SPEAR, 2 } }
local ARMY_MEDIUM = { { CB, 5 }, { SPEAR, 2 } }
local ARMY_LARGE = { { CB, 7 }, { SPEAR, 3 } }
-- Lab v16 (user ruling 2026-11-19): a march army the classical-era attack formation can fill
-- (MUFORMATION_SMALL_CITY_ATTACK_FORCE: slots 0-1 ATTACK/DEFENSE or ATTACK/COUNTER, slot 2 ATTACK/FAST_ATTACK, 2 ranged,
-- all required). A Spearman has no ATTACK role here, so it can't take slot 2: the third melee unit is a Swordsman.
-- (v14's version, 3 Spearmen + 4 bowmen, never filled slot 2: m2n3a.) Same size as ARMY_MEDIUM.
local ARMY_MARCH3 = { { CB, 4 }, { SPEAR, 2 }, { SWORD, 1 } }

local function weakCity() return { population = 3 } end
local function mediumCity() return { population = 6, buildings = { "BUILDING_WALLS" }, garrison = ARCHER } end
local function strongCity() return { population = 8, hill = true, buildings = { "BUILDING_WALLS" }, garrison = CB } end

SiegeLabScenarios = {}

SiegeLabScenarios.siege = {
	-- The earliest rush: no walls, nothing but the city itself.
	{ name = "ancient_rush_vs_open_city", turns = 12, startDistance = 3,
	  attackerTechs = A_ANCIENT, defenderTechs = { "TECH_ARCHERY" },
	  army = { { ARCHER, 4 }, { WARRIOR, 3 } },
	  city = { population = 2 } },

	-- Army size against city strength, three by three.
	{ name = "small_vs_weak", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_SMALL, city = weakCity() },
	{ name = "small_vs_medium", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_SMALL, city = mediumCity() },
	{ name = "small_vs_strong", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_CLASSICAL,
	  army = ARMY_SMALL, city = strongCity(), defenders = { { SPEAR, 2 } } },
	{ name = "medium_vs_weak", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM, city = weakCity() },
	{ name = "medium_vs_medium", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM, city = mediumCity() },
	{ name = "medium_vs_strong", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_CLASSICAL,
	  army = ARMY_MEDIUM, city = strongCity(), defenders = { { SPEAR, 2 } } },
	{ name = "large_vs_weak", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_LARGE, city = weakCity() },
	{ name = "large_vs_medium", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_LARGE, city = mediumCity() },
	{ name = "large_vs_strong", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_CLASSICAL,
	  army = ARMY_LARGE, city = strongCity(), defenders = { { SPEAR, 2 } } },

	-- The shape of the user's own Aztec game (HUMAN_GAME.md): about 6 experienced melee and 4 Composite
	-- Bowmen against a walled city, no siege. Warriors stand in for Jaguars; the 30 XP is the barbarian hunting.
	{ name = "ref_cb4_melee6_vs_walls", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = { { CB, 4 }, { WARRIOR, 6, 30 } },
	  city = { population = 6, buildings = { "BUILDING_WALLS" }, garrison = ARCHER } },

	-- Composition probes.
	{ name = "melee_only_vs_medium", turns = 12, startDistance = 3, attackerTechs = A_IRON, defenderTechs = D_ANCIENT,
	  army = { { SWORD, 6 } }, city = mediumCity() },
	{ name = "ranged_only_vs_weak", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = { { CB, 6 } }, city = weakCity() },   -- nothing here can take the city; shows what the AI does then
	{ name = "siege_train_vs_strong", turns = 12, startDistance = 3, attackerTechs = A_IRON, defenderTechs = D_CLASSICAL,
	  army = { { CATAPULT, 2 }, { CB, 3 }, { SWORD, 3 } },
	  city = strongCity(), defenders = { { SPEAR, 2 } } },

	-- The same middle case with forest and hills on the approach.
	{ name = "medium_vs_medium_rough", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM, city = mediumCity(), terrain = "rough" },
}

-- The march lab: the same armies starting at home, 12 tiles away. Run with Install-SiegeLab.ps1 -Set march.
SiegeLabScenarios.march = {
	{ name = "march_medium_vs_weak", turns = 25, atHome = true, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM, city = weakCity() },
	{ name = "march_medium_vs_medium", turns = 25, atHome = true, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM, city = mediumCity() },
	{ name = "march_large_vs_medium", turns = 25, atHome = true, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_LARGE, city = mediumCity() },
	-- The same middle case at other distances (lab v11): the stock AI only takes as attack targets cities it can
	-- reach within the land trade route range (10 on every map size in the Community Patch).
	{ name = "march_medium_vs_medium_d8", turns = 25, atHome = true, homeDistance = 8, attackerTechs = A_CLASSICAL,
	  defenderTechs = D_ANCIENT, army = ARMY_MEDIUM, city = mediumCity() },
	{ name = "march_medium_vs_medium_d10", turns = 25, atHome = true, homeDistance = 10, attackerTechs = A_CLASSICAL,
	  defenderTechs = D_ANCIENT, army = ARMY_MEDIUM, city = mediumCity() },
	{ name = "march_medium_vs_medium_d16", turns = 25, atHome = true, homeDistance = 16, attackerTechs = A_CLASSICAL,
	  defenderTechs = D_ANCIENT, army = ARMY_MEDIUM, city = mediumCity() },
	-- Rough ground in front of the city (PLAN.md M2: "extend with rough terrain"); 12 tiles, as the first three.
	{ name = "march_medium_vs_medium_rough", turns = 25, atHome = true, attackerTechs = A_CLASSICAL,
	  defenderTechs = D_ANCIENT, army = ARMY_MEDIUM, city = mediumCity(), terrain = "rough" },
}

-- Lab v16: the march with ARMY_MARCH3, which fills the classical attack formation's required slots, so the march itself
-- is measured (an army that can't fill them waits in "recruiting" for a unit the lab never builds; LOG.md M2 it. 2).
-- Its own set, so the march set stays as it was. Iron Working for the Swordsman.
SiegeLabScenarios.march3 = {
	{ name = "march3_vs_medium_d8", turns = 25, atHome = true, homeDistance = 8, attackerTechs = A_IRON,
	  defenderTechs = D_ANCIENT, army = ARMY_MARCH3, city = mediumCity() },
	{ name = "march3_vs_medium", turns = 25, atHome = true, attackerTechs = A_IRON, defenderTechs = D_ANCIENT,
	  army = ARMY_MARCH3, city = mediumCity() },
	{ name = "march3_vs_medium_d16", turns = 25, atHome = true, homeDistance = 16, attackerTechs = A_IRON,
	  defenderTechs = D_ANCIENT, army = ARMY_MARCH3, city = mediumCity() },
	{ name = "march3_vs_medium_rough", turns = 25, atHome = true, attackerTechs = A_IRON,
	  defenderTechs = D_ANCIENT, army = ARMY_MARCH3, city = mediumCity(), terrain = "rough" },
}

-- The regression set (user request 2026-11-19, PLAN.md section 8a): cases the main sets don't cover, run on stock
-- and on each new build so a fix for one milestone can't quietly break them. Units lost is reported per scenario.
-- regress_capital and regress_coastal are medium_vs_medium with one thing changed, so they read against it.
local A_MEDIEVAL = { "TECH_PHYSICS", "TECH_MACHINERY", "TECH_STEEL" }
local D_MEDIEVAL = { "TECH_CHIVALRY", "TECH_CIVIL_SERVICE", "TECH_MACHINERY", "TECH_MASONRY" }
local A_ARTILLERY = { "TECH_DYNAMITE", "TECH_RIFLING" }
local D_INDUSTRIAL = { "TECH_RIFLING", "TECH_RAILROAD", "TECH_CHIVALRY", "TECH_MASONRY" }

SiegeLabScenarios.regress = {
	{ name = "regress_capital", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM,
	  city = { population = 6, buildings = { "BUILDING_WALLS" }, garrison = ARCHER, capital = true } },
	{ name = "regress_coastal", turns = 12, startDistance = 3, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT,
	  army = ARMY_MEDIUM,
	  city = { population = 6, buildings = { "BUILDING_WALLS" }, garrison = ARCHER, coast = true } },
	-- Trebuchets, crossbows and longswords against a walled and castled city with pikes outside.
	{ name = "regress_medieval", era = 2, turns = 15, startDistance = 3, attackerTechs = A_MEDIEVAL, defenderTechs = D_MEDIEVAL,
	  army = { { TREBUCHET, 2 }, { CROSSBOW, 4 }, { LONGSWORD, 3 } },
	  city = { population = 8, buildings = { "BUILDING_WALLS", "BUILDING_CASTLE" }, garrison = CROSSBOW },
	  defenders = { { PIKE, 2 } } },
	-- Artillery (range 3) and riflemen against walls, castle and arsenal.
	{ name = "regress_artillery", era = 4, turns = 15, startDistance = 3, attackerTechs = A_ARTILLERY, defenderTechs = D_INDUSTRIAL,
	  army = { { ARTILLERY, 3 }, { RIFLE, 4 } },
	  city = { population = 12, buildings = { "BUILDING_WALLS", "BUILDING_CASTLE", "BUILDING_ARSENAL" }, garrison = RIFLE },
	  defenders = { { RIFLE, 2 } } },
}

-- The opening lab (lab v18, M3): not a list of battles. A normal game start played by the AI to this turn; the lab
-- logs every major civ's economy and army each turn (OPEN lines). Run with -Set opening.
SiegeLabScenarios.opening = { turns = 80 }

-- The transfer check (PLAN 8a item 4): the same opening mode to turn 100, with the new AI on for the player's own civ
-- (Songhai: Askia, an aggressive leader) and the stock AI for the rest; the game's AI logs say whether the lab fixes
-- fire in a real game. Run with -Set transfer -NewAI attacker. Maps are not seeded: each game is a fresh map of the
-- PC's set-up (Tiny, 4 civs).
SiegeLabScenarios.transfer = { turns = 100 }

-- Lab v22 (M3, stalled sieges): full-game sieges copied with their ground and units (make_stall_scenario.py). Kind b1:
-- the ranged units stand where they can't see the city. Population is a guess (the snapshot didn't log it then).
SiegeLabScenarios.stall = {
	-- St. Petersburg on turn 60 of the full game (sd24bN505, build ab-45cd73b, seed 505): city 200 of 200 hp, strength 1585
	{ name = "stall505", turns = 15, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT, ground = { y = 28, map = "-2:-4:f-;-1:-4:f-;0:-4:fF;1:-4:f-;2:-4:fF;-3:-3:h-;-2:-3:f-;-1:-3:f-;0:-3:f-;1:-3:f-;2:-3:h-;-3:-2:fF;-2:-2:f-;-1:-2:f-;0:-2:fF;1:-2:fF;2:-2:f-;3:-2:f-;-4:-1:f-;-3:-1:f-;-2:-1:f-;-1:-1:fF;0:-1:fF;1:-1:fF;2:-1:f-;3:-1:f-;-4:0:fF;-3:0:fF;-2:0:f-;-1:0:hF;0:0:h-c;1:0:f-;2:0:h-;3:0:f-;4:0:f-;-4:1:f-;-3:1:h-;-2:1:w-;-1:1:w-;0:1:w-;1:1:f-;2:1:h-;3:1:fF;-3:2:w-;-2:2:w-;-1:2:w-;0:2:w-;1:2:fF;2:2:f-;3:2:f-;-3:3:w-;-2:3:w-;-1:3:w-;0:3:w-;1:3:w-;2:3:w-;-2:4:w-;-1:4:w-;0:4:w-;1:4:w-;2:4:w-" },
	  city = { population = 6, buildings = {  }, garrison = "UNIT_WARRIOR" },
	  armyAt = { { "UNIT_SPEARMAN", -1, -4, 100 }, { "UNIT_WARRIOR", -2, -3, 100 }, { "UNIT_SPEARMAN", -1, -3, 100 }, { "UNIT_COMPOSITE_BOWMAN", -2, -2, 100 }, { "UNIT_COMPOSITE_BOWMAN", -1, -2, 64 }, { "UNIT_COMPOSITE_BOWMAN", 0, -2, 100 }, { "UNIT_COMPOSITE_BOWMAN", -2, -1, 100 } },
	  defendersAt = { { "UNIT_SPEARMAN", -4, 0, 100 } } },
	-- Attila's Court on turn 56 of the full game (sd24bN1616, build ab-45cd73b, seed 1616; kind b2: Warriors spent on the walls): city 200 of 200 hp, strength 1496
	{ name = "stall1616", turns = 15, attackerTechs = A_CLASSICAL, defenderTechs = D_ANCIENT, ground = { y = 26, map = "-2:-4:h-r;-1:-4:f-;0:-4:hF;1:-4:f-;2:-4:fF;-3:-3:fFr;-2:-3:f-r;-1:-3:f-;0:-3:f-;1:-3:f-;2:-3:fF;-3:-2:f-r;-2:-2:fFr;-1:-2:f-r;0:-2:f-;1:-2:f-;2:-2:f-;3:-2:fM;-4:-1:h-;-3:-1:fPr;-2:-1:f-r;-1:-1:f-r;0:-1:f-;1:-1:f-;2:-1:h-;3:-1:fM;-4:0:f-;-3:0:m-;-2:0:hFr;-1:0:h-r;0:0:f-c;1:0:fF;2:0:f-;3:0:fM;4:0:fM;-4:1:h-;-3:1:f-r;-2:1:m-r;-1:1:m-;0:1:h-;1:1:fM;2:1:fM;3:1:fM;-3:2:f-;-2:2:f-r;-1:2:h-r;0:2:h-;1:2:h-;2:2:fF;3:2:f-;-3:3:h-r;-2:3:f-r;-1:3:fF;0:3:h-;1:3:m-;2:3:m-;-2:4:f-r;-1:4:f-r;0:4:h-;1:4:h-;2:4:h-" },
	  city = { population = 4, buildings = {  }, garrison = "UNIT_HUN_BATTERING_RAM" },
	  armyAt = { { "UNIT_WARRIOR", -4, -1, 100 }, { "UNIT_WARRIOR", -3, -1, 100 }, { "UNIT_COMPOSITE_BOWMAN", -2, -1, 100 }, { "UNIT_COMPOSITE_BOWMAN", -4, 0, 100 }, { "UNIT_WARRIOR", -2, 0, 100 }, { "UNIT_COMPOSITE_BOWMAN", -3, 1, 100 } },
	  defendersAt = {  } },
}
