local config = {
	{chanceFrom = 0, chanceTo = 1438, itemId = 3723, count = 20}, -- white mushroom
	{chanceFrom = 1439, chanceTo = 2454, itemId = 16103}, -- mushroom pie
	{chanceFrom = 2455, chanceTo = 3259, itemId = 16143, count = 15}, -- envenomed arrow
	{chanceFrom = 3260, chanceTo = 4057, itemId = 15793, count = 15}, -- crystalline arrow
	{chanceFrom = 4058, chanceTo = 4836, itemId = 16167}, -- teleport crystal
	{chanceFrom = 4837, chanceTo = 5443, itemId = 236, count = 2}, -- strong health potion
	{chanceFrom = 5444, chanceTo = 6024, itemId = 237, count = 2}, -- strong mana potion
	{chanceFrom = 6025, chanceTo = 6568, itemId = 266, count = 4}, -- health potion
	{chanceFrom = 6569, chanceTo = 7069, itemId = 268, count = 4}, -- mana potion
	{chanceFrom = 7070, chanceTo = 7518, itemId = 238}, -- great mana potion
	{chanceFrom = 7519, chanceTo = 7930, itemId = 239}, -- great health potion
	{chanceFrom = 7931, chanceTo = 8270, itemId = 7443}, -- bullseye potion
	{chanceFrom = 8271, chanceTo = 8587, itemId = 7439}, -- berserk potion
	{chanceFrom = 8588, chanceTo = 8900, itemId = 5911}, -- red piece of cloth
	{chanceFrom = 8901, chanceTo = 9204, itemId = 3035, count = 5}, -- platinum coin
	{chanceFrom = 9205, chanceTo = 9504, itemId = 7440}, -- mastermind potion
	{chanceFrom = 9505, chanceTo = 9629, itemId = 16165}, -- pet pig
	{chanceFrom = 9630, chanceTo = 9741, itemId = 16257}, -- gnomish voucher type CA2
	{chanceFrom = 9742, chanceTo = 9837, itemId = 16254}, -- gnomish voucher type MA2
	{chanceFrom = 9838, chanceTo = 9890, itemId = 3043}, -- crystal coin
	{chanceFrom = 9891, chanceTo = 9923, itemId = 3039}, -- red gem
	{chanceFrom = 9924, chanceTo = 9956, itemId = 3037}, -- yellow gem
	{chanceFrom = 9957, chanceTo = 9986, itemId = 16242}, -- red teleport crystal
	{chanceFrom = 9987, chanceTo = 9993, itemId = 3041}, -- blue gem
	{chanceFrom = 9994, chanceTo = 10000, itemId = 3038} -- green gem
}

local gnomish = Action()

function gnomish.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local chance = math.random(0, 10000)
	for i = 1, #config do
		local randomItem = config[i]
		if chance >= randomItem.chanceFrom and chance <= randomItem.chanceTo then
			if randomItem.itemId then
				local gift = randomItem.itemId
				local count = randomItem.count or 1
				if type(count) == "table" then
					count = math.random(count[1], count[2])
				end
				player:addItem(gift, count)
			end

			item:getPosition():sendMagicEffect(CONST_ME_CRAPS)
			item:remove(1)
			return true
		end
	end
	return false
end

gnomish:id(15698)
gnomish:register()
