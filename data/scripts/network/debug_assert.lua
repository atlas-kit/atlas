local handler = PacketHandler(0xE8)

local function saveDebugAssert(player, assertLine, date, description, comment)
    local file = io.open(string.format("data/logs/debug_assert/%s.log", player:getName()), "a")
    if not file then
        print(string.format("[Error] Failed to open debug assert log for player: %s", playerName))
        return
    end

    file:write(string.format("[%s]\n", date))
    file:write(string.format("assertLine: %s\n", assertLine))
    file:write(string.format("description: %s\n", description))
    file:write(string.format("comment: %s\n\n", comment))
    file:close()
end

function handler.onReceive(player, msg)
    local playerGuid = player:getGuid()
    local assertLine = msg:getString()
    local date = msg:getString()
    local description = msg:getString()
    local comment = msg:getString()

    saveDebugAssert(player, assertLine, date, description, comment)
end

handler:register()
