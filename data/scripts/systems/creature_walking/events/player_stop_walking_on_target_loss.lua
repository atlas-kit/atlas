-- player_stop_walking_on_target_loss.lua
local event = Event()

-- Triggered whenever the player's attack target changes or is lost.
function event.onCreatureTargetCreatureChanged(creature)
    -- Verify that the entity is a player.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- Check if the player no longer has an active attack target.
    -- This occurs when the target dies, teleports away, or the player manually deselects them.
    if not player:hasTargetCreature() then
        -- Cancel current movement to prevent the player from walking into danger 
        -- or continuing toward a target that no longer exists.
        player:stopWalk()
    end
end

event:register()
