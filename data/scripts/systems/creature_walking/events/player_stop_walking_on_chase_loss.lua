-- player_stop_walking_on_chase_loss.lua
local event = Event()

-- Triggered whenever the player's chase/follow target changes.
function event.onCreatureChaseCreatureChanged(creature)
    -- Verify that the entity is a player.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- Check if the player has lost or cleared their chase target.
    -- This happens when the target is out of range, changes floor, or the player stops following.
    if not player:hasChaseCreature() then
        -- Force the player character to stop walking immediately.
        -- This prevents the player from continuing to walk toward the last known chase position.
        print(string.format("[player_stop_walking_on_chase_loss:onCreatureChaseCreatureChanged] Player %s: Chase lost, stopping all movement.", player:getName()))
        player:stopWalk()
    end
end

event:register()
