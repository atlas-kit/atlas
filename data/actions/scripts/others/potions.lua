local berserk = Condition(CONDITION_ATTRIBUTES)
berserk:setParameter(CONDITION_PARAM_TICKS, 10 * 60 * 1000)
berserk:setParameter(CONDITION_PARAM_SKILL_MELEE, 5)
berserk:setParameter(CONDITION_PARAM_SKILL_SHIELD, -10)
berserk:setParameter(CONDITION_PARAM_BUFF_SPELL, true)

local mastermind = Condition(CONDITION_ATTRIBUTES)
mastermind:setParameter(CONDITION_PARAM_TICKS, 10 * 60 * 1000)
mastermind:setParameter(CONDITION_PARAM_STAT_MAGICPOINTS, 3)
mastermind:setParameter(CONDITION_PARAM_BUFF_SPELL, true)

local bullseye = Condition(CONDITION_ATTRIBUTES)
bullseye:setParameter(CONDITION_PARAM_TICKS, 10 * 60 * 1000)
bullseye:setParameter(CONDITION_PARAM_SKILL_DISTANCE, 5)
bullseye:setParameter(CONDITION_PARAM_SKILL_SHIELD, -10)
bullseye:setParameter(CONDITION_PARAM_BUFF_SPELL, true)

local manaShield = Condition(CONDITION_MANASHIELD_BREAKABLE)
manaShield:setParameter(CONDITION_PARAM_TICKS, 3 * 60 * 1000)

local function magicShieldCapacity(player)
	manaShield:setParameter(CONDITION_PARAM_MANASHIELD_BREAKABLE, math.min(player:getMaxMana(), 300 + 7.6 * player:getLevel() + 7 * player:getMagicLevel()))
end

