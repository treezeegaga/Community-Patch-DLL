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
