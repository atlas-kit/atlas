local event = Event()

function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
	local monster = creature:asMonster()
	if not monster then
		return
	end

	if monster:isSummon() then
		return
	end

	local chaseCreature = monster:getChaseCreature()
	if not chaseCreature then
		if monster:isOpponent(nearbyCreature) then
			monster:selectTarget(nearbyCreature)
		end
		return
	end

	local monsterType = MonsterType(monster)
	if not monsterType then
		return
	end

	if monsterType:changeTargetChance() <= 0 then
		return
	end

	local position = creature:getPosition()
	local targetPosition = chaseCreature:getPosition()

	local offsetx = targetPosition:getDistanceX(position)
	local offsety = targetPosition:getDistanceY(position)
	if offsetx <= 1 and offsety <= 1 then
		return
	end

	local direction = position:getDirectionTo(followPosition)
	local nextPosition = position:getNextPosition(direction)

	local tile = Tile(nextPosition)
	if not tile then
		return
	end

	local tileTopCreature = tile:getTopCreature()
	if not tileTopCreature then
		return
	end

	if chaseCreature == tileTopCreature then
		return
	end

	if monster:isOpponent(tileTopCreature) then
		monster:selectTarget(tileTopCreature)
	end
end

event:register()
