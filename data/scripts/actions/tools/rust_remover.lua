local config = {
	[8894] = { -- Rusty Armor (Common)
		[1] = {id = 1207, chance = 6994}, -- Chain Armor
		[2] = {id = 1466, chance = 2965}, -- Scale Armor
		[3] = {id = 1208, chance = 2133}, -- Brass Armor
		[4] = {id = 1206, chance = 197} -- Plate Armor
	},
	[8895] = { -- Rusty Armor (Semi-rare)
		[1] = {id = 1466, chance = 6436}, -- Scale Armor
		[2] = {id = 1207, chance = 4595}, -- Chain Armor
		[3] = {id = 1208, chance = 4210}, -- Brass Armor
		[4] = {id = 1206, chance = 2198}, -- Plate Armor
		[5] = {id = 1459, chance = 595}, -- Knight Armor
		[6] = {id = 2139, chance = 283}, -- Paladin Armor
		[7] = {id = 1470, chance = 49} -- Crown Armor
	},
	[8896] = { -- Rusty Armor (Rare)
		[1] = {id = 1208, chance = 6681}, -- Brass Armor
		[2] = {id = 1206, chance = 2264}, -- Plate Armor
		[3] = {id = 1459, chance = 2619}, -- Knight Armor
		[4] = {id = 1470, chance = 177}, -- Crown Armor
		[5] = {id = 2139, chance = 31}, -- Paladin Armor
		[6] = {id = 1209, chance = 10} -- Golden Armor
	},
	[8897] = { -- Rusty Legs (Common)
		[1] = {id = 3558, chance = 6949}, -- Chain Legs
		[2] = {id = 1451, chance = 2007}, -- Studded Legs
		[3] = {id = 1461, chance = 1794}, -- Brass Legs
		[4] = {id = 3557, chance = 133} -- Plate Legs
	},
	[8898] = { -- Rusty Legs (Semi-Rare)
		[1] = {id = 1451, chance = 5962}, -- Studded Legs
		[2] = {id = 3558, chance = 3773}, -- Chain Legs
		[3] = {id = 1461, chance = 3058}, -- Brass Legs
		[4] = {id = 3557, chance = 1661}, -- Plate Legs
		[5] = {id = 1460, chance = 186}, -- Knight Legs
	},
	[8899] = { -- Rusty Legs (Rare)
		[1] = {id = 1461, chance = 6499}, -- Brass Legs
		[2] = {id = 3557, chance = 2297}, -- Plate Legs
		[3] = {id = 1460, chance = 200}, -- Knight Legs
		[4] = {id = 1471, chance = 52}, -- Crown Legs
		[5] = {id = 1453, chance = 30} -- Golden Legs
	},
	[8900] = { -- Heavily Rusted Shield
	},
	[8901] = { -- Rusted Shield
	},
	[8902] = { -- Slightly Rusted Shield
		[1] = {id = 1499, chance = 484}, -- Plate Shield
		[2] = {id = 1521, chance = 4068}, -- Ancient Shield
		[3] = {id = 7460, chance = 1161}, -- Norse Shield
		[4] = {id = 1508, chance = 23}, -- Crown Shield
		[5] = {id = 1523, chance = 10} -- Vampire Shield
	},
	[8906] = { -- Heavily Rusted Helmet
	},
	[8907] = { -- Rusted Helmet
		[1] = {id = 1203, chance = 3084}, -- Brass Helmet
		[2] = {id = 1465, chance = 2657}, -- Studded Helmet
		[3] = {id = 1202, chance = 105}, -- Iron Helmet
		[4] = {id = 1200, chance = 1255}, -- Steel Helmet
		[5] = {id = 1474, chance = 190}, -- Crown Helmet
		[6] = {id = 1480, chance = 10} -- Crusader Helmet
	},
	[8908] = { -- Slightly Rusted Helmet
		[1] = {id = 1202, chance = 503}, -- Iron Helmet
		[2] = {id = 1200, chance = 4157}, -- Steel Helmet
		[3] = {id = 1474, chance = 1195}, -- Crown Helmet
		[4] = {id = 1480, chance = 210}, -- Crusader Helmet
		[5] = {id = 1481, chance = 7} -- Royal Helmet
	},
	[8903] = { -- Heavily Rusted Boots
	},
	[8904] = { -- Rusted Boots
	},
	[8905] = { -- Slightly Rusted Boots
	},
}

local rustRemover = Action()

function rustRemover.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local targetItem = config[target.itemid]
	if not targetItem then
		return true
	end

	local randomChance = math.random(10000)
	local index = false

	if targetItem[1].chance >= randomChance then -- implying first item in the table index always has the highest chance.
		while not index do
			randomIndex = math.random(#targetItem)
			if targetItem[randomIndex].chance >= randomChance then
				index = randomIndex
			end
		end
	end

	if not index then
		if table.contains({9808, 9809, 9810}, target.itemid) then
			msg = "The armor was already damaged so badly that it broke when you tried to clean it."
		end
		if table.contains({9811, 9812, 9813}, target.itemid) then
			msg = "The legs were already damaged so badly that they broke when you tried to clean them."
		end
		if table.contains({9814, 9815, 9816}, target.itemid) then
			msg = "The shield was already damaged so badly that it broke when you tried to clean it."
		end
		if table.contains({9817, 9818, 9819}, target.itemid) then
			msg = "The boots were already damaged so badly that they broke when you tried to clean them."
		end
		if table.contains({9820, 9821, 9822}, target.itemid) then
			msg = "The helmet was already damaged so badly that it broke when you tried to clean it."
		end
		player:say(msg, TALKTYPE_MONSTER_SAY)
		target:getPosition():sendMagicEffect(CONST_ME_BLOCKHIT)
		target:remove()
	else
		target:transform(targetItem[index].id)
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_GREEN)
		player:addAchievementProgress("Polisher", 1000)
	end
	player:sendSupplyUsed(item)
	return item:remove(1)
end

rustRemover:id(9930)
rustRemover:register()
