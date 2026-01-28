local condition = Condition(CONDITION_OUTFIT)
condition:setOutfit({lookType = 267})
condition:setTicks(-1)

local conditions = {
	CONDITION_POISON, CONDITION_FIRE, CONDITION_ENERGY,
	CONDITION_PARALYZE, CONDITION_DRUNK, CONDITION_DROWN,
	CONDITION_FREEZING, CONDITION_DAZZLED, CONDITION_CURSED,
	CONDITION_BLEEDING
}

local ids = {4620, 4621, 4622, 4623, 4624, 4625, 4820, 4821, 4822, 4823, 4824, 4825}

do
	local event = MoveEvent()

	function event.onStepIn(creature, item, position, fromPosition)
		if not creature:isPlayer() then
			return false
		end
		for i = 1, #conditions do
			creature:removeCondition(conditions[i])
		end
		creature:addAchievementProgress("Waverider", 100000)
		creature:addCondition(condition)
		return true
	end

	event:id(unpack(ids))
	event:register()
end

do
	local event = MoveEvent()

	function event.onStepOut(creature, item, position, fromPosition)
		if not creature:isPlayer() then
			return false
		end
		creature:removeCondition(CONDITION_OUTFIT)
		return true
	end

	event:id(unpack(ids))
	event:register()
end
