local event = MoveEvent()

function event.onStepIn(creature, item, position, fromPosition)
	if not creature:isPlayer() or creature:isInGhostMode() then
		return true
	end

	item:transform(item.itemid + 1)
	item:decay()
	return true
end

event:id(293, 461, 3310)
event:register()
