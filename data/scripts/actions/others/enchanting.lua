local action = Action()

local equipment = {
	[3030] = { -- small ruby
		[COMBAT_FIREDAMAGE] = {id = 2343, targetId = 2147} -- helmet of the ancients (enchanted)
	},
	[3271] = { -- spike sword
		[COMBAT_FIREDAMAGE] = {id = 7744}, [COMBAT_ICEDAMAGE] = {id = 7763},
		[COMBAT_EARTHDAMAGE] = {id = 7854}, [COMBAT_ENERGYDAMAGE] = {id = 7869}
	},
	[3279] = { -- war hammer
		[COMBAT_FIREDAMAGE] = {id = 7758}, [COMBAT_ICEDAMAGE] = {id = 7777},
		[COMBAT_EARTHDAMAGE] = {id = 7868}, [COMBAT_ENERGYDAMAGE] = {id = 7883}
	},
	[3311] = { -- clerical mace
		[COMBAT_FIREDAMAGE] = {id = 7754}, [COMBAT_ICEDAMAGE] = {id = 7773},
		[COMBAT_EARTHDAMAGE] = {id = 7864}, [COMBAT_ENERGYDAMAGE] = {id = 7879}
	},
	[3317] = { -- barbarian axe
		[COMBAT_FIREDAMAGE] = {id = 7749}, [COMBAT_ICEDAMAGE] = {id = 7768},
		[COMBAT_EARTHDAMAGE] = {id = 7859}, [COMBAT_ENERGYDAMAGE] = {id = 7874}
	},
	[3318] = { -- knight axe
		[COMBAT_FIREDAMAGE] = {id = 7750}, [COMBAT_ICEDAMAGE] = {id = 7769},
		[COMBAT_EARTHDAMAGE] = {id = 7860}, [COMBAT_ENERGYDAMAGE] = {id = 7875}
	},
	[3333] = { -- crystal mace
		[COMBAT_FIREDAMAGE] = {id = 7755}, [COMBAT_ICEDAMAGE] = {id = 7774},
		[COMBAT_EARTHDAMAGE] = {id = 7865}, [COMBAT_ENERGYDAMAGE] = {id = 7880}
	},
	[3342] = { -- war axe
		[COMBAT_FIREDAMAGE] = {id = 7753}, [COMBAT_ICEDAMAGE] = {id = 7772},
		[COMBAT_EARTHDAMAGE] = {id = 7863}, [COMBAT_ENERGYDAMAGE] = {id = 7878}
	},
	[7380] = { -- headchopper
		[COMBAT_FIREDAMAGE] = {id = 7752}, [COMBAT_ICEDAMAGE] = {id = 7771},
		[COMBAT_EARTHDAMAGE] = {id = 7862}, [COMBAT_ENERGYDAMAGE] = {id = 7877}
	},
	[7383] = { -- relic sword
		[COMBAT_FIREDAMAGE] = {id = 7745}, [COMBAT_ICEDAMAGE] = {id = 7764},
		[COMBAT_EARTHDAMAGE] = {id = 7855}, [COMBAT_ENERGYDAMAGE] = {id = 7870}
	},
	[7384] = { -- mystic blade
		[COMBAT_FIREDAMAGE] = {id = 7746}, [COMBAT_ICEDAMAGE] = {id = 7765},
		[COMBAT_EARTHDAMAGE] = {id = 7856}, [COMBAT_ENERGYDAMAGE] = {id = 7871}
	},
	[7389] = { -- heroic axe
		[COMBAT_FIREDAMAGE] = {id = 7751}, [COMBAT_ICEDAMAGE] = {id = 7770},
		[COMBAT_EARTHDAMAGE] = {id = 7861}, [COMBAT_ENERGYDAMAGE] = {id = 7876}
	},
	[7392] = { -- orcish maul
		[COMBAT_FIREDAMAGE] = {id = 7757}, [COMBAT_ICEDAMAGE] = {id = 7776},
		[COMBAT_EARTHDAMAGE] = {id = 7867}, [COMBAT_ENERGYDAMAGE] = {id = 7882}
	},
	[7402] = { -- dragon slayer
		[COMBAT_FIREDAMAGE] = {id = 7748}, [COMBAT_ICEDAMAGE] = {id = 7767},
		[COMBAT_EARTHDAMAGE] = {id = 7858}, [COMBAT_ENERGYDAMAGE] = {id = 7873}
	},
	[7406] = { -- blacksteel sword
		[COMBAT_FIREDAMAGE] = {id = 7747}, [COMBAT_ICEDAMAGE] = {id = 7766},
		[COMBAT_EARTHDAMAGE] = {id = 7857}, [COMBAT_ENERGYDAMAGE] = {id = 7872}
	},
	[7415] = { -- cranial basher
		[COMBAT_FIREDAMAGE] = {id = 7756}, [COMBAT_ICEDAMAGE] = {id = 7775},
		[COMBAT_EARTHDAMAGE] = {id = 7866}, [COMBAT_ENERGYDAMAGE] = {id = 7881}
	},
	[8077] = { -- rainbow shield
		[COMBAT_FIREDAMAGE] = {id = 8906}, [COMBAT_ICEDAMAGE] = {id = 8907},
		[COMBAT_EARTHDAMAGE] = {id = 8909}, [COMBAT_ENERGYDAMAGE] = {id = 8908}
	},
	[9035] = { -- dracoyle statue
		[COMBAT_EARTHDAMAGE] = {id = 9948} -- dracoyle statue (enchanted)
	},
	[9040] = { -- dracoyle statue
		[COMBAT_EARTHDAMAGE] = {id = 9953} -- dracoyle statue (enchanted)
	},
	[9020] = { -- worn firewalker boots
		[COMBAT_FIREDAMAGE] = {id = 9933, say = {text = "Take the boots off first."}},
		slot = {type = CONST_SLOT_FEET, check = true}
	},
	[22060] = { -- werewolf amulet
		[COMBAT_NONE] = {
			id = 22061,
			effects = {failure = CONST_ME_POFF, success = CONST_ME_THUNDER},
			message = {text = "The amulet cannot be enchanted while worn."}
		},
		slot = {type = CONST_SLOT_NECKLACE, check = true}
	},
	[22062] = { -- werewolf helmet
		[COMBAT_NONE] = {
			id = {
				[SKILL_CLUB] = {id = 22127},
				[SKILL_SWORD] = {id = 22127},
				[SKILL_AXE] = {id = 22127},
				[SKILL_DISTANCE] = {id = 22127},
				[SKILL_MAGLEVEL] = {id = 24783}
			},
			effects = {failure = CONST_ME_POFF, success = CONST_ME_THUNDER},
			message = {text = "The helmet cannot be enchanted while worn."},
			storage = PlayerStorageKeys.werewolfHelmet
		},
		slot = {type = CONST_SLOT_HEAD, check = true}
	},
}

