local action = Action()

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	item:transform(item:getId() - 1)
	fromPosition:sendMagicEffect(CONST_ME_POFF)
	return true
end

action:id(3482)
action:register()
