local event = Event()

local function getClientIndexOfCreature(player, creature)
    local tile = creature:getTile()
    local n = 0

    if tile:getGround() ~= nil then
        n = n + 1
    end

    n = n + tile:getTopItemCount()

    local tileCreatures = tile:getCreatures()
    for i = #tileCreatures, 1, -1 do
        local tileCreature = tileCreatures[i]
        if tileCreature == creature then
            return n
        end

        if player:canSeeCreature(tileCreature) then
            n = n + 1
        end
    end

    return -1
end

local function canWalkthroughEx(player, creature)
    if player:getGroup():getAccess() then
        return true
    end

    if creature:isPlayer() and not configManager.getBoolean(configKeys.ALLOW_WALKTHROUGH) then
        return false
    end

    local tile = creature:getTile()
    if not tile then
        return false
    end

    return tile:hasFlag(TILESTATE_PROTECTIONZONE) or player:getLevel() <=
               configManager.getNumber(configKeys.PROTECTION_LEVEL)
end

local function sendCreatureTurn(player, creature, stackpos, direction)
    if not player:canSeeCreature(creature) then
        return
    end

    local msg = NetworkMessage()
    msg:addByte(0x6B)
    if stackpos >= MAX_STACKPOS then
        msg:addU16(0xFFFF)
        msg:addU32(creature:getId())
    else
        msg:addPosition(creature:getPosition())
        msg:addByte(stackpos)
    end

    msg:addU16(0x63)
    msg:addU32(creature:getId())
    msg:addByte(direction)
    msg:addBool(not canWalkthroughEx(player, creature))
    return msg:sendToPlayer(player)
end

function event.onPlayerTurn(player, direction)
    if player:getDirection() == direction then
        return false
    end

    player:resetIdleTime()

    local spectators = Game.getSpectators(player:getPosition(), true, true)
    for _, spectator in ipairs(spectators) do
        local stackpos = getClientIndexOfCreature(spectator, player)
        if stackpos ~= -1 then
            sendCreatureTurn(spectator, player, stackpos, direction)
        end
    end

    return true
end

event:register()
