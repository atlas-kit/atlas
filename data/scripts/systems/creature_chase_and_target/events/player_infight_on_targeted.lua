-- player_infight_on_targeted.lua
-- Manages the in-fight condition (PZ-lock timer) for players.
-- In-fight is applied in three situations:
--   1. A player selects an attack target → onTargetCreatureChanged
--   2. A player takes non-healing damage or loses mana → onChangeHealth / onChangeMana
--   3. A player maintains an attack target over time → onCreatureThink (renewal)
-- Also clears targets for players who log in inside a Protection Zone.

local function addInFightTicks(player)
	if player:hasFlag(PlayerFlag_NotGainInFight) then
		return
	end

	local condition = Condition(CONDITION_INFIGHT, CONDITIONID_DEFAULT)
	condition:setTicks(configManager.getNumber(configKeys.PZ_LOCKED))
	player:addCondition(condition)
end

do
	local event = Event()

	-- When a player targets a creature (or is targeted), both sides
	-- gain the in-fight condition.
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

	-- Periodic renewal while the player has an active target.
	function event.onCreatureThink(creature, interval)
		local player = creature:asPlayer()
		if not player then
			return
		end

		if not player:getTargetCreature() then
			return
		end

		addInFightTicks(player)
	end

	event:register()
end

do
	local event = Event()

	-- Applies in-fight to a player who takes non-healing damage.
	function event.onCreatureChangeHealth(creature, attacker, primaryDamage, primaryType, secondaryDamage, secondaryType, origin)
		if primaryType == COMBAT_HEALING then
			return primaryDamage, primaryType, secondaryDamage, secondaryType
		end

		local player = creature:asPlayer()
		if player then
			addInFightTicks(player)
		end

		return primaryDamage, primaryType, secondaryDamage, secondaryType
	end

	event:register()
end

do
	local event = Event()

	-- Applies in-fight to a player who loses mana.
	function event.onCreatureChangeMana(creature, attacker, primaryDamage, primaryType, secondaryDamage, secondaryType, origin)
		local player = creature:asPlayer()
		if player then
			addInFightTicks(player)
		end

		return primaryDamage, primaryType, secondaryDamage, secondaryType
	end

	event:register()
end

do
	local event = Event()

	-- Clears the attack target when a player logs in while inside a
	-- Protection Zone with an active target.
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
