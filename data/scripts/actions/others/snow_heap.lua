local action = Action()

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	Game.createItem(2992, 1, item:getPosition())
	return true
end

action:id(611)
action:register()
