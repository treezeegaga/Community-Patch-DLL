-- Runs SiegeLab.lua against a small fake of the game, outside Civ V:
--
--   cd ai-lab/siege && lua5.1 test/mock_run.lua
--
-- It checks the lab's own logic (staging, scoring, the log lines, the hand-over between scenarios). It says
-- nothing about how the real game or the AI behaves: the fake attacker just damages the city at a fixed rate.
-- The fake only has the engine functions the lab is known to use, so a misspelled call fails here.

local W, H = 40, 24
local turn = 0
local log = {}

local function strict(tbl, what)
	return setmetatable(tbl, { __index = function(_, key) error("fake " .. what .. " has no '" .. tostring(key) .. "'", 2) end })
end

-- Hex distance on the game's grid (rows offset by half a tile on odd rows).
local function cube(x, y)
	local q = x - (y - y % 2) / 2
	return q, y, -q - y
end
local function distance(x1, y1, x2, y2)
	local a1, b1, c1 = cube(x1, y1)
	local a2, b2, c2 = cube(x2, y2)
	return math.max(math.abs(a1 - a2), math.abs(b1 - b2), math.abs(c1 - c2))
end

------------------------------------------------------------------------------------------------------------
local plots, players, teams = {}, {}, {}
local nextUnitID, nextCityID = 1, 1

local function newPlot(x, y)
	local p = { x = x, y = y, water = (x < 6), mountain = false, city = nil, units = {}, river = ((x + y) % 4 == 0) }
	local m = {}
	function m:GetX() return p.x end
	function m:GetY() return p.y end
	function m:IsWater() return p.water end
	function m:IsMountain() return p.mountain end
	function m:IsCity() return p.city ~= nil end
	function m:GetPlotCity() return p.city end
	function m:GetNumUnits() local n = 0 for _ in pairs(p.units) do n = n + 1 end return n end
	function m:SetFeatureType(f) assert(type(f) == "number") end
	function m:SetResourceType(r, n, b) assert(type(r) == "number" and type(n) == "number" and type(b) == "boolean") end
	function m:SetImprovementType(i, pl, b) assert(type(i) == "number" and type(pl) == "number" and type(b) == "boolean") end
	function m:SetRouteType(r, pl) assert(type(r) == "number" and type(pl) == "number") end
	-- Every fourth plot has a river on one side. The engine asserts on any "no direction" value but 7.
	function m:IsWOfRiver() return p.river end
	function m:SetWOfRiver(b, d) assert(b == false and d == 7, "river removal must pass NO_FLOWDIRECTION (7)") p.river = false end
	m.IsNWOfRiver, m.IsNEOfRiver = m.IsWOfRiver, m.IsWOfRiver
	m.SetNWOfRiver, m.SetNEOfRiver = m.SetWOfRiver, m.SetWOfRiver
	function m:SetPlotType(t, a, b, c)
		assert(type(t) == "number" and type(a) == "boolean" and type(b) == "boolean" and type(c) == "boolean")
		p.water, p.mountain = false, (t == 0)
	end
	function m:SetTerrainType(t, a, b) assert(type(t) == "number" and type(a) == "boolean" and type(b) == "boolean") end
	m._p = p
	return strict(m, "plot")
end
for y = 0, H - 1 do for x = 0, W - 1 do plots[y * W + x] = newPlot(x, y) end end

Map = strict({
	GetGridSize = function() return W, H end,
	GetPlot = function(x, y) if x < 0 or y < 0 or x >= W or y >= H then return nil end return plots[y * W + x] end,
	PlotDistance = distance,
	RecalculateAreas = function() end,
}, "Map")

local function newUnit(owner, unitType, x, y)
	local u = { id = nextUnitID, owner = owner, type = unitType, x = x, y = y, xp = 0, dead = false }
	nextUnitID = nextUnitID + 1
	local m = {}
	function m:GetID() return u.id end
	function m:GetX() return u.x end
	function m:GetY() return u.y end
	function m:IsDead() return u.dead end
	function m:IsDelayedDeath() return false end
	function m:GetExperience() return u.xp end
	function m:SetExperience(v) assert(type(v) == "number") u.xp = v end
	function m:SetDeployFromOperationTurn(t) assert(type(t) == "number") end
	function m:Kill(delay, by)
		assert(type(delay) == "boolean" and type(by) == "number")
		if u.dead then return end
		u.dead = true
		players[u.owner]._units[u.id] = nil
		Map.GetPlot(u.x, u.y)._p.units[u.id] = nil
		GameEvents.UnitPrekill.fire(u.owner, u.id, u.type, u.x, u.y, false, by)
	end
	m._u = u
	return strict(m, "unit")
