-- cancel_target_creature.lua

-- Create a handler for packet 0xBE (190).
-- In most game clients, this packet is sent when the player clicks the "Stop" button or clears their target.
local handler = PacketHandler(0xBE)

-- Triggered when the server receives the 0xBE packet from a player.
function handler.onReceive(player, msg)
    -- Log the request to the console for debugging purposes.
    print(string.format("[PacketHandler 0xBE] Player %s requested to cancel their target.", player:getName()))

    -- Set the player's target to nil, effectively stopping the attack server-side.
    player:setTargetCreature(nil)   
end

-- Register the handler so the server listens for this packet ID.
handler:register()
