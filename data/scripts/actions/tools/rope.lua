local action = Action()

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
    return onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
end

action:id(3003, 646)
action:register()
