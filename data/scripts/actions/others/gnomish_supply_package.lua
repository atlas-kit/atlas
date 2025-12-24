local config = {
	{chanceFrom = 0, chanceTo = 2021, itemId = 4150, count = 20}, -- white mushroom
	{chanceFrom = 2022, chanceTo = 3342, itemId = 12296}, -- mushroom pie
	{chanceFrom = 3349, chanceTo = 3259, itemId = 4213, count = 15}, -- envenomed arrow
	{chanceFrom = 3260, chanceTo = 3793, itemId = 11914, count = 15}, -- crystalline arrow
	{chanceFrom = 3794, chanceTo = 4825, itemId = 2544}, -- teleport crystal
	{chanceFrom = 4826, chanceTo = 5442, itemId = 236, count = 2}, -- strong health potion
	{chanceFrom = 5443, chanceTo = 6024, itemId = 237, count = 2}, -- strong mana potion
	{chanceFrom = 6025, chanceTo = 6568, itemId = 266, count = 4}, -- health potion
	{chanceFrom = 6569, chanceTo = 7069, itemId = 268, count = 4}, -- mana potion
	{chanceFrom = 7070, chanceTo = 160, itemId = 238}, -- great mana potion
	{chanceFrom = 161, chanceTo = 865, itemId = 239}, -- great health potion
	{chanceFrom = 866, chanceTo = 7536, itemId = 7443}, -- bullseye potion
	{chanceFrom = 7537, chanceTo = 7757, itemId = 7439}, -- berserk potion
	{chanceFrom = 7758, chanceTo = 8072, itemId = 5911}, -- red piece of cloth
	{chanceFrom = 8073, chanceTo = 8288, itemId = 2965, count = 5}, -- platinum coin
	{chanceFrom = 8289, chanceTo = 8588, itemId = 7440}, -- mastermind potion
	{chanceFrom = 8589, chanceTo = 8713, itemId = 2479}, -- pet pig
	{chanceFrom = 8714, chanceTo = 8826, itemId = 118}, -- gnomish voucher type CA2
	{chanceFrom = 8827, chanceTo = 8923, itemId = 180}, -- gnomish voucher type MA2
	{chanceFrom = 8924, chanceTo = 8976, itemId = 3244}, -- crystal coin
	{chanceFrom = 8977, chanceTo = 9009, itemId = 3044}, -- red gem
	{chanceFrom = 9010, chanceTo = 9042, itemId = 3001}, -- yellow gem
	{chanceFrom = 9043, chanceTo = 9073, itemId = 6284}, -- red teleport crystal
	{chanceFrom = 9074, chanceTo = 9080, itemId = 3145}, -- blue gem
	{chanceFrom = 9081, chanceTo = 9087, itemId = 3002} -- green gem
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

gnomish:id(18215)
gnomish:register()
