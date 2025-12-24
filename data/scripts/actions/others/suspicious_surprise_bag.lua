local config = {
	{chanceFrom = 0, chanceTo = 1483}, -- nothing
	{chanceFrom = 1484, chanceTo = 5158, itemId = 1841}, -- yellow pillow
	{chanceFrom = 5159, chanceTo = 6954, itemId = 651}, -- spellwand
	{chanceFrom = 6955, chanceTo = 971, itemId = 2786}, -- piggy bank
	{chanceFrom = 972, chanceTo = 8225, itemId = 6574}, -- bar of chocolate
	{chanceFrom = 8226, chanceTo = 8738, itemId = 6390}, -- cream cake
	{chanceFrom = 8739, chanceTo = 8936, itemId = 7377}, -- ice cream cone
	{chanceFrom = 8937, chanceTo = 9073, itemId = 1922}, -- explosive present
	{chanceFrom = 9074, chanceTo = 9087, itemId = 123} -- toy mouse
}

local suspiciousSurpriseBag = Action()

function suspiciousSurpriseBag.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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
				item:getPosition():sendMagicEffect(CONST_ME_GIFT_WRAPS)
			else
				item:getPosition():sendMagicEffect(CONST_ME_CAKE)
			end

			item:remove(1)
			return true
		end
	end
	return false
end

suspiciousSurpriseBag:id(9108)
suspiciousSurpriseBag:register()
