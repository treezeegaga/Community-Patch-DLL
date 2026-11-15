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

local VERSION = 1
local TAG = "[SiegeLab] "

local PLOT_MOUNTAIN, PLOT_HILLS, PLOT_LAND = 0, 1, 2   -- PlotTypes in CvEnums.h
local NO_PLAYER = -1

-- The arena: a rectangle of flat grassland. The target city sits TARGET_DX east of its centre and the
-- attacker's home city HOME_DX west of it, 12 tiles apart on the same row.
local HALF_W, HALF_H = 12, 6
local TARGET_DX, HOME_DX = 3, -9
local TREASURY = 60   -- both sides are reset to this each turn: enough to cover upkeep, too little to buy or upgrade

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
	plot:SetWOfRiver(false, -1)
	plot:SetNWOfRiver(false, -1)
	plot:SetNEOfRiver(false, -1)
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
		local forest = typeID("FEATURE_FOREST")
		local i = 0
		forEachArenaPlot(function(plot)
			local d = Map.PlotDistance(plot:GetX(), plot:GetY(), tx, ty)
			if d >= 2 and d <= 3 and plot:GetX() <= tx then
				i = i + 1
				if i % 2 == 0 then
					setLand(plot, PLOT_HILLS, "TERRAIN_GRASS")
				elseif forest then
					plot:SetFeatureType(forest)
				end
				lab.changedPlots[#lab.changedPlots + 1] = plot
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

local function setTechs(playerID, names)
	local team = Teams[Players[playerID]:GetTeam()]
	local wanted = techSet(names)
	for tech in GameInfo.Technologies() do
		local want = wanted[tech.ID] == true
		if team:IsHasTech(tech.ID) ~= want then
			team:SetHasTech(tech.ID, want, playerID, false, false, true)
		end
	end
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
-- plots and ranged behind. Returns the roster: unit ID -> true.
local function placeUnits(playerID, entries, plots, justDeployed)
	local player = Players[playerID]
	local melee, ranged = {}, {}
	for _, entry in ipairs(entries or {}) do
		local id = typeID(entry[1])
		local info = GameInfo.Units[id]
		for _ = 1, entry[2] do
			local slot = { id = id, xp = entry[3] }
			if info and (info.Range or 0) > 0 then ranged[#ranged + 1] = slot else melee[#melee + 1] = slot end
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
			roster[unit:GetID()] = true
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

------------------------------------------------------------------------------------------------------------
-- Keeping the two sides still between scenarios and turns
------------------------------------------------------------------------------------------------------------

-- Run at the start of a side's turn: no purchases or upgrades, no policies or pantheons, nothing finishes
-- in a city, and anything that isn't part of the staged battle is removed.
local function holdStill(playerID, roster)
	local player = Players[playerID]
	player:SetGold(TREASURY)
	player:SetJONSCulture(0)
	player:SetFaith(0)
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
	        killedByEnemy = { [lab.attacker] = 0, [lab.defender] = 0 }, counted = {}, sampled = true }

	-- Wipe what the last scenario left.
	lab.cleaning = true
	killUnitsOf(attacker, nil)
	killUnitsOf(defender, nil)
	local old = targetCity()
	if old then Players[old:GetOwner()]:Disband(old) end
	lab.cleaning = false

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
	city:SetDamage(0, true)

	-- The defenders: one unit inside the city, the rest next to it on the far side.
	cur.defRoster = {}
	if sc.city.garrison then
		local unit = defender:InitUnit(typeID(sc.city.garrison), lab.tx, lab.ty)
		if unit then cur.defRoster[unit:GetID()] = true end
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

	cur.attStart, cur.defStart = 0, 0
	for _ in pairs(cur.attRoster) do cur.attStart = cur.attStart + 1 end
	for _ in pairs(cur.defRoster) do cur.defStart = cur.defStart + 1 end

	if not ensureWar() then
		out("ERROR," .. sc.name .. ": the two civs could not be put at war")
		cur.failed = true
		return
	end

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
	for _, unit in ipairs(alive) do
		local d = Map.PlotDistance(unit:GetX(), unit:GetY(), lab.tx, lab.ty)
		distSum = distSum + d
		if d <= 2 then within2 = within2 + 1 end
		local before = cur.xp[unit:GetID()]
		if before and unit:GetExperience() > before then fought = fought + 1 end
	end
	local meanDist = (#alive > 0) and (distSum / #alive) or -1
	cur.lastMeanDist = meanDist
	if meanDist >= 0 and (cur.minMeanDist < 0 or meanDist < cur.minMeanDist) then cur.minMeanDist = meanDist end

	-- A turn counts as engaged if a unit gained experience, the city ended it more damaged, or it fell.
	local engaged = fought > 0 or cur.captureTurn == cur.t or (damageAfter >= 0 and damageAfter > cur.damageBefore)
	if engaged then cur.engagedTurns = cur.engagedTurns + 1 else cur.idleTurns = cur.idleTurns + 1 end

	out(string.format("TURN,%s,%d,%d,%d,%d,%d,%d,%.1f,%d,%d,%d,%d", cur.sc.name, cur.rep, cur.t,
		cur.damageBefore, damageAfter, #alive, within2, meanDist, fought,
		#rosterAlive(lab.defender, cur.defRoster), cur.captured and 1 or 0, late and 1 or 0))
end

local function finishScenario()
	local city = targetCity()
	local finalDamage = -1
	if not cur.captured and city then finalDamage = city:GetDamage() end
	out(string.format("RESULT,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.1f,%.1f,%d", cur.sc.name, cur.rep,
		cur.captured and 1 or 0, cur.captureTurn, cur.t, cur.cityMaxHP or -1, cur.maxDamage, finalDamage,
		cur.attStart or 0, #rosterAlive(lab.attacker, cur.attRoster or {}), cur.killedByEnemy[lab.attacker],
		cur.defStart or 0, #rosterAlive(lab.defender, cur.defRoster or {}), cur.killedByEnemy[lab.defender],
		cur.engagedTurns, cur.idleTurns, cur.minMeanDist, cur.lastMeanDist, Game.GetGameTurn()))
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

local function onAttackerTurn()
	if cur then
		sampleTurn(true)   -- only if the defender's turn didn't already score it
		local attackersLeft = #rosterAlive(lab.attacker, cur.attRoster or {})
		if cur.failed or cur.captured or cur.t >= cur.sc.turns or attackersLeft == 0 then
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
	if cur.failed then return end   -- scored and skipped at the next attacker turn
	holdStill(lab.attacker, cur.attRoster)
	beginTurn()
end

local function onDefenderTurn()
	if not cur or cur.failed then return end
	if lab.defender > lab.attacker then sampleTurn(false) end
	holdStill(lab.defender, cur.defRoster)
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
	        human = config.human == true, cleaning = false, finished = false }

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

	out(string.format("RUN,%d,%s,%s,%d,%d,%s,%s,%d,%d,%d", VERSION,
		attacker:GetCivilizationShortDescription(), defender:GetCivilizationShortDescription(), w, h,
		GameInfo.HandicapInfos[Game.GetHandicapType()].Type, GameInfo.GameSpeeds[Game.GetGameSpeedType()].Type,
		lab.human and 1 or 0, scenarioCount, config.repeats or 1))

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
			if id then player:ChangeNumResourceTotal(id, 30) end
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

	-- The first scenario is staged at the start of the attacker's next turn, like every later one.
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
