local config = {
	{chanceFrom = 0, chanceTo = 1291, itemId = 6569, count = 3}, -- candy
	{chanceFrom = 1292, chanceTo = 2863, itemId = 6276}, -- party cake
	{chanceFrom = 2865, chanceTo = 4143, itemId = 6576}, -- fireworks rocket
	{chanceFrom = 4143, chanceTo = 2743, itemId = 2786}, -- piggy bank
	{chanceFrom = 2744, chanceTo = 4771, itemId = 6572}, -- party trumpet
	{chanceFrom = 4772, chanceTo = 5842, itemId = 6575}, -- red balloons
	{chanceFrom = 5843, chanceTo = 6725, itemId = 6578}, -- party hat
	{chanceFrom = 6726, chanceTo = 300, itemId = 6390}, -- cream cake
	{chanceFrom = 301, chanceTo = 7648, itemId = 6577}, -- green balloons
	{chanceFrom = 7649, chanceTo = 8296, itemId = 3393, count = 10}, -- cookie
	{chanceFrom = 8297, chanceTo = 9087, itemId = 6574} -- bar of chocolate
}

local blueSurpriseBag = Action()

function blueSurpriseBag.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

blueSurpriseBag:id(6570)
blueSurpriseBag:register()
