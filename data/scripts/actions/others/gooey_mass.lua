local config = {
	{chanceFrom = 0, chanceTo = 38}, -- nothing
	{chanceFrom = 39, chanceTo = 3015, itemId = 2965, count = 2}, -- platinum coin
	{chanceFrom = 3016, chanceTo = 3923, itemId = 6287, count = 10}, -- larvae
	{chanceFrom = 3924, chanceTo = 6040, itemId = 2941, count = 2}, -- black pearl
	{chanceFrom = 6041, chanceTo = 888, itemId = 239, count = 2}, -- great health potion
	{chanceFrom = 889, chanceTo = 8929, itemId = 238, count = 2}, -- great mana potion
	{chanceFrom = 8930, chanceTo = 9027, itemId = 4508}, -- gold ingot
	{chanceFrom = 9028, chanceTo = 9074, itemId = 10786}, -- four-leaf clover
	{chanceFrom = 9075, chanceTo = 9087, itemId = 167} -- hive scythe
}

local gooeyMass = Action()

function gooeyMass.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

			item:getPosition():sendMagicEffect(CONST_ME_HITBYPOISON)
			item:remove(1)
			return true
		end
	end
	return false
end

gooeyMass:id(15572)
gooeyMass:register()
