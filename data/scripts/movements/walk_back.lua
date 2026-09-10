local event = MoveEvent()

function event.onStepIn(creature, item, position, fromPosition)
	if item.uid > 0 and item.uid <= 65535 then
		creature:teleportTo(fromPosition, false)
	end
	return true
end

event:id(2469, 2472, 2478, 2480, 2481, 2482)
event:register()
