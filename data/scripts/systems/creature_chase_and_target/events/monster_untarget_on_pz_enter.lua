-- monster_untarget_on_pz_enter.lua
-- Clears monster targets when either the monster or its target enters a
-- Protection Zone. Also clears the chase target and resets attack ticks
-- to prevent the monster from attacking into or from safe zones.

do
	local event = Event()

	-- Triggered when the monster itself changes zones.
	-- Checks if either the monster or its target entered a Protection Zone.
	function event.onCreatureChangeZone(creature, fromZone, toZone)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		-- If the monster entered PZ, clear target/chase regardless.
		if toZone == ZONE_PROTECTION then
			monster:setTargetCreature(nil)
			monster:setChaseCreature(nil)
			monster:resetAttackTicks()
			return
		end

		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		-- Check the zone of the creature the monster is attacking.
		local zone = targetCreature:getZone()
		if zone ~= ZONE_PROTECTION then
			return
		end

		-- Clear target because the opponent is in a safe zone.
		monster:setTargetCreature(nil)
		monster:setChaseCreature(nil)
		monster:resetAttackTicks()
	end

	event:register()
end

do
	local event = Event()

	-- Triggered when a nearby creature (the target) changes zones.
	function event.onCreatureChangeNearbyCreatureZone(creature, nearbyCreature, fromZone, toZone)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		-- Verify if the creature that changed zones is actually the monster's target.
		if targetCreature ~= nearbyCreature then
			return
		end

		-- If the target moved into a Protection Zone, stop the attack.
		if toZone ~= ZONE_PROTECTION then
			return
		end

		monster:setTargetCreature(nil)
		monster:setChaseCreature(nil)
		monster:resetAttackTicks()
	end

	event:register()
end
