-- start_chase_creature.lua

-- Create a handler for packet 0xA2 (162).
-- This packet is sent by the client when a player clicks to "Follow" or "Chase" a creature.
local handler = PacketHandler(0xA2)

-- Triggered when the server receives the 0xA2 packet.
function handler.onReceive(player, msg)
    -- Read the 32-bit unsigned integer representing the Unique ID of the creature to chase.
    local chaseCreatureId = msg:getU32()
    
    -- If the ID is 0, it is an invalid target; exit the function.
    if chaseCreatureId == 0 then
        return
    end

    -- Attempt to find the creature object in the game world using the provided ID.
    local chaseCreature = Creature(chaseCreatureId)
    if not chaseCreature then
        return
    end

    -- Log the chase request for debugging.
    print(string.format("[PacketHandler 0xA2] Player %s is now chasing %s.", player:getName(), chaseCreature:getName()))

    -- Set the server-side chase target, initiating the auto-walk behavior toward that creature.
    player:setChaseCreature(chaseCreature)
end

handler:register()
