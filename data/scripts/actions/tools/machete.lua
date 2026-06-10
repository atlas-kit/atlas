local action = Action()

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
    return onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
end

action:id(3308, 3330)
action:register()
