-- player_cancel_target_on_chase.lua
local event = Event()

-- Triggered whenever the player's chase/follow target changes.
-- This script ensures that if a player starts following a different creature, they stop attacking their current target.
function event.onCreatureChaseCreatureChanged(creature)
    -- Ensure the entity is a player.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- Get the current attack target.
    local targetCreature = player:getTargetCreature()
    if not targetCreature then
        return
    end

    -- Get the new chase/follow target.
    local chaseCreature = player:getChaseCreature()
    if not chaseCreature then
        return
    end

    -- Logic: Only cancel the attack if the creature being chased is NOT the same as the creature being attacked.
    -- This allows "Attack + Follow" on the same target, but clears attack if you switch to following a friend/different enemy.
    if targetCreature ~= chaseCreature then
        print(string.format("[player_cancel_target_on_chase:onCreatureChaseCreatureChanged] Player %s: Followed a different creature. Clearing attack target: %s.", player:getName(), targetCreature:getName()))
        player:setTargetCreature(nil)
    end
end

event:register()
