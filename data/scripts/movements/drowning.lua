local condition = Condition(CONDITION_DROWN)
condition:setParameter(CONDITION_PARAM_PERIODICDAMAGE, -20)
condition:setParameter(CONDITION_PARAM_TICKS, -1)
condition:setParameter(CONDITION_PARAM_TICKINTERVAL, 2000)

local ids = {
	-- ocean floor
	5405, 5406, 5407, 5408, 5409, 5410,
	5427, 5428, 5429, 5430, 5431, 5432, 5433, 5434, 5435, 5436, 5437, 5438,
	10019,
	23944, 23945, 23946, 23947, 23948, 23949,
	23955, 23956, 23957, 23958, 23959, 23960, 23961, 23962, 23963, 23964, 23965, 23966,
	24624,

	5743, 5744, -- sunken ship
	5763, 5764, -- opening
	9671, -- sandstone floor
	9672, -- white marble floor
	9673, -- stone floor
	23950, 23951, 23952, 23953, 23954, -- rock
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
