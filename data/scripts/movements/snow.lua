local event = MoveEvent()

function event.onStepOut(creature, item, position, fromPosition)
	if not creature:isPlayer() or creature:isInGhostMode() then
		return true
	end

	if item:getId() == 670 then
		item:transform(6594)
	else
		item:transform(item.itemid + 15)
	end
	creature:addAchievementProgress("Snowbunny", 10000)
	item:decay()
	return true
end

event:id(670, 6580, 6581, 6582, 6583, 6584, 6585, 6586, 6587, 6588, 6589, 6590, 6591, 6592, 6593)
event:register()
