do
	local event = Event()

	function event.onCreatureMoved(creature, fromTile, toTile)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = chaseCreature:getPosition()
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureMoved(creature, nearbyCreature, fromTile, toTile)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		if chaseCreature ~= nearbyCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = chaseCreature:getPosition()
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureThink(creature, interval)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		local master = creature:getMaster()
		if master == chaseCreature then
			return
		end

		if creature:canSeeCreature(chaseCreature) then
			return
		end
		
		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureRemoved(creature, nearbyCreature)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		if chaseCreature ~= nearbyCreature then
			return
		end

		creature:setChaseCreature(nil)
	end

	event:register()
end
