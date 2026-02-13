do
	local event = Event()

	function event.onCreatureZoneChanged(creature)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		local zone = targetCreature:getZone()
		if zone ~= ZONE_PROTECTION then
			return
		end

		monster:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureZoneChanged(creature, nearbyCreature)
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

		local zone = targetCreature:getZone()
		if zone ~= ZONE_PROTECTION then
			return
		end

		monster:setTargetCreature(nil)
	end

	event:register()
end
