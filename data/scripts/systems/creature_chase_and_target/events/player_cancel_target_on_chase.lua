-- player_cancel_chase_on_untarget.lua
local event = Event()

-- Triggered whenever a creature's target changes.
-- This handles stopping the chase movement when the player stops attacking.
function event.onCreatureTargetCreatureChanged(creature)
    -- Verify that the creature is a player.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- If the player isn't chasing anyone, there's no movement to cancel.
    if not player:hasChaseCreature() then
        return
    end

    -- Check if the player no longer has an active target.
    -- This occurs when the target dies, logs out, or the player manually stops the attack.
    if not player:hasTargetCreature() then
        -- Stop the player from chasing/following since there is no longer a target.
        print(string.format("[player_cancel_chase_on_untarget:onCreatureTargetCreatureChanged] Player %s: Target lost, cancelling chase.", player:getName()))
        player:setChaseCreature(nil)
    end
end

event:register()
