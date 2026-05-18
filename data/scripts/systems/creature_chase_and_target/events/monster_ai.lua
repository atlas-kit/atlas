-- monster_ai.lua
-- Main AI loop for monsters: target acquisition, periodic target
-- re-evaluation, and chase/follow pathfinding. Runs every 1000ms via
-- the onCreatureThink event.
--
-- Uses monster:getPathTo() and monster:startAutoWalk() for movement.
-- Target re-evaluation follows the C++ onThinkTarget rules:
-- changeTargetSpeed, changeTargetChance, and targetDistance.

local targetChangeTicks = {}

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

		local hasMaster = monster:getMaster() ~= nil
		local position = monster:getPosition()

		if not monster:isInSpawnRange(position) then
			return
		end

		local targetCreature = monster:getTargetCreature()
		local chaseCreature = monster:getChaseCreature()
		local hasTargetList = monster:getTargetCount() > 0

		-- Target acquisition: pick a target from the list if we have none.
		if not targetCreature and not chaseCreature and hasTargetList and not hasMaster then
			monster:searchTarget(TARGETSEARCH_NEAREST)
			targetCreature = monster:getTargetCreature()
			chaseCreature = monster:getChaseCreature()
		end

		-- Periodic target re-evaluation for non-summon monsters.
		if not hasMaster and targetCreature and monster:getTargetCount() > 1 then
			local monsterType = monster:getType()
			local changeTargetSpeed = monsterType:changeTargetSpeed()
			if changeTargetSpeed ~= 0 then
				local id = monster:getId()
				local ticks = targetChangeTicks[id] or 0
				ticks = ticks + interval
				if ticks >= changeTargetSpeed then
					targetChangeTicks[id] = 0
					if monsterType:changeTargetChance() >= math.random(1, 100) then
						if monsterType:targetDistance() <= 1 then
							monster:searchTarget(TARGETSEARCH_RANDOM)
						else
							monster:searchTarget(TARGETSEARCH_NEAREST)
						end
					end
				else
					targetChangeTicks[id] = ticks
				end
			end
		elseif not targetCreature then
			targetChangeTicks[monster:getId()] = nil
		end

		-- Follow the chase target using A* pathfinding.
		chaseCreature = monster:getChaseCreature()
		if chaseCreature then
			local chasePosition = chaseCreature:getPosition()
			if position.z == chasePosition.z then
				local dirs = monster:getPathTo(chasePosition, 1, 1, true, true, 12)
				if dirs then
					monster:startAutoWalk(dirs)
				end
			end
		end
	end

	event:register()
end
