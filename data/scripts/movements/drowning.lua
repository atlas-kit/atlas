local condition = Condition(CONDITION_DROWN)
condition:setParameter(CONDITION_PARAM_PERIODICDAMAGE, -20)
condition:setParameter(CONDITION_PARAM_TICKS, -1)
condition:setParameter(CONDITION_PARAM_TICKINTERVAL, 2000)

local ids = {
	-- ocean floor
	5405, 5406, 5407, 5408, 5409,
	5427, 5428, 5429, 5430, 5431, 5432, 5433, 5434, 5435, 5436, 5437,
	10019,
	23944, 21576, 21577, 21578, 21579, 21580,
	23955, 21587, 21588, 21589, 21590, 21591, 21592, 21593, 21594, 21595, 21596, 21597,
	24624,

	5743, 5744, -- sunken ship
	5763, 5764, -- opening
	9671, -- sandstone floor
	9672, -- white marble floor
	9673, -- stone floor
	23950, 21582, 21583, 21584, 21585, -- rock
}

do
	local event = MoveEvent()

	function event.onStepIn(creature, item, position, fromPosition)
		if creature:isPlayer() then
			creature:addCondition(condition)
			creature:addAchievementProgress("Deep Sea Diver", 1000000)
		end
		return true
	end

	event:id(unpack(ids))
	event:register()
end

do
	local event = MoveEvent()

	function event.onStepOut(creature, item, position, fromPosition)
		if creature:isPlayer() then
			creature:removeCondition(CONDITION_DROWN)
		end
		return true
	end

	event:id(unpack(ids))
	event:register()
end
