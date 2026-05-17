-- player_infight_on_targeted.lua
-- Applies the in-fight condition (PZ-lock timer) when a player selects
-- an attack target. Also clears targets for players who log in while
-- inside a Protection Zone with an active target.

do
	local function addInFightTicks(player)
		if player:hasFlag(PlayerFlag_NotGainInFight) then
			return
		end

		local condition = Condition(CONDITION_INFIGHT, CONDITIONID_DEFAULT)
		condition:setTicks(configManager.getNumber(configKeys.PZ_LOCKED))
		player:addCondition(condition)
	end

	local event = Event()

	function event.onCreatureTargetCreatureChanged(creature)
		local targetCreature = creature:getTargetCreature()
		if not targetCreature then
			return
		end

		local player = creature:asPlayer()
		if player then
			addInFightTicks(player)
		end

		local targetPlayer = targetCreature:asPlayer()
		if targetPlayer then
			addInFightTicks(targetPlayer)
		end
	end

	event:register()
end

do
	local event = Event()

	function event.onPlayerLogin(player)
		if player:getZone() ~= ZONE_PROTECTION then
			return true
		end

		if player:hasFlag(PlayerFlag_IgnoreProtectionZone) then
			return true
		end

		if not player:getTargetCreature() then
			return true
		end

		player:sendCancelTarget()
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
		player:setTargetCreature(nil)
		return true
	end

	event:register()
end
