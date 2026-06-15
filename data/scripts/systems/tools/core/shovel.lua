Tools = Tools or {}

function onUseShovel(player, item, fromPosition, target, toPosition, isHotkey)
    local tile = Tile(toPosition)
    if not tile then
        return false
    end

    local ground = tile:getGround()
    if not ground then
        return false
    end

    local groundId = ground:getId()
    if table.contains(Tools.shovelHoleIds, groundId) then
        ground:transform(groundId + 1)
        ground:decay()

        toPosition.z = toPosition.z + 1
        tile:relocateTo(toPosition)
        player:addAchievementProgress("The Undertaker", 500)
        return true
    end

    if target.itemid == 7932 then
        target:transform(7933)
        target:decay()
        player:addAchievementProgress("The Undertaker", 500)
        return true
    end

    if target.itemid == 20230 then
        local storage = player:getStorageValue(PlayerStorageKeys.swampDigging)
        if storage <= os.time() then
            local chance = math.random(100)
            if chance <= 42 then
                player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a dead snake.")
                player:addItem(3077)
            elseif chance <= 79 then
                player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a small diamond.")
                player:addItem(2145)
            else
                player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a leech.")
                player:addItem(20138)
            end

            player:setStorageValue(PlayerStorageKeys.swampDigging, os.time() + 7 * 24 * 60 * 60)
            player:getPosition():sendMagicEffect(CONST_ME_GREEN_RINGS)
        end
        return true
    end

    if table.contains(Tools.sandGroundIds, groundId) then
        local randomValue = math.random(1, 100)
        if target.actionid == actionIds.sandHole and randomValue <= 20 then
            ground:transform(489)
            ground:decay()
        elseif randomValue == 1 then
            Game.createItem(2159, 1, toPosition)
            player:addAchievementProgress("Gold Digger", 100)
        elseif randomValue > 95 then
            Game.createMonster("Scarab", toPosition)
        end
        toPosition:sendMagicEffect(CONST_ME_POFF)
        return true
    end

    return false
end

Tools.onUseShovel = onUseShovel
