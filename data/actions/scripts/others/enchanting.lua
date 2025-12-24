local items = {
	equipment = {
		[3947] = { -- small ruby
			[COMBAT_FIREDAMAGE] = {id = 577, targetId = 2944} -- helmet of the ancients (enchanted)
		},
		[3271] = { -- spike sword
			[COMBAT_FIREDAMAGE] = {id = 660}, [COMBAT_ICEDAMAGE] = {id = 679},
			[COMBAT_EARTHDAMAGE] = {id = 1157}, [COMBAT_ENERGYDAMAGE] = {id = 794}
		},
		[3279] = { -- war hammer
			[COMBAT_FIREDAMAGE] = {id = 674}, [COMBAT_ICEDAMAGE] = {id = 693},
			[COMBAT_EARTHDAMAGE] = {id = 793}, [COMBAT_ENERGYDAMAGE] = {id = 810}
		},
		[1067] = { -- clerical mace
			[COMBAT_FIREDAMAGE] = {id = 799}, [COMBAT_ICEDAMAGE] = {id = 689},
			[COMBAT_EARTHDAMAGE] = {id = 1167}, [COMBAT_ENERGYDAMAGE] = {id = 1184}
		},
		[1073] = { -- barbarian axe
			[COMBAT_FIREDAMAGE] = {id = 665}, [COMBAT_ICEDAMAGE] = {id = 684},
			[COMBAT_EARTHDAMAGE] = {id = 1162}, [COMBAT_ENERGYDAMAGE] = {id = 801}
		},
		[1074] = { -- knight axe
			[COMBAT_FIREDAMAGE] = {id = 666}, [COMBAT_ICEDAMAGE] = {id = 685},
			[COMBAT_EARTHDAMAGE] = {id = 1163}, [COMBAT_ENERGYDAMAGE] = {id = 802}
		},
		[1137] = { -- crystal mace
			[COMBAT_FIREDAMAGE] = {id = 800}, [COMBAT_ICEDAMAGE] = {id = 690},
			[COMBAT_EARTHDAMAGE] = {id = 1168}, [COMBAT_ENERGYDAMAGE] = {id = 807}
		},
		[1146] = { -- war axe
			[COMBAT_FIREDAMAGE] = {id = 669}, [COMBAT_ICEDAMAGE] = {id = 688},
			[COMBAT_EARTHDAMAGE] = {id = 1166}, [COMBAT_ENERGYDAMAGE] = {id = 805}
		},
		[7380] = { -- headchopper
			[COMBAT_FIREDAMAGE] = {id = 668}, [COMBAT_ICEDAMAGE] = {id = 687},
			[COMBAT_EARTHDAMAGE] = {id = 1165}, [COMBAT_ENERGYDAMAGE] = {id = 1182}
		},
		[7383] = { -- relic sword
			[COMBAT_FIREDAMAGE] = {id = 661}, [COMBAT_ICEDAMAGE] = {id = 680},
			[COMBAT_EARTHDAMAGE] = {id = 1158}, [COMBAT_ENERGYDAMAGE] = {id = 795}
		},
		[7384] = { -- mystic blade
			[COMBAT_FIREDAMAGE] = {id = 662}, [COMBAT_ICEDAMAGE] = {id = 681},
			[COMBAT_EARTHDAMAGE] = {id = 1159}, [COMBAT_ENERGYDAMAGE] = {id = 796}
		},
		[7389] = { -- heroic axe
			[COMBAT_FIREDAMAGE] = {id = 667}, [COMBAT_ICEDAMAGE] = {id = 686},
			[COMBAT_EARTHDAMAGE] = {id = 1164}, [COMBAT_ENERGYDAMAGE] = {id = 803}
		},
		[7392] = { -- orcish maul
			[COMBAT_FIREDAMAGE] = {id = 673}, [COMBAT_ICEDAMAGE] = {id = 692},
			[COMBAT_EARTHDAMAGE] = {id = 792}, [COMBAT_ENERGYDAMAGE] = {id = 809}
		},
		[7402] = { -- dragon slayer
			[COMBAT_FIREDAMAGE] = {id = 664}, [COMBAT_ICEDAMAGE] = {id = 683},
			[COMBAT_EARTHDAMAGE] = {id = 1161}, [COMBAT_ENERGYDAMAGE] = {id = 798}
		},
		[7406] = { -- blacksteel sword
			[COMBAT_FIREDAMAGE] = {id = 663}, [COMBAT_ICEDAMAGE] = {id = 682},
			[COMBAT_EARTHDAMAGE] = {id = 1160}, [COMBAT_ENERGYDAMAGE] = {id = 797}
		},
		[7415] = { -- cranial basher
			[COMBAT_FIREDAMAGE] = {id = 672}, [COMBAT_ICEDAMAGE] = {id = 691},
			[COMBAT_EARTHDAMAGE] = {id = 1169}, [COMBAT_ENERGYDAMAGE] = {id = 808}
		},
		[2508] = { -- rainbow shield
			[COMBAT_FIREDAMAGE] = {id = 3410}, [COMBAT_ICEDAMAGE] = {id = 3446},
			[COMBAT_EARTHDAMAGE] = {id = 3448}, [COMBAT_ENERGYDAMAGE] = {id = 3447}
		},
		[5719] = { -- dracoyle statue
			[COMBAT_EARTHDAMAGE] = {id = 5718} -- dracoyle statue (enchanted)
		},
		[5724] = { -- dracoyle statue
			[COMBAT_EARTHDAMAGE] = {id = 5723} -- dracoyle statue (enchanted)
		},
		[8192] = { -- worn firewalker boots
			[COMBAT_FIREDAMAGE] = {id = 6286, say = {text = "Take the boots off first."}},
			slot = {type = CONST_SLOT_FEET, check = true}
		},
		[19726] = { -- werewolf amulet
			[COMBAT_NONE] = {
				id = 17447,
				effects = {failure = CONST_ME_POFF, success = CONST_ME_THUNDER},
				message = {text = "The amulet cannot be enchanted while worn."}
			},
			slot = {type = CONST_SLOT_NECKLACE, check = true}
		},
		[19728] = { -- werewolf helmet
			[COMBAT_NONE] = {
				id = {
					[SKILL_CLUB] = {id = 17513},
					[SKILL_SWORD] = {id = 17513},
					[SKILL_AXE] = {id = 17513},
					[SKILL_DISTANCE] = {id = 17513},
					[SKILL_MAGLEVEL] = {id = 17513}
				},
				effects = {failure = CONST_ME_POFF, success = CONST_ME_THUNDER},
				message = {text = "The helmet cannot be enchanted while worn."},
				usesStorage = true
			},
			slot = {type = CONST_SLOT_HEAD, check = true}
		},
		charges = 1245, effect = CONST_ME_MAGIC_RED
	},

	valuables = {
		[4210] = {id = 675, shrine = {150, 7509, 7510, 7511}}, -- small sapphire
		[4211] = {id = 676, shrine = {146, 7505, 7506, 7507}}, -- small ruby
		[4213] = {id = 677, shrine = {158, 7517, 7518, 7519}}, -- small emerald
		[4214] = {id = 678, shrine = {154, 7513, 7514, 7515}}, -- small amethyst
		soul = 2, mana = 300, effect = CONST_ME_HOLYDAMAGE
	},

	[576] = {combatType = COMBAT_FIREDAMAGE, targetId = 3947}, -- helmet of the ancients
	[675] = {combatType = COMBAT_ICEDAMAGE}, -- small enchanted sapphire
	[676] = {combatType = COMBAT_FIREDAMAGE}, -- small enchanted ruby
	[677] = {combatType = COMBAT_EARTHDAMAGE}, -- small enchanted emerald
	[678] = {combatType = COMBAT_ENERGYDAMAGE}, -- small enchanted amethyst
	[19749] = {combatType = COMBAT_NONE} -- moonlight crystals
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or not target:isItem() then
		return false
	end

	local itemId, targetId = item:getId(), target:getId()
	local targetType = items.valuables[itemId] or items.equipment[items[itemId].targetId or targetId]
	if not targetType then
		return false
	end

	if targetType.shrine then
		if not table.contains(targetType.shrine, targetId) then
			player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
			return true
		end

		if player:getMana() < items.valuables.mana then
			player:sendCancelMessage(RETURNVALUE_NOTENOUGHMANA)
			return true
		end

		if player:getSoul() < items.valuables.soul then
			player:sendCancelMessage(RETURNVALUE_NOTENOUGHSOUL)
			return true
		end
		player:addSoul(-items.valuables.soul)
		player:addMana(-items.valuables.mana)
		player:addManaSpent(items.valuables.mana)
		player:addItem(targetType.id)
		player:getPosition():sendMagicEffect(items.valuables.effect)
		player:sendSupplyUsed(item)
		item:remove(1)
	else
		local targetItem = targetType[items[itemId].combatType]
		if not targetItem or targetItem.targetId and targetItem.targetId ~= targetId then
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
				if targetItem.usesStorage then
					local vocationId = player:getVocation():getDemotion():getId()
					local storage = storages[itemId] and storages[itemId][targetId] and storages[itemId][targetId][vocationId]
					if not storage then
						return false
					end

					local storageValue = player:getStorageValue(storage.key)
					if not storageValue then
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
					target:setAttribute(ITEM_ATTRIBUTE_CHARGES, items.equipment.charges)
				end
				player:sendSupplyUsed(item)
				item:remove(1)
			end
		end
		player:getPosition():sendMagicEffect(targetItem.effects and (isInSlot and targetItem.effects.failure or targetItem.effects.success) or items.equipment.effect)
	end
	return true
end
