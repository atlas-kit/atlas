Tools = Tools or {}

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
    local targetId = target.itemid
    if not targetId then
        return true
    end

    if table.contains(Tools.wildGrowth, targetId) then
        toPosition:sendMagicEffect(CONST_ME_POFF)
        target:remove()
        return true
    end

    local grass = Tools.jungleGrass[targetId]
    if grass then
        target:transform(grass)
        target:decay()
        player:addAchievementProgress("Nothing Can Stop Me", 100)
        return true
    end

    return destroyItem(player, target, toPosition)
end

Tools.onUseMachete = onUseMachete
