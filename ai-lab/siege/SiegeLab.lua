-- Siege lab: stages short battles in a running game, lets the AI play them, and logs the outcome to Lua.log.
-- See README.md next to this file. Loaded as an in-game UI add-in by Install-SiegeLab.ps1; does nothing
-- unless SiegeLabConfig.lua (written by the installer) says enabled = true.
--
-- How a run goes:
--   1. When the load screen closes on turn 0 of a single-player game, the lab clears a flat arena, gives the
--      attacker (the player's own civ) a home city and the defender (the next civ) a walled-off reserve
--      capital, removes every other civ, city-state and barbarian, puts the two at permanent war and, unless
--      Human is set, starts autoplay so the AI plays the attacker too.
--   2. At the start of each of the attacker's turns it scores the turn that just ended, and when a scenario
--      is over it wipes both armies and the target city and stages the next one.
--   3. Results go to Lua.log as lines starting with "[SiegeLab] " (RUN, START, TURN, RESULT, WARN, ERROR, DONE).
--
-- Every engine call below was checked against the DLL's Lua bindings (CvGameCoreDLL_Expansion2/Lua), but the
-- lab had not been run in the game when this was written. Arguments are always passed in full: the bindings
-- read a missing argument as 0 or false, not as the C++ default.

include("SiegeLabScenarios")
include("SiegeLabConfig")

local VERSION = 14
local TAG = "[SiegeLab] "

local PLOT_MOUNTAIN, PLOT_HILLS, PLOT_LAND, PLOT_OCEAN = 0, 1, 2, 3   -- PlotTypes in CvEnums.h
local NO_PLAYER = -1
local NO_FLOWDIRECTION = 7   -- FlowDirectionTypes in CvEnums.h: the six directions, a count, then this (not -1)

-- The arena: a rectangle of flat grassland. The target city sits TARGET_DX east of its centre and the
-- attacker's home city HOME_DX west of it, 12 tiles apart on the same row.
local HALF_W, HALF_H = 12, 6
local TARGET_DX, HOME_DX = 3, -9
local TREASURY = 60   -- both sides are reset to this each turn: too little to buy or upgrade anything
local GOLD_RATE = 10  -- both sides' income per turn is held here, whatever their units cost

local lab = nil       -- state of the run; nil until the lab starts
local cur = nil       -- state of the scenario being played

local function out(line)
	print(TAG .. line)
end

local function typeID(name)
	return GameInfoTypes[name]
end

local function maxMajors() return GameDefines.MAX_MAJOR_CIVS or 22 end
local function maxPlayers() return GameDefines.MAX_PLAYERS or 64 end

------------------------------------------------------------------------------------------------------------
-- Scenario checks and the run queue
------------------------------------------------------------------------------------------------------------

local function checkNames(list, what, problems)
	for _, name in ipairs(list or {}) do
		if typeID(name) == nil then
			problems[#problems + 1] = what .. " " .. tostring(name)
		end
	end
end

-- Returns nil if the scenario can be staged, or a string naming what the database doesn't have.
local function scenarioProblem(sc)
	local problems = {}
	if type(sc.name) ~= "string" or sc.name:find(",") then problems[#problems + 1] = "name" end
	if type(sc.turns) ~= "number" or sc.turns < 1 then problems[#problems + 1] = "turns" end
	if type(sc.army) ~= "table" or #sc.army == 0 then problems[#problems + 1] = "army" end
	if type(sc.city) ~= "table" then problems[#problems + 1] = "city" end
	checkNames(sc.attackerTechs, "tech", problems)
	checkNames(sc.defenderTechs, "tech", problems)
	for _, entry in ipairs(sc.army or {}) do checkNames({ entry[1] }, "unit", problems) end
	for _, entry in ipairs(sc.defenders or {}) do checkNames({ entry[1] }, "unit", problems) end
	if sc.city then
		checkNames(sc.city.buildings, "building", problems)
		if sc.city.garrison then checkNames({ sc.city.garrison }, "unit", problems) end
	end
	if #problems > 0 then return table.concat(problems, "; ") end
	return nil
end

local function buildQueue(config)
	local set = SiegeLabScenarios and SiegeLabScenarios[config.set or "siege"]
	if not set then
		out("ERROR,no scenario set named " .. tostring(config.set))
		return {}
	end
	local only = nil
	if config.only and #config.only > 0 then
		only = {}
		for _, name in ipairs(config.only) do only[name] = true end
	end
	local chosen = {}
	for _, sc in ipairs(set) do
		if not only or only[sc.name] then
			local problem = scenarioProblem(sc)
			if problem then
				out("WARN,scenario " .. tostring(sc.name) .. " skipped: " .. problem)
			else
				chosen[#chosen + 1] = sc
			end
		end
	end
	-- Repeats are the outer loop, so anything that drifts over a long run affects every scenario alike.
	local queue = {}
	for rep = 1, (config.repeats or 1) do
		for _, sc in ipairs(chosen) do
			queue[#queue + 1] = { sc = sc, rep = rep }
		end
	end
	return queue, #chosen
end

------------------------------------------------------------------------------------------------------------
-- Terrain
------------------------------------------------------------------------------------------------------------

local function clearPlot(plot)
	plot:SetFeatureType(-1)
	plot:SetResourceType(-1, 0, false)
	plot:SetImprovementType(-1, NO_PLAYER, false)
	plot:SetRouteType(-1, NO_PLAYER)
	if plot:IsWOfRiver() then plot:SetWOfRiver(false, NO_FLOWDIRECTION) end
	if plot:IsNWOfRiver() then plot:SetNWOfRiver(false, NO_FLOWDIRECTION) end
	if plot:IsNEOfRiver() then plot:SetNEOfRiver(false, NO_FLOWDIRECTION) end
end

local function setLand(plot, plotType, terrainName)
	clearPlot(plot)
	plot:SetPlotType(plotType, false, true, false)
	plot:SetTerrainType(typeID(terrainName), false, true)
end

local function forEachArenaPlot(fn)
	for dy = -HALF_H, HALF_H do
		for dx = -HALF_W, HALF_W do
			local plot = Map.GetPlot(lab.cx + dx, lab.cy + dy)
			if plot then fn(plot) end
		end
	end
end

-- The arena goes where the map already has the most open land, so as little as possible is rebuilt.
local function pickArenaCentre()
	local w, h = Map.GetGridSize()
	local bestX, bestY, bestScore = nil, nil, -1
	for cy = HALF_H + 2, h - HALF_H - 3, 2 do
		for cx = HALF_W + 2, w - HALF_W - 3, 2 do
			local score = 0
			for dy = -HALF_H, HALF_H do
				for dx = -HALF_W, HALF_W do
					local plot = Map.GetPlot(cx + dx, cy + dy)
					if plot and not plot:IsWater() and not plot:IsMountain() then score = score + 1 end
				end
			end
			if score > bestScore then bestX, bestY, bestScore = cx, cy, score end
		end
	end
	return bestX, bestY
end

-- The defender's capital: as far from the arena as the map allows and ringed with mountains, so nothing can
-- reach it. It keeps the defender alive when the target city falls, and it is the defender's original
-- capital, so a captured target never counts toward a domination victory.
local function buildReserveSite()
	local w, h = Map.GetGridSize()
	local best, bestDist = nil, -1
	for y = 3, h - 4 do
		for x = 3, w - 4 do
			local plot = Map.GetPlot(x, y)
			if plot and not plot:IsWater() then
				local d = Map.PlotDistance(x, y, lab.cx, lab.cy)
				if d > bestDist then best, bestDist = plot, d end
			end
		end
	end
	if not best then return nil end
	local x, y = best:GetX(), best:GetY()
	for dy = -1, 1 do
		for dx = -1, 1 do
			local plot = Map.GetPlot(x + dx, y + dy)
			if plot and Map.PlotDistance(x, y, plot:GetX(), plot:GetY()) == 1 then
				setLand(plot, PLOT_MOUNTAIN, "TERRAIN_MOUNTAIN")
			end
		end
	end
	setLand(best, PLOT_LAND, "TERRAIN_GRASS")
	if bestDist < 2 * HALF_W then
		out("WARN,the reserve capital is only " .. bestDist .. " tiles from the arena centre; use a larger map")
	end
	return best
end

-- Puts the arena back to flat grass where the last scenario changed it, then applies this scenario's terrain.
local function applyScenarioTerrain(sc)
	for _, plot in ipairs(lab.changedPlots) do
		setLand(plot, PLOT_LAND, "TERRAIN_GRASS")
	end
	lab.changedPlots = {}
	local tx, ty = lab.tx, lab.ty
	clearPlot(lab.targetPlot)   -- also removes the ruins of the last target city
	if sc.city.hill then
		setLand(lab.targetPlot, PLOT_HILLS, "TERRAIN_GRASS")
		lab.changedPlots[#lab.changedPlots + 1] = lab.targetPlot
	end
	if sc.terrain == "rough" then
		-- Hills and forest alternate along rings 2 and 3 on the attacker's (west) side. The rings are walked with
		-- hex directions from the city, so the layout is the same wherever the arena lands on the map (lab v6
		-- went by map coordinates, and each launch got a different layout; LOG.md M0 iteration 7).
		local forest = typeID("FEATURE_FOREST")
		for k = 2, 3 do
			local x, y = tx, ty
			for _ = 1, k do
				local p = Map.PlotDirection(x, y, 4)   -- start k tiles due west
				x, y = p:GetX(), p:GetY()
			end
			local i = 0
			for side = 0, 5 do                          -- NE, E, SE, SW, W, NW: back round to the start
				for j = 1, k do
					local p = Map.PlotDirection(x, y, side)
					x, y = p:GetX(), p:GetY()
					i = i + 1
					local west = side == 0 or side == 5 or (side == 1 and j <= k / 2) or (side == 4 and j > k / 2)
					if west then
						if i % 2 == 0 then
							setLand(p, PLOT_HILLS, "TERRAIN_GRASS")
						elseif forest then
							p:SetFeatureType(forest)
						end
						lab.changedPlots[#lab.changedPlots + 1] = p
					end
				end
			end
		end
	end
	if sc.city.coast then
		-- Lab v12: sea on the city's north side, out to 4 tiles, so the city is on the coast (a body of water
		-- this size counts as sea, not a lake). The attacker's row and the defenders' east side stay land.
		local coast = typeID("TERRAIN_COAST")
		forEachArenaPlot(function(p)
			local d = Map.PlotDistance(p:GetX(), p:GetY(), tx, ty)
			if d >= 1 and d <= 4 and p:GetY() > ty then
				clearPlot(p)
				p:SetPlotType(PLOT_OCEAN, false, true, false)
				p:SetTerrainType(coast, false, true)
				lab.changedPlots[#lab.changedPlots + 1] = p
			end
		end)
	end
	Map.RecalculateAreas()
end

------------------------------------------------------------------------------------------------------------
-- Techs
------------------------------------------------------------------------------------------------------------

local g_prereqs = nil

local function techSet(names)
	if not g_prereqs then
		g_prereqs = {}
		for row in GameInfo.Technology_PrereqTechs() do
			local list = g_prereqs[row.TechType] or {}
			list[#list + 1] = row.PrereqTech
			g_prereqs[row.TechType] = list
		end
	end
	local wanted = {}
	local function add(name)
		local id = typeID(name)
		if id == nil or wanted[id] then return end
		wanted[id] = true
		for _, prereq in ipairs(g_prereqs[name] or {}) do add(prereq) end
	end
	add("TECH_AGRICULTURE")
	for _, name in ipairs(names or {}) do add(name) end
	return wanted
end

-- No research during a run: progress kept on a removed tech made it come back at once in a later battle, so a
-- side climbed eras over a run (removing techs doesn't lower the era), and from three eras up the AI scraps its
-- Spearmen and Warriors as "long obsolete" (CvEconomicAI::DisbandLongObsoleteUnits; seen in m0a-c, LOG.md M0
-- iteration 6). Called at every battle and at the start of every turn.
local function clearResearch(playerID)
	local player = Players[playerID]
	local teamTechs = Teams[player:GetTeam()]:GetTeamTechs()
	for tech in GameInfo.Technologies() do
		if teamTechs:GetResearchProgress(tech.ID) ~= 0 then teamTechs:SetResearchProgress(tech.ID, 0, playerID) end
	end
	player:SetOverflowResearch(0)
end

local function setTechs(playerID, names)
	local team = Teams[Players[playerID]:GetTeam()]
	local wanted = techSet(names)
	for tech in GameInfo.Technologies() do
		local want = wanted[tech.ID] == true
		if team:IsHasTech(tech.ID) ~= want then
			team:SetHasTech(tech.ID, want, playerID, false, false, true)
		end
	end
	clearResearch(playerID)
end

------------------------------------------------------------------------------------------------------------
-- Units
------------------------------------------------------------------------------------------------------------

local function isUsable(unit)
	return unit ~= nil and not unit:IsDead() and not unit:IsDelayedDeath()
end

local function killUnitsOf(player, keep)
	local doomed = {}
	for unit in player:Units() do
		if not keep or not keep[unit:GetID()] then doomed[#doomed + 1] = unit end
	end
	for _, unit in ipairs(doomed) do unit:Kill(false, NO_PLAYER) end
	return #doomed
end

-- Empty arena plots between minD and maxD tiles from (x, y), nearest first, then closest to the row the two
-- cities share. side = "west" keeps the attacker's side of the target, "east" the far side, nil both.
local function openPlots(x, y, minD, maxD, side)
	local found = {}
	forEachArenaPlot(function(plot)
		local px, py = plot:GetX(), plot:GetY()
		local d = Map.PlotDistance(px, py, x, y)
		if d >= minD and d <= maxD and not plot:IsCity() and not plot:IsWater() and not plot:IsMountain()
			and plot:GetNumUnits() == 0
			and (side == nil or (side == "west" and px < x) or (side == "east" and px > x)) then
			found[#found + 1] = { plot = plot, d = d, dy = math.abs(py - y), x = px }
		end
	end)
	table.sort(found, function(a, b)
		if a.d ~= b.d then return a.d < b.d end
		if a.dy ~= b.dy then return a.dy < b.dy end
		return a.x > b.x
	end)
	return found
end

-- Creates the units of a list of { type, count [, xp] } entries on the given plots, melee in the front
-- plots and ranged behind. Returns the roster: unit ID -> "melee" or "ranged".
local function placeUnits(playerID, entries, plots, justDeployed)
	local player = Players[playerID]
	local melee, ranged = {}, {}
	for _, entry in ipairs(entries or {}) do
		local id = typeID(entry[1])
		local info = GameInfo.Units[id]
		for _ = 1, entry[2] do
			local slot = { id = id, xp = entry[3], kind = "melee" }
			if info and (info.Range or 0) > 0 then
				slot.kind = "ranged"
				ranged[#ranged + 1] = slot
			else
				melee[#melee + 1] = slot
			end
		end
	end
	local order = {}
	for _, slot in ipairs(melee) do order[#order + 1] = slot end
	for _, slot in ipairs(ranged) do order[#order + 1] = slot end

	local roster, placed = {}, 0
	for i, slot in ipairs(order) do
		local spot = plots[i]
		if not spot then break end
		local unit = player:InitUnit(slot.id, spot.plot:GetX(), spot.plot:GetY())
		if unit then
			if slot.xp then unit:SetExperience(slot.xp) end
			if justDeployed then unit:SetDeployFromOperationTurn(Game.GetGameTurn()) end
			roster[unit:GetID()] = slot.kind
			placed = placed + 1
		end
	end
	if placed < #order then
		out("WARN," .. cur.sc.name .. ": placed " .. placed .. " of " .. #order .. " units for player " .. playerID)
	end
	return roster, placed
end

local function rosterAlive(playerID, roster)
	local player = Players[playerID]
	local alive = {}
	for id in pairs(roster) do
		local unit = player:GetUnitByID(id)
		if isUsable(unit) then alive[#alive + 1] = unit end
	end
	return alive
end

-- How many of a roster's melee units (the only ones that can take a city) are still alive.
local function meleeAlive(playerID, roster)
	local player = Players[playerID]
	local n = 0
	for id, kind in pairs(roster) do
		if kind == "melee" and isUsable(player:GetUnitByID(id)) then n = n + 1 end
	end
	return n
end

------------------------------------------------------------------------------------------------------------
-- Keeping the two sides still between scenarios and turns
------------------------------------------------------------------------------------------------------------

-- Unit upkeep rises with the turn count, so over a long run an army that broke even in the first battle
-- costs 60 gold a turn by the last, and the AI starts disbanding units and holding back. To keep every
-- battle on the same footing, each side's permanent city gets whatever extra gold makes the side's income
-- GOLD_RATE. One step per call: called every turn, it follows the army as units die.
local function balanceIncome(playerID)
	local player = Players[playerID]
	local city = lab.incomePlot[playerID]:GetPlotCity()
	if not city or city:GetOwner() ~= playerID then return end
	local extra = (lab.extraGold[playerID] or 0) + GOLD_RATE - player:CalculateGoldRate()
	if extra < 0 then extra = 0 end
	local change = extra - (lab.extraGold[playerID] or 0)
	if change ~= 0 then
		city:ChangeBaseYieldRateFromMisc(GameInfoTypes.YIELD_GOLD or 2, change)
		lab.extraGold[playerID] = extra
	end
end

-- Run at the start of a side's turn: no purchases or upgrades, no policies or pantheons, nothing finishes
-- in a city, income is levelled, and anything that isn't part of the staged battle is removed.
local function holdStill(playerID, roster)
	local player = Players[playerID]
	balanceIncome(playerID)
	player:SetGold(TREASURY)
	player:SetJONSCulture(0)
	player:SetFaith(0)
	clearResearch(playerID)
	for city in player:Cities() do
		city:SetProduction(0)
		city:SetOverflowProduction(0)
	end
	lab.cleaning = true
	killUnitsOf(player, roster)
	lab.cleaning = false
end

local function ensureWar()
	local attackerTeamID = Players[lab.attacker]:GetTeam()
	local defenderTeamID = Players[lab.defender]:GetTeam()
	local attackerTeam = Teams[attackerTeamID]
	attackerTeam:Meet(defenderTeamID, true)
	if not attackerTeam:IsAtWar(defenderTeamID) then
		attackerTeam:SetPermanentWarPeace(defenderTeamID, false)
		Teams[defenderTeamID]:SetPermanentWarPeace(attackerTeamID, false)
		attackerTeam:DeclareWar(defenderTeamID, false, lab.attacker)
	end
	if attackerTeam:IsAtWar(defenderTeamID) then
		attackerTeam:SetPermanentWarPeace(defenderTeamID, true)
		Teams[defenderTeamID]:SetPermanentWarPeace(attackerTeamID, true)
		return true
	end
	return false
end

------------------------------------------------------------------------------------------------------------
-- One scenario
------------------------------------------------------------------------------------------------------------

local function targetCity()
	return lab.targetPlot:GetPlotCity()
end

local function beginScenario()
	local entry = lab.queue[lab.index]
	local sc = entry.sc
	local attacker, defender = Players[lab.attacker], Players[lab.defender]
	cur = { sc = sc, rep = entry.rep, t = 0, startTurn = Game.GetGameTurn(), captured = false, captureTurn = -1,
	        maxDamage = 0, idleTurns = 0, engagedTurns = 0, minMeanDist = -1, lastMeanDist = -1,
	        firstArrival = -1, readyTurn = -1,
	        killedByEnemy = { [lab.attacker] = 0, [lab.defender] = 0 }, counted = {}, sampled = true }

	-- Wipe what the last scenario left.
	lab.cleaning = true
	killUnitsOf(attacker, nil)
	killUnitsOf(defender, nil)
	local old = targetCity()
	if old then Players[old:GetOwner()]:Disband(old) end
	lab.cleaning = false

	-- Lab v13: end both sides' AI operations. Otherwise an attack operation aimed at the last battle's city lives on
	-- and recruits the next battle's army for a city that is no longer there (m2n1a: the 10-tile march battle's army
	-- went to the 8-tile battle's operation and sat at home). Needs a DLL with Player:LabEndOperations (ab-<commit>
	-- from 2026-11-19 on); with an older DLL the lab warns once per battle.
	for _, p in ipairs({ attacker, defender }) do
		if p.LabEndOperations then
			local n = p:LabEndOperations()
			if n > 0 then out("NOTE," .. sc.name .. ": ended " .. n .. " leftover AI operation(s) of player " .. p:GetID()) end
		else
			out("WARN," .. sc.name .. ": this DLL can't end leftover AI operations (no Player:LabEndOperations)")
		end
	end

	-- The reserve city must be the defender's capital again after a capital battle (lab v12).
	local reserve = lab.reservePlot and lab.reservePlot:GetPlotCity()
	if reserve and not reserve:IsCapital() then
		reserve:SetNumRealBuilding(typeID("BUILDING_PALACE"), 1)
		out("WARN," .. sc.name .. ": the palace was put back in the reserve city by the lab")
	end

	-- Where the target city stands: 12 tiles east of the attacker's home (the arena's fixed spot), or for a march
	-- scenario homeDistance tiles (lab v11). Moving it leaves the old spot's ruins to clear.
	local wantTx = lab.hx + ((sc.atHome and sc.homeDistance) or (TARGET_DX - HOME_DX))
	if wantTx ~= lab.tx then
		clearPlot(lab.targetPlot)
		lab.tx = wantTx
		lab.targetPlot = Map.GetPlot(lab.tx, lab.ty)
	end

	applyScenarioTerrain(sc)
	setTechs(lab.attacker, sc.attackerTechs)
	setTechs(lab.defender, sc.defenderTechs)

	-- The target city.
	local city = defender:InitCity(lab.tx, lab.ty)
	if not city then
		out("ERROR," .. sc.name .. ": the target city could not be founded")
		cur.failed = true
		return
	end
	city:SetPopulation(sc.city.population or 1, true)
	for _, name in ipairs(sc.city.buildings or {}) do
		city:SetNumRealBuilding(typeID(name), 1)
	end
	if sc.city.capital then
		-- Lab v12: the target is the defender's capital for this battle. The palace moves here from the reserve
		-- city; when the target falls or is wiped, the game hands it back to the reserve (CvCity::kill ->
		-- findNewCapital), and the next battle checks that it did.
		local palace = typeID("BUILDING_PALACE")
		local reserve = lab.reservePlot:GetPlotCity()
		if reserve then reserve:SetNumRealBuilding(palace, 0) end
		city:SetNumRealBuilding(palace, 1)
		if not city:IsCapital() then out("WARN," .. sc.name .. ": the target city did not become the capital") end
	end
	city:SetDamage(0, true)

	-- The defenders: one unit inside the city, the rest next to it on the far side.
	cur.defRoster = {}
	if sc.city.garrison then
		local unit = defender:InitUnit(typeID(sc.city.garrison), lab.tx, lab.ty)
		if unit then cur.defRoster[unit:GetID()] = "garrison" end
	end
	local fieldRoster = placeUnits(lab.defender, sc.defenders, openPlots(lab.tx, lab.ty, 1, 2, "east"), false)
	for id in pairs(fieldRoster) do cur.defRoster[id] = true end

	-- The attackers: in front of the city as if an operation had just delivered them, or around home.
	local plots
	if sc.atHome then
		plots = openPlots(lab.hx, lab.hy, 1, 3, nil)
	else
		local d = sc.startDistance or 3
		plots = openPlots(lab.tx, lab.ty, d, d + 2, "west")
	end
	cur.attRoster = placeUnits(lab.attacker, sc.army, plots, not sc.atHome)

	cur.attStart, cur.defStart, cur.meleeStart = 0, 0, 0
	for _, kind in pairs(cur.attRoster) do
		cur.attStart = cur.attStart + 1
		if kind == "melee" then cur.meleeStart = cur.meleeStart + 1 end
	end
	for _ in pairs(cur.defRoster) do cur.defStart = cur.defStart + 1 end

	-- The attacker knows where its target is, as after a real march: the city's tile and its ring are shown to the
	-- attacker's team for a moment, which records the new city on its map. Without this, on a new map the AI's
	-- tactical layer had no zone for the city in the first battles (diag5). Live sight of the city is not given:
	-- getting a unit within sight of it is the AI's job.
	local attackerTeamID = attacker:GetTeam()
	local reveal = { Map.GetPlot(lab.tx, lab.ty) }
	for dir = 0, 5 do reveal[#reveal + 1] = Map.PlotDirection(lab.tx, lab.ty, dir) end
	for _, plot in ipairs(reveal) do
		if plot then
			plot:ChangeVisibilityCount(attackerTeamID, 1, -1, false, false)
			plot:ChangeVisibilityCount(attackerTeamID, -1, -1, false, false)
		end
	end

	if not ensureWar() then
		out("ERROR," .. sc.name .. ": the two civs could not be put at war")
		cur.failed = true
		return
	end

	balanceIncome(lab.attacker)
	balanceIncome(lab.defender)
	city:UpdateStrengthValue()
	cur.cityMaxHP = city:GetMaxHitPoints()
	out(string.format("START,%s,%d,%d,%d,%d,%d,%d,%d", sc.name, cur.rep, Game.GetGameTurn(),
		city:GetStrengthValue(), cur.cityMaxHP, cur.attStart, cur.defStart,
		attacker:CalculateGoldRate()))
end

-- Called at the start of each attacker turn of a scenario, before its units move.
local function beginTurn()
	cur.t = cur.t + 1
	cur.sampled = false
	local city = targetCity()
	cur.damageBefore = (city and city:GetOwner() == lab.defender) and city:GetDamage() or -1
	cur.xp = {}
	for _, unit in ipairs(rosterAlive(lab.attacker, cur.attRoster)) do
		cur.xp[unit:GetID()] = unit:GetExperience()
	end
end

-- Scores the attacker's turn. Normally called at the start of the defender's turn, right after the attacker
-- moved (late = 0). The city has already healed for the defender's new turn by then, so damageAfter is the
-- damage left after that heal. If the defender's turn doesn't come between two attacker turns, it is called
-- at the next attacker turn instead (late = 1) and the numbers also include the defender's moves.
local function sampleTurn(late)
	if not cur or cur.sampled or cur.t == 0 then return end
	cur.sampled = true
	local city = targetCity()
	if not city or city:GetOwner() ~= lab.defender then
		if not cur.captured then
			cur.captured = true
			cur.captureTurn = cur.t
		end
	end
	local damageAfter = -1
	if not cur.captured and city then
		damageAfter = city:GetDamage()
		if damageAfter > cur.maxDamage then cur.maxDamage = damageAfter end
	end
	if cur.damageBefore > cur.maxDamage then cur.maxDamage = cur.damageBefore end

	local alive = rosterAlive(lab.attacker, cur.attRoster)
	local fought, within2, distSum = 0, 0, 0
	local within3, meleeWithin3, rangedWithin3 = 0, 0, 0
	for _, unit in ipairs(alive) do
		local d = Map.PlotDistance(unit:GetX(), unit:GetY(), lab.tx, lab.ty)
		distSum = distSum + d
		if d <= 2 then within2 = within2 + 1 end
		if d <= 3 then
			within3 = within3 + 1
			if cur.attRoster[unit:GetID()] == "melee" then meleeWithin3 = meleeWithin3 + 1
			else rangedWithin3 = rangedWithin3 + 1 end
		end
		local before = cur.xp[unit:GetID()]
		if before and unit:GetExperience() > before then fought = fought + 1 end
	end
	local meanDist = (#alive > 0) and (distSum / #alive) or -1
	cur.lastMeanDist = meanDist
	-- The march (M2): the first turn any unit is within 3 tiles of the city, and the first turn the army is
	-- "ready": at least 80% of the units it started with within 3 tiles, melee and ranged among them (if it
	-- started with both). A capture before that counts as ready on the capture turn.
	if within3 > 0 and cur.firstArrival < 0 then cur.firstArrival = cur.t end
	if cur.readyTurn < 0 then
		local hasMelee, hasRanged = cur.meleeStart > 0, cur.attStart > cur.meleeStart
		if (within3 >= math.ceil(0.8 * cur.attStart) and (not hasMelee or meleeWithin3 > 0)
		    and (not hasRanged or rangedWithin3 > 0)) or cur.captureTurn == cur.t then
			cur.readyTurn = cur.t
		end
	end
	if meanDist >= 0 and (cur.minMeanDist < 0 or meanDist < cur.minMeanDist) then cur.minMeanDist = meanDist end

	-- A turn counts as engaged if a unit gained experience, the city ended it more damaged, or it fell.
	local engaged = fought > 0 or cur.captureTurn == cur.t or (damageAfter >= 0 and damageAfter > cur.damageBefore)
	if engaged then cur.engagedTurns = cur.engagedTurns + 1 else cur.idleTurns = cur.idleTurns + 1 end

	out(string.format("TURN,%s,%d,%d,%d,%d,%d,%d,%.1f,%d,%d,%d,%d,%d,%d,%d,%d", cur.sc.name, cur.rep, cur.t,
		cur.damageBefore, damageAfter, #alive, within2, meanDist, fought,
		#rosterAlive(lab.defender, cur.defRoster), cur.captured and 1 or 0, late and 1 or 0,
		meleeAlive(lab.attacker, cur.attRoster), within3, meleeWithin3, rangedWithin3))
end

local function finishScenario()
	local city = targetCity()
	local finalDamage = -1
	if not cur.captured and city then finalDamage = city:GetDamage() end
	out(string.format("RESULT,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.1f,%.1f,%d,%d,%d,%d,%d", cur.sc.name, cur.rep,
		cur.captured and 1 or 0, cur.captureTurn, cur.t, cur.cityMaxHP or -1, cur.maxDamage, finalDamage,
		cur.attStart or 0, #rosterAlive(lab.attacker, cur.attRoster or {}), cur.killedByEnemy[lab.attacker],
		cur.defStart or 0, #rosterAlive(lab.defender, cur.defRoster or {}), cur.killedByEnemy[lab.defender],
		cur.engagedTurns, cur.idleTurns, cur.minMeanDist, cur.lastMeanDist, Game.GetGameTurn(),
		cur.meleeStart or 0, meleeAlive(lab.attacker, cur.attRoster or {}), cur.firstArrival, cur.readyTurn))
	cur = nil
	lab.index = lab.index + 1
end

-- Autoplay is left to run out by itself a few turns later (stopping it from inside the attacker's own turn
-- start is not worth the risk); the game then hands the attacker back to the player.
local function finishRun()
	lab.finished = true
	out("DONE")
end

------------------------------------------------------------------------------------------------------------
-- Game hooks
------------------------------------------------------------------------------------------------------------

local function guarded(fn)
	return function(...)
		local ok, err = pcall(fn, ...)
		if not ok then out("ERROR," .. tostring(err)) end
	end
end

-- A battle is staged during the defender's turn, before the attacker's next turn starts. Staged during the
-- attacker's own turn (lab v1-v3), the attacker's AI had already read the map for that turn, so on the battle's
-- first turn it didn't know the new city existed: every unit went to the homeland AI, which started walking them
-- home (seen in the tactical log of diag3/diag4, LOG.md M0 iteration 3).
local function onAttackerTurn()
	if not cur or cur.failed then return end
	sampleTurn(true)   -- only if the defender's turn didn't already score the last turn
	holdStill(lab.attacker, cur.attRoster)
	beginTurn()
end

local function onDefenderTurn()
	if cur and not cur.failed then
		sampleTurn(lab.defender < lab.attacker)   -- right after the attacker's turn
		holdStill(lab.defender, cur.defRoster)
	end
	if cur then
		local attackersLeft = #rosterAlive(lab.attacker, cur.attRoster or {})
		if cur.failed or cur.captured or cur.t >= cur.sc.turns or (cur.t > 0 and attackersLeft == 0) then
			finishScenario()
		end
	end
	if not cur then
		if lab.index > #lab.queue then
			finishRun()
			return
		end
		beginScenario()
	end
end

------------------------------------------------------------------------------------------------------------
-- The scripted attacker (lab mode "script"): a fixed, simple way to play the attack, used as the reference in
-- place of a human player (user ruling, LOG.md). It moves the attacker's staged units at the start of the
-- attacker's unit update, before the AI does, and then ends their moves so the AI can't change them.
-- What it does each turn, in this order (v3 of the rules: losses don't matter, only the capture does, per the
-- user's ruling of 2026-11-18; LOG.md M0 iteration 10):
--   1. (no falling back: every unit fights every turn);
--   2. ranged units shoot the most damaged enemy unit outside the city within range, else the city, moving
--      first to a free tile 2 from the city if they can't shoot from where they stand;
--   3. melee units attack an adjacent enemy unit outside the city if it is no healthier than they are; attack
--      the city when it is down to half its health or less (or, if the army has no ranged units, whenever they
--      are at half health or more); and otherwise move up to a free tile 2 from the city.
------------------------------------------------------------------------------------------------------------

local function healthFraction(unit)
	local maxHP = unit:GetMaxHitPoints()
	return (maxHP - unit:GetDamage()) / maxHP
end

local function scriptNote(text)
	out("SCRIPT," .. cur.sc.name .. "," .. cur.rep .. "," .. cur.t .. "," .. text)
end

-- A free plot between minD and maxD from the city on the attacker's side, nearest to the unit, not already
-- taken this turn.
local function freePlotNear(unit, minD, maxD, taken)
	local best, bestD = nil, nil
	for _, spot in ipairs(openPlots(lab.tx, lab.ty, minD, maxD, "west")) do
		local key = spot.plot:GetX() .. ":" .. spot.plot:GetY()
		if not taken[key] then
			local d = Map.PlotDistance(unit:GetX(), unit:GetY(), spot.plot:GetX(), spot.plot:GetY())
			if not bestD or d < bestD then best, bestD = spot.plot, d end
		end
	end
	if best then taken[best:GetX() .. ":" .. best:GetY()] = true end
	return best
end

local function moveTo(unit, plot)
	if plot and (unit:GetX() ~= plot:GetX() or unit:GetY() ~= plot:GetY()) then
		unit:PushMission(MissionTypes.MISSION_MOVE_TO, plot:GetX(), plot:GetY())
	end
end

local function cityHealthFraction(city)
	local maxHP = city:GetMaxHitPoints()
	return (maxHP - city:GetDamage()) / maxHP
end

-- Enemy field units (not the garrison inside the city) within range of a plot.
local function enemyUnitsNear(x, y, range)
	local found = {}
	for id in pairs(cur.defRoster) do
		local unit = Players[lab.defender]:GetUnitByID(id)
		if isUsable(unit) and not unit:GetPlot():IsCity() and Map.PlotDistance(x, y, unit:GetX(), unit:GetY()) <= range then
			found[#found + 1] = unit
		end
	end
	table.sort(found, function(a, b) return a:GetDamage() > b:GetDamage() end)
	return found
end

local function scriptAttackerTurn()
	local city = targetCity()
	if not city or city:GetOwner() ~= lab.defender then return end
	local units = rosterAlive(lab.attacker, cur.attRoster)
	local taken = {}
	local hasRanged = false
	for _, unit in ipairs(units) do
		if cur.attRoster[unit:GetID()] == "ranged" then hasRanged = true end
	end

	-- 1. Fall back when badly hurt, and stay back until healed.
	cur.resting = cur.resting or {}
	local active = {}
	for _, unit in ipairs(units) do
		local id = unit:GetID()
		local h = healthFraction(unit)
		cur.resting[id] = nil   -- rules v3: nobody falls back
		if cur.resting[id] then
			if Map.PlotDistance(unit:GetX(), unit:GetY(), lab.tx, lab.ty) < 4 then
				moveTo(unit, freePlotNear(unit, 4, 6, taken))
				scriptNote("fall back " .. id)
			end
		else
			active[#active + 1] = unit
		end
	end

	-- 2. Ranged units.
	for _, unit in ipairs(active) do
		if cur.attRoster[unit:GetID()] == "ranged" and isUsable(unit) then
			local function shoot()
				for _, enemy in ipairs(enemyUnitsNear(unit:GetX(), unit:GetY(), unit:Range())) do
					if unit:CanRangeStrikeAt(enemy:GetX(), enemy:GetY()) then
						unit:PushMission(MissionTypes.MISSION_RANGE_ATTACK, enemy:GetX(), enemy:GetY())
						scriptNote("shoot unit " .. unit:GetID())
						return true
					end
				end
				local c = targetCity()
				if c and c:GetOwner() == lab.defender and unit:CanRangeStrikeAt(lab.tx, lab.ty) then
					unit:PushMission(MissionTypes.MISSION_RANGE_ATTACK, lab.tx, lab.ty)
					scriptNote("shoot city " .. unit:GetID())
					return true
				end
				return false
			end
			if not shoot() then
				moveTo(unit, freePlotNear(unit, 2, 2, taken))
				if isUsable(unit) and unit:MovesLeft() > 0 and not shoot() then scriptNote("no shot " .. unit:GetID()) end
			end
		end
	end

	-- 3. Melee units: the strongest go first.
	local melee = {}
	for _, unit in ipairs(active) do
		if cur.attRoster[unit:GetID()] == "melee" and isUsable(unit) then melee[#melee + 1] = unit end
	end
	table.sort(melee, function(a, b) return a:GetDamage() < b:GetDamage() end)
	for _, unit in ipairs(melee) do
		local c = targetCity()
		if not c or c:GetOwner() ~= lab.defender then break end
		local cityLeft = cityHealthFraction(c)
		local victim = nil
		for _, enemy in ipairs(enemyUnitsNear(unit:GetX(), unit:GetY(), 1)) do
			if healthFraction(enemy) <= healthFraction(unit) then victim = enemy break end
		end
		if victim then
			unit:PushMission(MissionTypes.MISSION_MOVE_TO, victim:GetX(), victim:GetY())
			scriptNote("attack unit " .. unit:GetID())
		elseif cityLeft <= 0.5 or (not hasRanged and healthFraction(unit) >= 0.5) then
			unit:PushMission(MissionTypes.MISSION_MOVE_TO, lab.tx, lab.ty)
			scriptNote(string.format("attack city %d (city at %d%%)", unit:GetID(), math.floor(cityLeft * 100)))
		else
			moveTo(unit, freePlotNear(unit, 2, 2, taken) or freePlotNear(unit, 3, 3, taken))
		end
	end

	-- Whatever is left of the turn is spent, so the AI doesn't move these units.
	for _, unit in ipairs(rosterAlive(lab.attacker, cur.attRoster)) do unit:FinishMoves() end
end

local function onPreAIUnitUpdate(playerID)
	if not lab or lab.finished or not lab.script or playerID ~= lab.attacker then return end
	if not cur or cur.failed or cur.t == 0 or cur.scriptedTurn == cur.t then return end
	cur.scriptedTurn = cur.t   -- the update can run several times in one turn
	scriptAttackerTurn()
end

local function onPlayerDoTurn(playerID)
	if not lab or lab.finished then return end
	if playerID == lab.attacker then
		onAttackerTurn()
	elseif playerID == lab.defender then
		onDefenderTurn()
	end
end

local function onCityCaptureComplete(oldOwner, isCapital, x, y, newOwner)
	if not lab or lab.finished or not cur then return end
	if x == lab.tx and y == lab.ty and oldOwner == lab.defender and not cur.captured then
		cur.captured = true
		cur.captureTurn = cur.t
	end
end

local function onUnitPrekill(owner, unitID, unitType, x, y, delay, killer)
	if not lab or lab.finished or not cur or lab.cleaning then return end
	if killer == nil or killer == NO_PLAYER then return end
	local roster = (owner == lab.attacker and cur.attRoster) or (owner == lab.defender and cur.defRoster) or nil
	-- The game can report one death twice (once delayed, once final), so each unit is counted once.
	local key = owner .. ":" .. unitID
	if roster and roster[unitID] and cur.killedByEnemy[owner] and not cur.counted[key] then
		cur.counted[key] = true
		cur.killedByEnemy[owner] = cur.killedByEnemy[owner] + 1
	end
end

------------------------------------------------------------------------------------------------------------
-- Starting the run
------------------------------------------------------------------------------------------------------------

local function startLab()
	local config = SiegeLabConfig
	local attackerID = Game.GetActivePlayer()
	local attacker = Players[attackerID]

	local defenderID = nil
	for id = 0, maxMajors() - 1 do
		local player = Players[id]
		if id ~= attackerID and player and player:IsAlive() and not player:IsMinorCiv() and not player:IsBarbarian() then
			if defenderID == nil or (defenderID < attackerID and id > attackerID) then defenderID = id end
		end
	end
	if defenderID == nil then
		out("ERROR,the game needs at least two civs")
		return
	end
	local defender = Players[defenderID]
	if attacker:GetTeam() == defender:GetTeam() then
		out("ERROR,the first two civs are on the same team")
		return
	end

	local queue, scenarioCount = buildQueue(config)
	if #queue == 0 then
		out("ERROR,no scenarios to run")
		return
	end

	lab = { attacker = attackerID, defender = defenderID, queue = queue, index = 1, changedPlots = {},
	        human = config.human == true, script = config.attacker == "script", cleaning = false, finished = false }

	local w, h = Map.GetGridSize()
	local cx, cy = pickArenaCentre()
	if not cx then
		out("ERROR,the map is too small for the arena (" .. w .. "x" .. h .. ")")
		lab = nil
		return
	end
	lab.cx, lab.cy = cx, cy
	lab.tx, lab.ty = cx + TARGET_DX, cy
	lab.hx, lab.hy = cx + HOME_DX, cy

	-- The new-AI switch (DLL ai/v2 builds only; -1 = this DLL doesn't have it).
	local function setNewAI(player, on)
		if not player.SetNewAI then return -1 end
		player:SetNewAI(on)
		return player:IsNewAI() and 1 or 0
	end
	local newAI = config.newAI or "none"
	lab.attackerNewAI = setNewAI(attacker, newAI == "attacker" or newAI == "both")
	lab.defenderNewAI = setNewAI(defender, newAI == "defender" or newAI == "both")

	out(string.format("RUN,%d,%s,%s,%d,%d,%s,%s,%d,%d,%d,%d,%d", VERSION,
		attacker:GetCivilizationShortDescription(), defender:GetCivilizationShortDescription(), w, h,
		GameInfo.HandicapInfos[Game.GetHandicapType()].Type, GameInfo.GameSpeeds[Game.GetGameSpeedType()].Type,
		lab.human and 1 or (lab.script and 2 or 0), scenarioCount, config.repeats or 1, lab.attackerNewAI, lab.defenderNewAI))

	-- The world: a flat arena, the reserve capital, no victories, no barbarians, enough turns.
	forEachArenaPlot(function(plot) setLand(plot, PLOT_LAND, "TERRAIN_GRASS") end)
	lab.targetPlot = Map.GetPlot(lab.tx, lab.ty)
	local reservePlot = buildReserveSite()
	Map.RecalculateAreas()
	if not reservePlot then
		out("ERROR,no land for the defender's reserve capital")
		lab = nil
		return
	end
	for victory in GameInfo.Victories() do
		Game.SetVictoryValid(victory.ID, false)
	end
	Game.SetOption("GAMEOPTION_NO_BARBARIANS", true)
	local turnsNeeded = 5
	for _, entry in ipairs(queue) do turnsNeeded = turnsNeeded + entry.sc.turns + 1 end
	if Game.GetMaxTurns() > 0 then Game.ChangeMaxTurns(turnsNeeded + 20) end

	-- The two sides get their permanent cities first, so removing their starting units doesn't eliminate them.
	local home = attacker:InitCity(lab.hx, lab.hy)
	local reserve = defender:InitCity(reservePlot:GetX(), reservePlot:GetY())
	if not home or not reserve then
		out("ERROR,the permanent cities could not be founded")
		lab = nil
		return
	end
	lab.incomePlot = { [attackerID] = Map.GetPlot(lab.hx, lab.hy), [defenderID] = reservePlot }
	lab.reservePlot = reservePlot
	lab.extraGold = {}
	lab.cleaning = true
	for id = 0, maxPlayers() - 1 do
		local player = Players[id]
		if player and player:IsEverAlive() then
			killUnitsOf(player, nil)
			if id ~= attackerID and id ~= defenderID then player:KillCities() end
		end
	end
	lab.cleaning = false

	-- Both sides: no unhappiness penalties over a long run, and the strategic resources their units need.
	for _, player in ipairs({ attacker, defender }) do
		player:ChangeExtraHappinessPerCity(20)
		for _, name in ipairs({ "RESOURCE_IRON", "RESOURCE_HORSE" }) do
			local id = typeID(name)
			-- Booked as coming from an event: the game checks its "from tiles" count against the map every turn.
			if id then player:ChangeNumResourceTotal(id, 30, false, true) end
		end
	end
	if not ensureWar() then
		out("ERROR,the two civs could not be put at war")
		lab = nil
		return
	end

	GameEvents.PlayerDoTurn.Add(guarded(onPlayerDoTurn))
	GameEvents.CityCaptureComplete.Add(guarded(onCityCaptureComplete))
	GameEvents.UnitPrekill.Add(guarded(onUnitPrekill))
	if lab.script then GameEvents.PlayerPreAIUnitUpdate.Add(guarded(onPreAIUnitUpdate)) end

	-- The first scenario is staged in the defender's first turn, like every later one.
	if lab.human then
		out("WARN,human run: end the turn to stage the first battle; you play the attacker")
	else
		Game.SetAIAutoPlay(turnsNeeded, attackerID)
		if Game.GetAIAutoPlay() == 0 then
			out("ERROR,autoplay did not start (no free player slot for the observer); start the game with fewer than 22 civs")
		end
	end
end

local function onLoadScreenClose()
	if lab then return end
	local config = SiegeLabConfig
	if type(config) ~= "table" or not config.enabled then return end
	if Game.IsNetworkMultiPlayer() or Game.IsHotSeat() then
		out("WARN,not started: the siege lab only runs in single player. Remove it before multiplayer (Install-SiegeLab.ps1 -Remove)")
		return
	end
	if Game.GetElapsedGameTurns() ~= 0 or Players[Game.GetActivePlayer()]:GetNumCities() > 0 then
		out("WARN,not started: the siege lab only runs from the first turn of a new game")
		return
	end
	startLab()
end

Events.LoadScreenClose.Add(guarded(onLoadScreenClose))
