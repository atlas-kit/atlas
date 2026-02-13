do
	local event = Event()

	function event.onCreatureZoneChanged(creature)
		local player = creature:asPlayer()
		if not player then
			return
		end

		if not player:hasTargetCreature() then
			return
		end

		local zone = player:getZone()
		if zone ~= ZONE_PROTECTION then
			return
		end

		if player:hasFlag(PlayerFlag_IgnoreProtectionZone) then
			return
		end

		player:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	function event.onCreatureNearbyCreatureZoneChanged(creature, nearbyCreature)
		local player = creature:asPlayer()
		if not player then
			return
		end

		local targetCreature = player:getTargetCreature()
		if not targetCreature then
			return
		end

		if targetCreature ~= nearbyCreature then
			return
		end

		if player:hasFlag(PlayerFlag_IgnoreProtectionZone) then
			return
		end

		local zone = targetCreature:getZone()
		if zone == ZONE_PROTECTION then
			player:setTargetCreature(nil)
		elseif zone == ZONE_NOPVP then
			if targetCreature:isPlayer() then
				player:setTargetCreature(nil)
			end
		elseif zone == ZONE_NORMAL then
			if targetCreature:isPlayer() then
				if Game.getWorldType() == WORLD_TYPE_NO_PVP then
					player:setTargetCreature(nil)
				end
			end
		end
	end

	event:register()
end
