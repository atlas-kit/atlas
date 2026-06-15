Tools = Tools or {}

function onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
    if not table.contains({2416, 10515}, item.itemid) then
        return false
    end

    return destroyItem(player, target, toPosition)
end

Tools.onUseCrowbar = onUseCrowbar
