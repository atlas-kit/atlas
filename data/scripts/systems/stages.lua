-- Experience stages
-- minlevel and multiplier are MANDATORY
-- maxlevel is OPTIONAL, but is considered infinite by default
-- For a flat experience rate, register no stages (empty table) and the
-- rateExp value from config.lua will be used instead.
Game.setExperienceStages({
	{ minlevel = 1, maxlevel = 8, multiplier = 7 },
	{ minlevel = 9, maxlevel = 20, multiplier = 6 },
	{ minlevel = 21, maxlevel = 50, multiplier = 5 },
	{ minlevel = 51, maxlevel = 100, multiplier = 4 },
	{ minlevel = 101, multiplier = 3 }
})
