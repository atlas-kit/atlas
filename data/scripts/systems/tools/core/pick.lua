Tools = Tools or {}

function onUsePick(player, item, fromPosition, target, toPosition, isHotkey)
    if target.itemid == 11227 then
        local chance = math.random(1, 100)
        if chance == 1 then
            player:addItem(ITEM_CRYSTAL_COIN)
        elseif chance <= 6 then
            player:addItem(ITEM_GOLD_COIN)
        elseif chance <= 51 then
            player:addItem(ITEM_PLATINUM_COIN)
        else
            player:addItem(2145)
        end

        player:addAchievementProgress("Petrologist", 100)
        target:getPosition():sendMagicEffect(CONST_ME_BLOCKHIT)
        target:remove(1)
        return true
    end

    local tile = Tile(toPosition)
    if not tile then
        return false
    end

    local ground = tile:getGround()
    if not ground then
        return false
    end

    if table.contains(Tools.pickableGroundIds, ground.itemid) and ground.actionid == actionIds.pickHole then
        ground:transform(392)
        ground:decay()
        toPosition:sendMagicEffect(CONST_ME_POFF)

        toPosition.z = toPosition.z + 1
        tile:relocateTo(toPosition)
        return true
    end

    if ground.itemid == 7200 then
        ground:transform(7236)
        ground:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
        return true
    end

    return false
end

Tools.onUsePick = onUsePick
