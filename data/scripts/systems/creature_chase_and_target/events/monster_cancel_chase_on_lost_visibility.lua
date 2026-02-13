do
	local event = Event()

	function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		if targetCreature ~= nearbyCreature then
			return
		end

		local position = monster:getPosition()
		local targetPosition = targetCreature:getPosition()
		if position.z ~= targetPosition.z or not monster:canSee(targetPosition) then
			monster:setChaseCreature(nil)
		end
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureMoved(creature, fromTile, toTile)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		local position = monster:getPosition()
		local targetPosition = targetCreature:getPosition()
		if position.z ~= targetPosition.z or not monster:canSee(targetPosition) then
			monster:setChaseCreature(nil)
		end
	end

	event:register()
end
