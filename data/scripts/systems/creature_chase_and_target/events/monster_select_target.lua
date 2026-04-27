-- monster_select_target.lua
local event = Event()

-- Triggered when a nearby creature moves, allowing the monster to evaluate or switch targets.
function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
    -- Ensure the creature is a monster.
    local monster = creature:asMonster()
    if not monster then
        return
    end

    -- Skip logic if the monster is a summon (summons usually follow master's target).
    if monster:isSummon() then
        return
    end

    local chaseCreature = monster:getChaseCreature()
    
    -- Case: The monster currently has no target/chase.
    if not chaseCreature then
        -- If the creature that moved is an opponent, select it as the new target.
        if monster:isOpponent(nearbyCreature) and monster:canTarget(nearbyCreature) then
            monster:selectTarget(nearbyCreature)
        end
        return
    end

    -- Fetch the monster's configuration type.
    local monsterType = MonsterType(monster)
    if not monsterType then
        return
    end

    -- If the monster type is configured to never change targets, stop here.
    if monsterType:changeTargetChance() <= 0 then
        return
    end

    local position = creature:getPosition()
    local targetPosition = chaseCreature:getPosition()

    -- If the current target is already adjacent (distance <= 1), there is no need to switch based on movement.
    local offsetx = targetPosition:getDistanceX(position)
    local offsety = targetPosition:getDistanceY(position)
    if offsetx <= 1 and offsety <= 1 then
        return
    end

    -- Logic to check if a better opponent is blocking the path to the current target.
    local direction = position:getDirectionTo(followPosition) -- Note: Ensure followPosition is defined in your engine
    local nextPosition = position:getNextPosition(direction)

    local tile = Tile(nextPosition)
    if not tile then
        return
    end

    -- Check if there is a creature on the tile the monster wants to step into.
    local tileTopCreature = tile:getTopCreature()
    if not tileTopCreature then
        return
    end

    -- If the creature on the next tile is already our target, do nothing.
    if chaseCreature == tileTopCreature then
        return
    end

    -- if a different opponent is standing right in front of the monster, switch target to them.
    if monster:isOpponent(tileTopCreature) and monster:canTarget(tileTopCreature) then
        monster:selectTarget(tileTopCreature)
    end
end

event:register()
