local action = Action()

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
    return onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
end

action:id(3453)
action:register()
