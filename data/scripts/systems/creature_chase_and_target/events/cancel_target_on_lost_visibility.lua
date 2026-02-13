do
	local event = Event()

	function event.onCreatureMoved(creature, fromTile, toTile)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = targetCreature:getPosition()
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		creature:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		if targetCreature ~= nearbyCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = targetCreature:getPosition()
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		creature:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureThink(creature, interval)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		local master = creature:getMaster()
		if master == targetCreature then
			return
		end

		if creature:canSeeCreature(targetCreature) then
			return
		end

		creature:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureRemoved(creature, nearbyCreature)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		if targetCreature ~= nearbyCreature then
			return
		end

		creature:setTargetCreature(nil)
	end

	event:register()
end
