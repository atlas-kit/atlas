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

    -- Only cancel the attack if the creature being chased is not the creature being attacked.
    if targetCreature ~= chaseCreature then
        player:setTargetCreature(nil)
    end
end

event:register()
