--[[
Reserved player storage ranges:
- 300000 to 301000+ reserved for achievements
- 20000 to 21000+ reserved for achievement progress
- 10000000 to 20000000 reserved for outfits and mounts on source
]]--

AccountStorageKeys = {
}

GlobalStorageKeys = {
	cobraBastionFlask = 27344,
}

PlayerStorageKeys = {
	-- connection stability subsystem
	lastPing = 7464,
	lastPong = 7465,

	-- Misc:
	annihilatorReward = 27359,
	goldenOutfit = 27360,
	-- empty: 30017
	promotion = 30018,
	delayLargeSeaShell = 30019,
	firstRod = 30020,
	delayWallMirror = 30021,
	-- empty: 30022
	madSheepSummon = 27367,
	crateUsable = 27368,
	-- empty: 30025
	afflictedOutfit = 27370,
	afflictedPlagueMask = 27371,
	afflictedPlagueBell = 27372,
	-- empty: 30029
	-- empty: 30030
	nailCaseUseCount = 30031,
	swampDigging = 30032,
	insectoidCell = 30033,
	-- empty: 30034
	mutatedPumpkin = 30035,

	-- Achievements:
	achievementsTotal = 17719,
	achievementsCounter = 17720,
	achievementsBase = 300000,

	-- Bestiary:
	bestiaryKillsBase = 400000,
	bestiaryTrackerBase = 500000,
}
