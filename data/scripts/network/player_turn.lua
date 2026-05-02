local packetTypes = {
    [0x6F] = DIRECTION_NORTH,
    [0x70] = DIRECTION_EAST,
    [0x71] = DIRECTION_SOUTH,
    [0x72] = DIRECTION_WEST
}

local function playerTurn(playerId, direction)
    local player = Player(playerId)
    if not player then
        return
    end

    if not player:onTurn(direction) then
        return
    end

    player:setDirection(direction)
end

for type, direction in pairs(packetTypes) do
    local handler = PacketHandler(type)

    function handler.onReceive(player, msg)
        addEvent(playerTurn, SCHEDULER_MINTICKS, player:getId(), direction)
    end

    handler:register()
end