end

local function newCity(owner, x, y)
	local c = { id = nextCityID, owner = owner, x = x, y = y, damage = 0, pop = 1 }
	nextCityID = nextCityID + 1
	local m = {}
	function m:GetOwner() return c.owner end
	function m:GetDamage() return c.damage end
	function m:SetDamage(v, quiet) assert(type(v) == "number" and type(quiet) == "boolean") c.damage = v end
	function m:GetMaxHitPoints() return 200 end
	function m:GetStrengthValue() return 1500 end
	function m:UpdateStrengthValue() end
	function m:SetPopulation(v, reassign) assert(type(v) == "number" and reassign == true) c.pop = v end
	function m:SetNumRealBuilding(b, n) assert(type(b) == "number" and type(n) == "number") end
	function m:SetProduction(v) assert(v == 0) end
	function m:SetOverflowProduction(v) assert(v == 0) end
	m._c = c
	return strict(m, "city")
end

local function newPlayer(id, isMinor)
	local m = { _units = {}, _cities = {}, _everAlive = true }
	function m:IsAlive() for _ in pairs(m._units) do return true end for _ in pairs(m._cities) do return true end return false end
	function m:IsEverAlive() return m._everAlive end
	function m:IsMinorCiv() return isMinor end
	function m:IsBarbarian() return false end
	function m:GetTeam() return id end
	function m:GetNumCities() local n = 0 for _ in pairs(m._cities) do n = n + 1 end return n end
	function m:GetCivilizationShortDescription() return "Civ" .. id end
	function m:CalculateGoldRate() return -3 end
	function m:SetGold(v) assert(type(v) == "number") end
	function m:SetJONSCulture(v) assert(v == 0) end
	function m:SetFaith(v) assert(v == 0) end
	function m:ChangeExtraHappinessPerCity(v) assert(type(v) == "number") end
	function m:ChangeNumResourceTotal(r, v) assert(type(r) == "number" and type(v) == "number") end
	function m:InitCity(x, y, ...)
		assert(select("#", ...) == 0, "InitCity's optional arguments are read as numbers; pass none")
		local plot = Map.GetPlot(x, y)
		assert(plot and not plot:IsCity() and not plot:IsWater() and not plot:IsMountain(), "bad city plot")
		local city = newCity(id, x, y)
		m._cities[city._c.id] = city
		plot._p.city = city
		return city
	end
	function m:InitUnit(unitType, x, y)
		assert(type(unitType) == "number")
		local plot = Map.GetPlot(x, y)
		assert(plot and not plot:IsWater() and not plot:IsMountain(), "bad unit plot")
		local unit = newUnit(id, unitType, x, y)
		m._units[unit._u.id] = unit
		plot._p.units[unit._u.id] = unit
		return unit
	end
	function m:GetUnitByID(unitID) return m._units[unitID] end
	function m:Units()
		local list = {}
		for _, unit in pairs(m._units) do list[#list + 1] = unit end
		local i = 0
		return function() i = i + 1 return list[i] end
	end
	function m:Cities()
		local list = {}
		for _, city in pairs(m._cities) do list[#list + 1] = city end
		local i = 0
		return function() i = i + 1 return list[i] end
	end
	function m:KillCities() for cityID, city in pairs(m._cities) do Map.GetPlot(city._c.x, city._c.y)._p.city = nil m._cities[cityID] = nil end end
	function m:Disband(city)
		assert(city._c.owner == id, "Disband called on another player's city")
		Map.GetPlot(city._c.x, city._c.y)._p.city = nil
		m._cities[city._c.id] = nil
	end
	return strict(m, "player")
end

local function newTeam(id)
	local m = { _war = {}, _techs = { [0] = true } }
	function m:Meet(other, quiet) assert(type(other) == "number" and type(quiet) == "boolean") end
	function m:IsAtWar(other) return m._war[other] == true end
	function m:SetPermanentWarPeace(other, v) assert(type(other) == "number" and type(v) == "boolean") end
	function m:DeclareWar(other, pact, by)
		assert(type(other) == "number" and pact == false and type(by) == "number")
		m._war[other] = true
		teams[other]._war[id] = true
	end
	function m:IsHasTech(t) return m._techs[t] == true end
	function m:SetHasTech(t, v, pl, first, announce, noBonus)
		assert(type(t) == "number" and type(v) == "boolean" and type(pl) == "number" and first == false and announce == false and noBonus == true)
		m._techs[t] = v or nil
	end
	return strict(m, "team")
end

Players, Teams = players, teams
for id = 0, 4 do players[id] = newPlayer(id, id >= 3) teams[id] = newTeam(id) end
for id = 0, 4 do players[id]:InitUnit(1, 10 + id, 2) end   -- everyone's starting unit

------------------------------------------------------------------------------------------------------------
local function event()
	local handlers = {}
	return { Add = function(fn) handlers[#handlers + 1] = fn end,
	         fire = function(...) for _, fn in ipairs(handlers) do fn(...) end end }
end
GameEvents = strict({ PlayerDoTurn = event(), CityCaptureComplete = event(), UnitPrekill = event() }, "GameEvents")
Events = strict({ LoadScreenClose = event() }, "Events")
GameDefines = { MAX_MAJOR_CIVS = 3, MAX_PLAYERS = 5 }

local autoplay, maxTurns = 0, 330
Game = strict({
	GetActivePlayer = function() return 0 end,
	GetGameTurn = function() return turn end,
	GetElapsedGameTurns = function() return turn end,
	IsNetworkMultiPlayer = function() return false end,
	IsHotSeat = function() return false end,
	SetVictoryValid = function(v, valid) assert(type(v) == "number" and valid == false) end,
	SetOption = function(name, v) assert(name == "GAMEOPTION_NO_BARBARIANS" and v == true) end,
	GetMaxTurns = function() return maxTurns end,
	ChangeMaxTurns = function(n) maxTurns = maxTurns + n end,
	SetAIAutoPlay = function(n, pl) assert(type(n) == "number" and type(pl) == "number") autoplay = n end,
	GetAIAutoPlay = function() return autoplay end,
	GetHandicapType = function() return 0 end,
	GetGameSpeedType = function() return 0 end,
}, "Game")

local typeNames = { "TECH_AGRICULTURE", "TECH_ARCHERY", "TECH_MASONRY", "TECH_BRONZE_WORKING", "TECH_THE_WHEEL",
	"TECH_CONSTRUCTION", "TECH_IRON_WORKING", "TECH_MATHEMATICS",
	"UNIT_WARRIOR", "UNIT_ARCHER", "UNIT_SPEARMAN", "UNIT_COMPOSITE_BOWMAN", "UNIT_SWORDSMAN", "UNIT_CATAPULT",
	"BUILDING_WALLS", "TERRAIN_GRASS", "TERRAIN_MOUNTAIN", "FEATURE_FOREST", "RESOURCE_IRON", "RESOURCE_HORSE" }
GameInfoTypes = {}
for i, name in ipairs(typeNames) do GameInfoTypes[name] = i - 1 end
local function rows(list)
	return function() local i = 0 return function() i = i + 1 return list[i] end end
end
local techRows, unitRows = {}, {}
for name, id in pairs(GameInfoTypes) do
	if name:find("^TECH_") then techRows[#techRows + 1] = { ID = id, Type = name } end
	if name:find("^UNIT_") then
		unitRows[id] = { ID = id, Type = name, Range = (name:find("ARCHER") or name:find("BOWMAN") or name:find("CATAPULT")) and 2 or 0 }
	end
end
GameInfo = {
	Technologies = rows(techRows),
	Technology_PrereqTechs = rows({ { TechType = "TECH_CONSTRUCTION", PrereqTech = "TECH_MASONRY" },
		{ TechType = "TECH_CONSTRUCTION", PrereqTech = "TECH_THE_WHEEL" },
		{ TechType = "TECH_IRON_WORKING", PrereqTech = "TECH_BRONZE_WORKING" } }),
	Victories = rows({ { ID = 0 }, { ID = 1 } }),
	Units = unitRows,
	HandicapInfos = { [0] = { Type = "HANDICAP_KING" } },
	GameSpeeds = { [0] = { Type = "GAMESPEED_QUICK" } },
}

function include(name)
	local f = io.open(name .. ".lua")
	if f then f:close() dofile(name .. ".lua") end
end
local realPrint = print
function print(line)
	log[#log + 1] = line
	realPrint(line)
end

------------------------------------------------------------------------------------------------------------
SiegeLabConfig = { enabled = true, set = arg[1] or "siege", repeats = 2, only = {}, human = false }
dofile("SiegeLab.lua")
Events.LoadScreenClose.fire()

-- The fake game: each turn every attacking unit damages the city from where it stands and gains experience;
-- every third scenario's city can fall; the defender then heals a little and kills one attacker every
-- fourth turn. Units never move.
local attacker, defender = players[0], players[1]
local function targetOf()   -- the defender's newest city; its first one is the reserve capital
	local best = nil
	for _, city in pairs(defender._cities) do
		if best == nil or city._c.id > best._c.id then best = city end
	end
	return best
end
local scenarioNumber = 0
for step = 1, 2000 do
	turn = turn + 1
	if autoplay > 0 then autoplay = autoplay - 1 end
	local linesBefore = #log
	GameEvents.PlayerDoTurn.fire(0)
	for i = linesBefore + 1, #log do
		if log[i]:find("%] START,") then scenarioNumber = scenarioNumber + 1 end
	end
	local city = targetOf()
	if city and defender:GetNumCities() >= 2 then
		local hitters = 0
		for _, unit in pairs(attacker._units) do
			hitters = hitters + 1
			unit._u.xp = unit._u.xp + 2
		end
		city._c.damage = city._c.damage + hitters * 12
		if scenarioNumber % 3 == 0 and city._c.damage >= 200 then
			defender._cities[city._c.id] = nil
			city._c.owner = 0
			attacker._cities[city._c.id] = city
			GameEvents.CityCaptureComplete.fire(1, false, city._c.x, city._c.y, 0, city._c.pop, true, 0, 0)
		elseif city._c.damage >= 200 then
			city._c.damage = 199
		end
	end
	GameEvents.PlayerDoTurn.fire(1)
	city = targetOf()
	if city and city._c.owner == 1 then city._c.damage = math.max(0, city._c.damage - 20) end
	if turn % 4 == 0 then
		for _, unit in pairs(attacker._units) do unit:Kill(false, 1) break end
	end
	if log[#log] == "[SiegeLab] DONE" then break end
end

------------------------------------------------------------------------------------------------------------
-- Checks
local counts = { RUN = 0, START = 0, TURN = 0, RESULT = 0, WARN = 0, ERROR = 0, DONE = 0 }
local captured, fields = 0, { RUN = 11, START = 9, TURN = 13, RESULT = 20 }
for _, line in ipairs(log) do
	local body = line:match("^%[SiegeLab%] (.*)$")
	assert(body, "a line without the tag: " .. line)
	local kind = body:match("^(%u+)")
	counts[kind] = (counts[kind] or 0) + 1
	if fields[kind] then
		local n = select(2, body:gsub(",", ",")) + 1
		assert(n == fields[kind], kind .. " has " .. n .. " fields, expected " .. fields[kind] .. ": " .. body)
	end
	if kind == "RESULT" and body:match("^RESULT,[^,]+,%d+,1,") then captured = captured + 1 end
end
local expected = #SiegeLabScenarios[SiegeLabConfig.set] * SiegeLabConfig.repeats
realPrint(string.format("-- %d scenarios staged, %d results, %d captured, %d turns scored, %d warnings, %d errors",
	counts.START, counts.RESULT, captured, counts.TURN, counts.WARN, counts.ERROR))
assert(counts.ERROR == 0, "the lab logged errors")
assert(counts.RUN == 1 and counts.DONE == 1, "expected one RUN and one DONE line")
assert(counts.START == expected and counts.RESULT == expected, "expected " .. expected .. " scenarios")
assert(captured > 0 and captured < expected, "the fake game should produce both captures and failures")
assert(players[2]:IsAlive() == false and players[3]:IsAlive() == false, "bystanders should be gone")
realPrint("-- mock run passed")
