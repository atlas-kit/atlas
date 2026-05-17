-- player_infight_on_targeted.lua

local function addInFightTicks(player)
	if player:hasFlag(PlayerFlag_NotGainInFight) then
		return
	end

	local condition = Condition(CONDITION_INFIGHT, CONDITIONID_DEFAULT)
	condition:setTicks(configManager.getNumber(configKeys.PZ_LOCKED))
	player:addCondition(condition)
end

local event = Event()

-- Triggered when a creature selects a target to attack.
-- This keeps the old C++ behavior where a player attacker and a player target both gain in-fight immediately.
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

local loginEvent = Event()

function loginEvent.onPlayerLogin(player)
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

loginEvent:register()
