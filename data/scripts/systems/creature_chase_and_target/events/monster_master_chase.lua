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

    if monster:isSummon() then
        return
    end

    if 
end

event:register()
