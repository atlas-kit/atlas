-- monster_ai.lua
-- Main AI loop for monsters: target acquisition, periodic target
-- re-evaluation, and chase/follow pathfinding. Runs every 1000ms via
-- the onCreatureThink event.
--
-- Uses monster:getPathTo() and monster:startAutoWalk() for movement.
-- Target re-evaluation follows the C++ onThinkTarget rules:
-- changeTargetSpeed, changeTargetChance, and targetDistance.

local targetChangeTicks = {}

local function reevaluateTarget(monster, interval)
	local id = monster:getId()
	local monsterType = monster:getType()

	if monsterType:changeTargetSpeed() == 0 then
		return
	end

	local ticks = (targetChangeTicks[id] or 0) + interval
	if ticks < monsterType:changeTargetSpeed() then
		targetChangeTicks[id] = ticks
		return
	end

	targetChangeTicks[id] = 0

	if monsterType:changeTargetChance() < math.random(1, 100) then
		return
	end

	if monsterType:targetDistance() <= 1 then
		monster:searchTarget(TARGETSEARCH_RANDOM)
	else
		monster:searchTarget(TARGETSEARCH_NEAREST)
	end
end

do
	local event = Event()

	function event.onCreatureRemoved(creature)
		targetChangeTicks[creature:getId()] = nil
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureThink(creature, interval)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		if monster:isIdle() then
			targetChangeTicks[monster:getId()] = nil
			return
		end

		local position = monster:getPosition()
		if not monster:isInSpawnRange(position) then
			targetChangeTicks[monster:getId()] = nil
			return
		end

		local hasMaster = monster:getMaster() ~= nil
		local targetCreature = monster:getTargetCreature()

		-- Target acquisition.
		if not targetCreature and not monster:getChaseCreature() and monster:getTargetCount() > 0 and not hasMaster then
			monster:searchTarget(TARGETSEARCH_NEAREST)
			targetCreature = monster:getTargetCreature()
		end

		-- Target re-evaluation or state cleanup.
		if not targetCreature then
			targetChangeTicks[monster:getId()] = nil
		elseif not hasMaster and monster:getTargetCount() > 1 then
			reevaluateTarget(monster, interval)
		end

		-- Chase pathfinding.
		local chaseCreature = monster:getChaseCreature()
		if not chaseCreature then
			return
		end

		local chasePosition = chaseCreature:getPosition()
		if position.z ~= chasePosition.z then
			return
		end

		local dirs = monster:getPathTo(chasePosition, 1, 1, true, true, 0)
		if dirs then
			monster:startAutoWalk(dirs)
		end
	end

	event:register()
end
