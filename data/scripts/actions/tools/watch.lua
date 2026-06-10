local watch = Action()

function watch.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	player:sendTextMessage(MESSAGE_INFO_DESCR, "The time is " .. Game.getFormattedWorldTime() .. ".")
	return true
end

watch:id(2445, 2446, 2447, 2448, 2906, 2771, 6091, 751, 8527, 8528)
watch:register()
