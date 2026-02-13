-- monster_chase.lua
local event = Event()

-- Periodic check to manage the monster's behavior (targeting or following).
function event.onCreatureThink(creature, interval)
    -- Ensure the creature is a monster.
    local monster = creature:asMonster()
    if not monster then
        return
    end

    -- Skip logic if the monster is idle (not doing anything).
    if monster:isIdle() then
        return
    end

    -- Check if the monster has a master (summons/pets).
    local master = monster:getMaster()
    if not master then
        return
    end

    -- Ensure the monster is still within its allowed spawn/range.
    local position = monster:getPosition()
    if not monster:isInSpawnRange(position) then
        return
    end

    local chaseCreature = monster:getChaseCreature()
    local targetCreature = monster:getTargetCreature()

    -- Case 1: The monster currently has no target.
    if not targetCreature then
        local masterTargetCreature = master:getTargetCreature()
        
        -- If the master is attacking something, the monster should help.
        if masterTargetCreature then
            -- this happens if the monster is summoned during combat.
            print(string.format("[monster_chase:onCreatureThink] %s: Selecting master's target.", monster:getName()))
            monster:selectTarget(masterTargetCreature)
            
        -- If no target exists, ensure the monster follows its master.
        elseif master ~= chaseCreature then
            -- our master has not ordered us to attack anything, lets follow him around instead.
            print(string.format("[monster_chase:onCreatureThink] %s: Following master.", monster:getName()))
            monster:setChaseCreature(master)
        end
        
    -- Case 2: The monster's target is somehow itself (safety check).
    elseif targetCreature == monster then
        print(string.format("[monster_chase:onCreatureThink] %s: Target is self, clearing chase.", monster:getName()))
        monster:setChaseCreature(nil)
        
    -- Case 3: The monster has a target but is not chasing/following it yet.
    elseif targetCreature ~= chaseCreature then
        -- this happens just after a master orders an attack, so lets follow it as well.
        print(string.format("[monster_chase:onCreatureThink] %s: Now chasing target: %s.", monster:getName(), targetCreature:getName()))
        monster:setChaseCreature(targetCreature)
    end
end

event:register()
