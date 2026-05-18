-- monster_ai.lua
-- Main AI loop for monsters: handles target acquisition, chase/follow
-- pathfinding, and periodic target re-evaluation. Runs on the
-- onCreatureThink event (every 1000ms).

do
	local event = Event()

	function event.onCreatureThink(creature, interval)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		if monster:isIdle() then
			return
		end

		local targetCreature = monster:getTargetCreature()
		local chaseCreature = monster:getChaseCreature()

		-- Acquire a target from the target list if we have none.
		if not targetCreature and not chaseCreature and monster:getTargetCount() > 0 then
			monster:searchTarget(TARGETSEARCH_NEAREST)
			targetCreature = monster:getTargetCreature()
			chaseCreature = monster:getChaseCreature()
		end

		-- Follow the chase target using A* pathfinding.
		if chaseCreature then
			local position = monster:getPosition()
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
