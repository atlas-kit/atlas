-- player_untarget_on_pz_enter.lua
-- Clears the player's attack target (and chase sync) when either the
-- player or their target enters a restricted zone. Handles Protection
-- Zones, No-PVP zones, and Optional-PVP world type rules. Respects
-- the IgnoreProtectionZone GM flag.

do
	local event = Event()

	-- Triggered when the player themselves changes zones.
	-- Prevents players from maintaining an active target while inside a Protection Zone.
	function event.onCreatureZoneChanged(creature)
		local player = creature:asPlayer()
		if not player then
			return
		end

		-- If the player isn't attacking anyone, no action is needed.
		if not player:hasTargetCreature() then
			return
		end

		-- Check if the player has entered a Protection Zone (PZ).
		local zone = player:getZone()
		if zone ~= ZONE_PROTECTION then
			return
		end

		-- Respect player flags (e.g., GMs) that allow attacking from within a PZ.
		if player:hasFlag(PlayerFlag_IgnoreProtectionZone) then
			return
		end

		-- Cancel target because the attacker entered a safe zone.
		if player:getChaseCreature() == player:getTargetCreature() then
			player:setChaseCreature(nil)
		end
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		player:setTargetCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	-- Triggered when a nearby creature (potentially the target) changes zones.
	function event.onCreatureNearbyCreatureZoneChanged(creature, nearbyCreature)
		local player = creature:asPlayer()
		if not player then
			return
		end

		local targetCreature = player:getTargetCreature()
		if not targetCreature then
			return
		end

		-- Ensure the zone change event belongs to the player's current target.
		if targetCreature ~= nearbyCreature then
			return
		end

		-- Respect special flags that ignore zone restrictions.
		if player:hasFlag(PlayerFlag_IgnoreProtectionZone) then
			return
		end

		local zone = targetCreature:getZone()
		local isTargetPlayer = targetCreature:isPlayer()

		-- Logic for different zone types:
		if zone == ZONE_PROTECTION then
			-- Case 1: Target entered a Protection Zone.
			if player:getChaseCreature() == targetCreature then
				player:setChaseCreature(nil)
			end
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
			player:setTargetCreature(nil)

		elseif zone == ZONE_NOPVP and isTargetPlayer then
			-- Case 2: Target entered a No-PVP zone and the target is a player.
			if player:getChaseCreature() == targetCreature then
				player:setChaseCreature(nil)
			end
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
			player:setTargetCreature(nil)

		elseif zone == ZONE_NORMAL and isTargetPlayer and Game.getWorldType() == WORLD_TYPE_NO_PVP then
			-- Case 3: In Optional-PVP worlds, players cannot be targeted in normal zones.
			if player:getChaseCreature() == targetCreature then
				player:setChaseCreature(nil)
			end
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
			player:setTargetCreature(nil)
		end
	end

	event:register()
end
