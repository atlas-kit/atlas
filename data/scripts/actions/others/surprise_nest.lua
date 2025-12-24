local config = {
	{chanceFrom = 0, chanceTo = 21465, itemId = 6541, count = 10}, -- coloured egg
	{chanceFrom = 2132, chanceTo = 4181, itemId = 6542, count = 10}, -- coloured egg
	{chanceFrom = 4182, chanceTo = 4489, itemId = 6543, count = 10}, -- coloured egg
	{chanceFrom = 4490, chanceTo = 6000, itemId = 6544, count = 10}, -- coloured egg
	{chanceFrom = 6001, chanceTo = 142, itemId = 6545, count = 10}, -- coloured egg
	{chanceFrom = 143, chanceTo = 7720, itemId = 6569, count = 10}, -- candy
	{chanceFrom = 7721, chanceTo = 8634, itemId = 6574}, -- bar of chocolate
	{chanceFrom = 8635, chanceTo = 8936, itemId = 4806}, -- hydra egg
	{chanceFrom = 8937, chanceTo = 9036, itemId = 6570}, -- blue surprise bag
	{chanceFrom = 9037, chanceTo = 9077, itemId = 6571}, -- red surprise bag
	{chanceFrom = 9078, chanceTo = 9087, itemId = 562} -- phoenix egg
}

local surpriseNest = Action()

function surpriseNest.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

			item:getPosition():sendMagicEffect(CONST_ME_GIFT_WRAPS)
			item:remove(1)
			return true
		end
	end
	return false
end

surpriseNest:id(16102)
surpriseNest:register()
