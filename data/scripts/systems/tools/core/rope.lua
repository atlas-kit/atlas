Tools = Tools or {}

function onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
    local tile = Tile(toPosition)
    if not tile then
        return false
    end

    local ground = tile:getGround()
    if (ground and table.contains(ropeSpots, ground:getId())) or tile:getItemById(14435) then
        tile = Tile(toPosition:moveUpstairs())
        if not tile then
            return false
        end

        if tile:hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
            player:sendCancelMessage(RETURNVALUE_PLAYERISPZLOCKED)
            return true
        end

        return player:teleportTo(toPosition, false)
    end

    if table.contains(Tools.ropeHoleIds, target.itemid) then
        toPosition.z = toPosition.z + 1
        tile = Tile(toPosition)
        if not tile then
            return false
        end

        local thing = tile:getTopVisibleThing()
        if not thing then
            return true
        end

        if thing:isPlayer() then
            if Tile(toPosition:moveUpstairs()):queryAdd(thing) ~= RETURNVALUE_NOERROR then
                return false
            end

            return thing:teleportTo(toPosition, false)
        elseif thing:isItem() and thing:getType():isMovable() then
            return thing:moveTo(toPosition:moveUpstairs())
        end

        return true
    end

    return false
end

Tools.onUseRope = onUseRope
