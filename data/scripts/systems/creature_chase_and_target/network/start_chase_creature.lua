-- start_chase_creature.lua

-- Create a handler for packet 0xA2 (162).
-- This packet is sent by the client when a player clicks to "Follow" or "Chase" a creature.
local handler = PacketHandler(0xA2)

-- Triggered when the server receives the 0xA2 packet.
function handler.onReceive(player, msg)
    -- Read the 32-bit unsigned integer representing the Unique ID of the creature to chase.
    local chaseCreatureId = msg:getU32()
    
    -- Case: ID is 0. This happens when the player clicks to stop chasing.
    if chaseCreatureId == 0 then
        -- Updated log and function to reflect 'Chase' instead of 'Target'
        print(string.format("[start_chase_creature:Packet 0xA2] Player %s: Manual stop chase (ID 0 received).", player:getName()))
        player:setChaseCreature(nil)
        return
    end

    -- Attempt to find the creature object in the game world using the provided ID.
    local chaseCreature = Creature(chaseCreatureId)
    if not chaseCreature then
        return
    end

    -- Log the chase request for debugging.
    print(string.format("[start_chase_creature:Packet 0xA2] Player %s is now chasing %s.", player:getName(), chaseCreature:getName()))

    -- Set the server-side chase target, initiating the auto-walk behavior toward that creature.
    player:setChaseCreature(chaseCreature)
end

handler:register()
