Tools = Tools or {}

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
    if not table.contains({2550, 10513}, item.itemid) then
        return false
    end

    if target.itemid == 2739 then
        target:transform(2737)
        target:decay()
        Game.createItem(2694, 1, toPosition)
        player:addAchievementProgress("Happy Farmer", 200)
        return true
    end

    if target.itemid == 5465 then
        target:transform(5464)
        target:decay()
        Game.createItem(5467, 1, toPosition)
        player:addAchievementProgress("Natural Sweetener", 50)
        return true
    end

    return destroyItem(player, target, toPosition)
end

Tools.onUseScythe = onUseScythe
