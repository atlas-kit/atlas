-- cancel_target_on_lost_visibility.lua
-- Cancels the attack target (and chase sync) when the target becomes
-- invisible, changes floor, or is removed.

-- Helper shared by all handlers: clears target + chase sync + monster
-- attack cooldown when a target is lost due to visibility, zone, or removal.
local function clearTarget(creature, targetCreature)
	local player = creature:asPlayer()
	if player then
		if player:getChaseCreature() == targetCreature then
			player:setChaseCreature(nil)
		end
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		creature:setTargetCreature(nil)
		return
	end

	local monster = creature:asMonster()
	if monster then
		monster:resetAttackTicks()
	end
	creature:setTargetCreature(nil)
end

do
	local event = Event()

	-- Target lost after the creature itself moves out of sight.
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

		clearTarget(creature, targetCreature)
	end

	event:register()
end

do
	local event = Event()

	-- Target lost after a nearby creature (our target) moves out of sight.
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

		clearTarget(creature, targetCreature)
	end

	event:register()
end

do
	local event = Event()

	-- Target lost during periodic visibility check.
	function event.onCreatureThink(creature, interval)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		if creature:getMaster() == targetCreature then
			return
		end

		if creature:canSeeCreature(targetCreature) then
			return
		end

		clearTarget(creature, targetCreature)
	end

	event:register()
end

do
	local event = Event()

	-- Target lost because the creature was removed (death, teleport, logout).
	function event.onCreatureNearbyCreatureRemoved(creature, nearbyCreature)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		if targetCreature ~= nearbyCreature then
			return
		end

		clearTarget(creature, targetCreature)
	end

	event:register()
end
