local christmasBundle = Action()

function christmasBundle.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local presents = { -- [christmas bundle item id] = {{reward item id, count}, ...}
		[6506] = { -- red bundle
			{6569, 15}, -- candy
			{3598, 20}, -- cookie
			{3599, 10}, -- candy cane
			{3586, 10}, -- orange
			{3585, 5}, -- red apple
			6501, -- gingerbreadman
			6502, -- christmas wreath
			6490, -- christmas branch
			6504, -- red christmas garland
			6388 -- christmas card
		},
		[6507] = { -- blue bundle
			{6569, 15}, -- candy
			{3598, 20}, -- cookie
			{3599, 10}, -- candy cane
			{3586, 10}, -- orange
			{3585, 5}, -- red apple
			6501, -- gingerbreadman
			6502, -- christmas wreath
			6490, -- christmas branch
			6505, -- blue christmas garland
			6388 -- christmas card
		},
		[6508] = { -- green bundle
			{6569, 15}, -- candy
			{3598, 20}, -- cookie
			{3599, 10}, -- candy cane
			{3586, 10}, -- orange
			{3585, 5}, -- red apple
			6501, -- gingerbreadman
			6502, -- christmas wreath
			6490, -- christmas branch
			6503, -- christmas garland
			6388 -- christmas card
		}
	}

	local targetItem = presents[item.itemid]
	if not targetItem then
		return true
	end

	local rewards = {}
	while #rewards < 7 do
		local count = 1
		local rand = math.random(#targetItem)
		local gift = targetItem[rand]
		if type(gift) == "table" then
			gift, count = unpack(gift)
		end
		rewards[#rewards + 1] = {gift, count}
		table.remove(targetItem, rand)
	end

	for i = 1, #rewards do
		player:addItem(unpack(rewards[i]))
	end
	item:remove(1)
	player:getPosition():sendMagicEffect(CONST_ME_GIFT_WRAPS)
	player:addAchievementProgress("Santa's Li'l Helper", 25)
	return true
end

christmasBundle:id(6506, 6507, 6508)
christmasBundle:register()