local potions = {
	[6558] = { -- flask of demonic blood
		transform = {236, 7589},
		effect = CONST_ME_DRAWBLOOD
	},
	[7439] = { -- berserk potion
		condition = berserk,
		vocations = {
			VOCATION_KNIGHT,
			VOCATION_ELITE_KNIGHT
		},
		effect = CONST_ME_MAGIC_RED,
		description = "Only knights may drink this potion.",
		text = "You feel stronger."
	},
	[7440] = { -- mastermind potion
		condition = mastermind,
		vocations = {
			VOCATION_SORCERER,
			VOCATION_DRUID,
			VOCATION_MASTER_SORCERER,
			VOCATION_ELDER_DRUID
		},
		effect = CONST_ME_MAGIC_BLUE,
		description = "Only sorcerers and druids may drink this potion.",
		text = "You feel smarter."
	},
	[7443] = { -- bullseye potion
		condition = bullseye,
		vocations = {
			VOCATION_PALADIN,
			VOCATION_ROYAL_PALADIN
		},
		effect = CONST_ME_MAGIC_GREEN,
		description = "Only paladins may drink this potion.",
		text = "You feel more accurate."
	},
	[32907] = { -- magic shield potion
		condition = manaShield,
		vocations = {
			VOCATION_SORCERER,
			VOCATION_DRUID,
			VOCATION_MASTER_SORCERER,
			VOCATION_ELDER_DRUID
		},
		level = 14,
		effect = CONST_ME_ENERGYAREA,
		description = "Only sorcerers and druids of level 14 or above may drink this potion.",
		capacity = magicShieldCapacity
	},
	[236] = { -- strong health potion
		health = {250, 350},
		vocations = {
			VOCATION_PALADIN,
			VOCATION_KNIGHT,
			VOCATION_ROYAL_PALADIN,
			VOCATION_ELITE_KNIGHT
		},
		level = 50,
		flask = 283,
		description = "Only knights and paladins of level 50 or above may drink this fluid."
	},
	[237] = { -- strong mana potion
		mana = {115, 185},
		level = 50,
		flask = 283,
		description = "Only players of level 50 or above may drink this fluid."
	},
	[238] = { -- great mana potion
		mana = {150, 250},
		vocations = {
			VOCATION_SORCERER,
			VOCATION_DRUID,
			VOCATION_PALADIN,
			VOCATION_MASTER_SORCERER,
			VOCATION_ELDER_DRUID,
			VOCATION_ROYAL_PALADIN
		},
		level = 80,
		flask = 284,
		description = "Only sorcerers, druids and paladins of level 80 or above may drink this fluid."
	},
	[239] = { -- great health potion
		health = {430, 575},
		vocations = {
			VOCATION_KNIGHT,
			VOCATION_ELITE_KNIGHT
		},
		level = 80,
		flask = 284,
		description = "Only knights of level 80 or above may drink this fluid."
	},
	[266] = { -- health potion
		health = {125, 175},
		flask = 285
	},
	[268] = { -- mana potion
		mana = {75, 125},
		flask = 285
	},
	[291] = { -- great spirit potion
		health = {250, 350},
		mana = {100, 200},
		vocations = {
			VOCATION_PALADIN,
			VOCATION_ROYAL_PALADIN
		},
		level = 80,
		flask = 284,
		description = "Only paladins of level 80 or above may drink this fluid."
	},
	[292] = { -- ultimate health potion
		health = {650, 850},
		vocations = {
			VOCATION_KNIGHT,
			VOCATION_ELITE_KNIGHT
		},
		level = 130,
		flask = 284,
		description = "Only knights of level 130 or above may drink this fluid."
	},
	[295] = { -- antidote potion
		antidote = true,
		flask = 285
	},
	[803] = { -- small health potion
		health = {60, 90},
		flask = 285
	},
	[21002] = { -- ultimate mana potion
		mana = {430, 575},
		vocations = {
			VOCATION_SORCERER,
			VOCATION_DRUID,
			VOCATION_MASTER_SORCERER,
			VOCATION_ELDER_DRUID
		},
		level = 130,
		flask = 284,
		description = "Only druids and sorcerers of level 130 or above may drink this fluid."
	},
	[21003] = { -- ultimate spirit potion
		health = {413, 580},
		mana = {150, 250},
		vocations = {
			VOCATION_PALADIN,
			VOCATION_ROYAL_PALADIN
		},
		level = 130,
		flask = 284,
		description = "Only paladins of level 130 or above may drink this fluid."
	},
	[21004] = { -- supreme health potion
		health = {1083, 1125},
		vocations = {
			VOCATION_KNIGHT,
			VOCATION_ELITE_KNIGHT
		},
		level = 200,
		flask = 284,
		description = "Only knights of level 200 or above may drink this fluid."
	}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if type(target) == "userdata" and not target:isPlayer() then
		return false
	end

	local potion = potions[item:getId()]
	if not player:getGroup():getAccess() and (potion.level and player:getLevel() < potion.level or potion.vocations and not table.contains(potion.vocations, player:getVocation():getId())) then
		player:say(potion.description, TALKTYPE_POTION)
		return true
	end

	if potion.condition then
		if potion.capacity then
			potion.capacity(player)
		end
		player:addCondition(potion.condition)
		player:say(potion.text, TALKTYPE_POTION)
		player:getPosition():sendMagicEffect(potion.effect)
	elseif potion.transform then
		local reward = potion.transform[math.random(#potion.transform)]
		if fromPosition.x == CONTAINER_POSITION then
			local targetContainer = Container(item:getParent().uid)
			targetContainer:addItem(reward, 1)
		else
			Game.createItem(reward, 1, fromPosition)
		end

		item:getPosition():sendMagicEffect(potion.effect)
		item:remove(1)
		return true
	else
		if potion.health then
			doTargetCombat(player, target, COMBAT_HEALING, potion.health[1], potion.health[2])
		end

		if potion.mana then
			doTargetCombat(player, target, COMBAT_MANADRAIN, potion.mana[1], potion.mana[2])
		end

		if potion.antidote then
			target:removeCondition(CONDITION_POISON)
		end

		player:addAchievementProgress("Potion Addict", 100000)
		player:addItem(potion.flask)
		target:say("Aaaah...", TALKTYPE_POTION)
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
	end

	if not configManager.getBoolean(configKeys.REMOVE_POTION_CHARGES) then
		return true
	end

	item:remove(1)
	return true
end
