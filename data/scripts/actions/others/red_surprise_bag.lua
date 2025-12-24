local config = {
	{chanceFrom = 0, chanceTo = 4749, itemId = 6576}, -- fireworks rocket
	{chanceFrom = 4750, chanceTo = 6841, itemId = 6390}, -- cream cake
	{chanceFrom = 6842, chanceTo = 1010, itemId = 6574}, -- bar of chocolate
	{chanceFrom = 1011, chanceTo = 8179, itemId = 6578}, -- party hat
	{chanceFrom = 8180, chanceTo = 8603, itemId = 2786}, -- piggy bank
	{chanceFrom = 8604, chanceTo = 3094, itemId = 2966}, -- violet gem
	{chanceFrom = 8778, chanceTo = 8936, itemId = 5944}, -- soul orb
	{chanceFrom = 8937, chanceTo = 8971, itemId = 3044}, -- red gem
	{chanceFrom = 8972, chanceTo = 8993, itemId = 2549}, -- teddy bear
	{chanceFrom = 8994, chanceTo = 9011, itemId = 3027}, -- demon shield
	{chanceFrom = 9012, chanceTo = 9030, itemId = 6568}, -- panda teddy
	{chanceFrom = 9031, chanceTo = 9046, itemId = 4391}, -- boots of haste
	{chanceFrom = 9047, chanceTo = 9061, itemId = 2928}, -- dragon scale mail
	{chanceFrom = 9062, chanceTo = 9073, itemId = 2991}, -- royal helmet
	{chanceFrom = 9074, chanceTo = 9082, itemId = 3444}, -- amulet of loss
	{chanceFrom = 9083, chanceTo = 9087, itemId = 6566}, -- stuffed dragon
}

local redSurpriseBag = Action()

function redSurpriseBag.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

redSurpriseBag:id(6571)
redSurpriseBag:register()
