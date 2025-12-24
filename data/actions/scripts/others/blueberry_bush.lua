function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	item:transform(3700)
	item:decay()
	Game.createItem(2489, 3, fromPosition)
	player:addAchievementProgress("Bluebarian", 500)
	return true
end
