local config = {
	{chanceFrom = 0, chanceTo = 2662, itemId = 15218, count = 2}, -- glooth spear
	{chanceFrom = 2663, chanceTo = 1507, itemId = 15243}, -- glooth amulet
	{chanceFrom = 1508, chanceTo = 4931, itemId = 14388}, -- glooth club
	{chanceFrom = 4932, chanceTo = 6396, itemId = 14390}, -- glooth axe
	{chanceFrom = 6397, chanceTo = 752, itemId = 14389}, -- glooth blade
	{chanceFrom = 753, chanceTo = 7632, itemId = 15353}, -- glooth backpack
	{chanceFrom = 7633, chanceTo = 8147, itemId = 15203, count = 10}, -- glooth sandwich
	{chanceFrom = 8148, chanceTo = 8553, itemId = 15204, count = 10}, -- bowl of glooth soup
	{chanceFrom = 8554, chanceTo = 9008, itemId = 14369, count = 10}, -- glooth steak
	{chanceFrom = 9009, chanceTo = 9087, itemId = 15246} -- control unit
}

local gloothBag = Action()

function gloothBag.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

gloothBag:id(23574)
gloothBag:register()
