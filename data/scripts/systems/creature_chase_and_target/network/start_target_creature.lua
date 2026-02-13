-- start_target_creature.lua

-- Create a handler for packet 0xA1 (161).
-- This packet is sent by the client when a player clicks to attack a creature.
local handler = PacketHandler(0xA1)

-- Triggered when the server receives the 0xA1 packet.
function handler.onReceive(player, msg)
    -- Read the 32-bit ID of the creature the player wants to attack.
    local targetCreatureId = msg:getU32()
    
    -- Case: ID is 0. This happens when the player clicks on the ground or 
    -- deselects the current target manually.
    if targetCreatureId == 0 then
        print(string.format("[start_target_creature:Packet 0xA1] Player %s: Manual untarget (ID 0 received).", player:getName()))
        player:setTargetCreature(nil)
        return
    end

    -- Find the creature object in the game world.
    local targetCreature = Creature(targetCreatureId)
    if not targetCreature then
        return
    end

    local position = player:getPosition()
    local targetPosition = targetCreature:getPosition()
    
    -- Safety Check: Ensure the player can actually see the target and is on the same floor.
    if position.z ~= targetPosition.z or not player:canSee(targetPosition) then
        -- If they can't see the target, tell the client to clear the red square.
        player:sendCancelTarget()
        return
    end

    -- The following block is a placeholder for engine-level combat checks 
    -- (e.g., checking if the target is in a protection zone or if it's a teammate).
    -- ReturnValue ret = Combat::canTargetCreature(player, targetCreature);
    -- if (ret != RETURNVALUE_NOERROR) {
    --  player->sendCancelMessage(ret);
    --  player->sendCancelTarget();
    --  player->setTargetCreature(nullptr);
    --  return;
    -- }

    -- Log the successful target acquisition.
    print(string.format("[start_target_creature:Packet 0xA1] Player %s clicked to attack %s.", player:getName(), targetCreature:getName()))

    -- Set the server-side target, which will trigger attack ticks and combat logic.
    player:setTargetCreature(targetCreature)
end

handler:register()
