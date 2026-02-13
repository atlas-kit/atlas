-- player_infight_on_targeted.lua

local condition = Condition(CONDITION_INFIGHT, CONDITIONID_DEFAULT) 

local event = Event()

-- Triggered when a player selects a target to attack.
-- This ensures the player gains the 'In-Fight' (Battle) condition immediately.
function event.onCreatureTargetCreatureChanged(creature)    
    -- Ensure the entity is a player.
    local player = creature:asPlayer()
    if not player then
        return
    end

    -- If the player just cleared their target (is no longer attacking), stop here.
    local targetCreature = player:getTargetCreature()
    if not targetCreature then
        return
    end

    -- Do not apply the condition if the player has a flag that prevents gaining 'In-Fight' status.
    if player:hasFlag(PlayerFlag_NotGainInFight) then
        return
    end
    
    -- Set the condition duration based on the server's PZ_LOCKED configuration (usually 60 seconds).
    condition:setTicks(configManager.getNumber(configKeys.PZ_LOCKED))
    
    -- Apply the battle sign (In-Fight) to the player.
    print(string.format("[player_infight_on_targeted:onCreatureTargetCreatureChanged] Player %s gained In-Fight condition by targeting %s.", player:getName(), targetCreature:getName()))
    player:addCondition(condition)
end

event:register()