local valuables = {
	[3029] = {id = 675, shrine = {150, 151, 152, 153}}, -- small sapphire
	[3030] = {id = 676, shrine = {146, 147, 148, 149}}, -- small ruby
	[3032] = {id = 677, shrine = {158, 159, 160, 161}}, -- small emerald
	[3033] = {id = 678, shrine = {154, 155, 156, 157}}, -- small amethyst
}

local items = {
	[2342] = {combatType = COMBAT_FIREDAMAGE, targetId = 2147}, -- helmet of the ancients
	[7759] = {combatType = COMBAT_ICEDAMAGE}, -- small enchanted sapphire
	[7760] = {combatType = COMBAT_FIREDAMAGE}, -- small enchanted ruby
	[7761] = {combatType = COMBAT_EARTHDAMAGE}, -- small enchanted emerald
	[7762] = {combatType = COMBAT_ENERGYDAMAGE}, -- small enchanted amethyst
	[24739] = {combatType = COMBAT_NONE} -- moonlight crystals
}

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or not target:isItem() then
		return false
	end

	local itemId, targetId = item:getId(), target:getId()
	local targetType = valuables[itemId] or equipment[items[itemId].targetId or targetId]
	if not targetType then
		return false
	end

	if targetType.shrine then
		if not table.contains(targetType.shrine, targetId) then
			player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
			return true
		end

		if player:getMana() < 300 then
			player:sendCancelMessage(RETURNVALUE_NOTENOUGHMANA)
			return true
		end

		if player:getSoul() < 2 then
			player:sendCancelMessage(RETURNVALUE_NOTENOUGHSOUL)
			return true
		end
		player:addSoul(-2)
		player:addMana(-300)
		player:addManaSpent(300)
		player:addItem(targetType.id)
		player:getPosition():sendMagicEffect(CONST_ME_HOLYDAMAGE)
		player:sendSupplyUsed(item)
		item:remove(1)
	else
		local targetItem = targetType[items[itemId].combatType]
		if not targetItem or (targetItem.targetId and targetItem.targetId ~= targetId) then
			return false
		end

		local isInSlot = targetType.slot and targetType.slot.check and target:getType():usesSlot(targetType.slot.type) and Player(target:getParent())
		if isInSlot then
			if targetItem.say then
				player:say(targetItem.say.text, TALKTYPE_MONSTER_SAY)
				return true
			elseif targetItem.message then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, targetItem.message.text)
			else
				return false
			end
		else
			if targetItem.targetId then
				item:transform(targetItem.id)
				item:decay()
				player:sendSupplyUsed(target)
				target:remove(1)
			else
				if targetItem.storage then
					local storageValue = player:getStorageValue(targetItem.storage)
					if storageValue == nil then
						return false
					end

					local transform = targetItem.id and targetItem.id[storageValue]
					if not transform then
						return false
					end
					target:transform(transform.id)
				else
					target:transform(targetItem.id)
				end

				if target:hasAttribute(ITEM_ATTRIBUTE_DURATION) then
					target:decay()
				end

				if target:hasAttribute(ITEM_ATTRIBUTE_CHARGES) then
					target:setAttribute(ITEM_ATTRIBUTE_CHARGES, 1245)
				end
				player:sendSupplyUsed(item)
				item:remove(1)
			end
		end
		player:getPosition():sendMagicEffect(targetItem.effects and (isInSlot and targetItem.effects.failure or targetItem.effects.success) or CONST_ME_MAGIC_RED)
	end
	return true
end

for k, _ in pairs(valuables) do
	action:id(k)
end
for k, _ in pairs(items) do
	action:id(k)
end
action:register()
