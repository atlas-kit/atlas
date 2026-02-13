-- player_auto_chase_on_target.lua
local event = Event()

-- Triggered when a player selects a new target or loses their current one.
function event.onCreatureTargetCreatureChanged(creature)
    -- Ensure the logic only applies to players.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- If the player has no target (target cleared), stop here.
    local targetCreature = player:getTargetCreature()
    if not targetCreature then
        return
    end

    -- Flow: If 'Secure Mode' (Hand icon) is active, the player will automatically chase the target.
    -- If Secure Mode is off, chase is set to nil (stand still).
    local shouldChase = player:hasChaseMode() and targetCreature or nil
    
    if shouldChase then
        print(string.format("[player_auto_chase_on_target:onCreatureTargetCreatureChanged] Player %s is now automatically chasing %s.", player:getName(), targetCreature:getName()))
    end

    player:setChaseCreature(shouldChase)
end

event:register()
