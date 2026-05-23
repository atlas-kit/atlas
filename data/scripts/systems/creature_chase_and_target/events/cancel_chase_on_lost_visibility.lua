-- cancel_chase_on_lost_visibility.lua
-- Handles automatic chase/follow cancellation when the followed creature
-- goes out of sight, changes floor, or is removed from the game world.
-- Uses both event-driven (move, remove) and periodic (Think) checks.

do
	local event = Event()

	-- Triggered when the creature itself moves.
	-- Checks if the new position still allows visibility of the target.
	function event.onCreatureMove(creature, fromTile, toTile)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		-- Do not cancel if the target is the creature's master (e.g., summons).
		local master = creature:getMaster()
		if master == chaseCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = chaseCreature:getPosition()
		
		-- If the creature is on the same floor and can see the target, keep chasing.
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		-- Stop chasing if visibility is lost after the move.
		local player = creature:asPlayer()
		if player then
			player:sendCancelTarget()
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		end
		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	-- Triggered when a nearby creature (potentially the target) moves.
	function event.onCreatureMoveNearbyCreature(creature, nearbyCreature, fromTile, toTile)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		-- Do not cancel if the target is the creature's master (e.g., summons).
		local master = creature:getMaster()
		if master == chaseCreature then
			return
		end

		-- Ensure the creature moving is actually the one being chased.
		if chaseCreature ~= nearbyCreature then
			return
		end

		local position = creature:getPosition()
		local targetPosition = chaseCreature:getPosition()
		
		-- If target is still on the same floor and visible, keep chasing.
		if position.z == targetPosition.z and creature:canSee(targetPosition) then
			return
		end

		-- Stop chasing if the target moved out of sight or changed floors.
		local player = creature:asPlayer()
		if player then
			player:sendCancelTarget()
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		end
		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	-- Periodic check (Think) to maintain chase status.
	function event.onCreatureThink(creature, interval)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		-- Do not cancel if the target is the creature's master (e.g., summons).
		local master = creature:getMaster()
		if master == chaseCreature then
			return
		end

		-- If the creature can still see the target, do nothing.
		if creature:canSeeCreature(chaseCreature) then
			return
		end
		
		-- Cancel chase if visibility is lost during the think cycle.
		local player = creature:asPlayer()
		if player then
			player:sendCancelTarget()
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		end
		creature:setChaseCreature(nil)
	end

	event:register()
end

do
	local event = Event()

	-- Triggered when a creature is removed from the vicinity (teleport, logout, or death).
	function event.onCreatureRemoveNearbyCreature(creature, nearbyCreature)
		local chaseCreature = creature:getChaseCreature()
		if not chaseCreature then
			return
		end

		-- If the removed creature is the chase target, clear the chase.
		if chaseCreature ~= nearbyCreature then
			return
		end

		local player = creature:asPlayer()
		if player then
			player:sendCancelTarget()
		end

		creature:setChaseCreature(nil)
	end

	event:register()
end
