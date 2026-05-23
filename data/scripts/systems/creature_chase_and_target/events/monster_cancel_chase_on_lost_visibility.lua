-- monster_cancel_chase_on_lost_visibility.lua
-- Monster-specific chase cancellation on visibility loss.
-- Separate from the generic cancel_chase handler because monsters
-- only check on move events, not on Think or creature removal.

do
	local event = Event()

	-- Triggered when a nearby creature moves.
	-- This specific block checks if the monster's target has moved out of sight.
	function event.onCreatureMoveNearbyCreature(creature, nearbyCreature, fromTile, toTile)
		-- Ensure the creature executing this logic is a monster.
		local monster = creature:asMonster()
		if not monster then
			return
		end

		-- Check if the monster currently has a target.
		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		-- Only proceed if the creature that moved is the monster's current target.
		if targetCreature ~= nearbyCreature then
			return
		end

		local position = monster:getPosition()
		local targetPosition = targetCreature:getPosition()
		
		-- If the target changes floor (Z) or the monster loses line of sight:
		if position.z ~= targetPosition.z or not monster:canSee(targetPosition) then
			monster:setChaseCreature(nil)
		end
	end

	event:register()
end

do
	local event = Event()

	-- Triggered when the monster itself moves.
	-- Verifies if the monster can still see its target from its new position.
	function event.onCreatureMove(creature, fromTile, toTile)
		-- Ensure the creature executing this logic is a monster.
		local monster = creature:asMonster()
		if not monster then
			return
		end

		-- Check if the monster currently has a target.
		local targetCreature = monster:getTargetCreature()
		if not targetCreature then
			return
		end

		local position = monster:getPosition()
		local targetPosition = targetCreature:getPosition()
		
		-- If the monster moved to a different floor or a spot where it cannot see the target:
		if position.z ~= targetPosition.z or not monster:canSee(targetPosition) then
			monster:setChaseCreature(nil)
		end
	end

	event:register()
end
