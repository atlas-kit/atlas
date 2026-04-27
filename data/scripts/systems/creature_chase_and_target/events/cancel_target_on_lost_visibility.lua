-- cancel_target_on_lost_visibility.lua

do
    local event = Event()

    -- Triggered when the creature itself moves.
    -- Verifies if the creature still has line of sight to its target from the new tile.
    function event.onCreatureMoved(creature, fromTile, toTile)
        local targetCreature = creature:getTargetCreature()
        if not targetCreature then
            return
        end

        local position = creature:getPosition()
        local targetPosition = targetCreature:getPosition()
        
        -- If target is on the same floor and visible, keep the target.
        if position.z == targetPosition.z and creature:canSee(targetPosition) then
            return
        end

        -- Clear target because it's no longer visible after moving.
        creature:setTargetCreature(nil)
    end

    event:register()
end

do
    local event = Event()

    -- Triggered when a nearby creature moves.
    -- Checks if the current target moved out of range or behind an object.
    function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
        local targetCreature = creature:getTargetCreature()
        if not targetCreature then
            return
        end

        -- Only proceed if the creature that moved is the actual target.
        if targetCreature ~= nearbyCreature then
            return
        end

        local position = creature:getPosition()
        local targetPosition = targetCreature:getPosition()
        
        -- Keep target if they are on the same floor and within line of sight.
        if position.z == targetPosition.z and creature:canSee(targetPosition) then
            return
        end

        -- Target moved to a different floor or behind an obstacle.
        creature:setTargetCreature(nil)
    end

    event:register()
end

do
    local event = Event()

    -- Periodic check (Think) to validate if the target is still reachable/visible.
    function event.onCreatureThink(creature, interval)
        local targetCreature = creature:getTargetCreature()
        if not targetCreature then
            return
        end

        -- Prevents clearing target if the creature is attacking its own master.
        local master = creature:getMaster()
        if master == targetCreature then
            return
        end

        -- If target is still visible, continue attacking.
        if creature:canSeeCreature(targetCreature) then
            return
        end

        -- Target lost during the periodic think cycle.
        creature:setTargetCreature(nil)
    end

    event:register()
end

do
    local event = Event()

    -- Triggered when a creature is removed from the game world (death, logout, teleport).
    function event.onCreatureNearbyCreatureRemoved(creature, nearbyCreature)
        local targetCreature = creature:getTargetCreature()
        if not targetCreature then
            return
        end

        -- If the creature being removed is our current target, clear it.
        if targetCreature ~= nearbyCreature then
            return
        end

        creature:setTargetCreature(nil)
    end

    event:register()
end
