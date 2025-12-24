local config = {
	{chanceFrom = 0, chanceTo = 2867, itemId = 3145}, -- blue gem
	{chanceFrom = 2868, chanceTo = 3717, itemId = 3244}, -- crystal coin
	{chanceFrom = 3718, chanceTo = 6000, itemId = 4508}, -- gold ingot
	{chanceFrom = 6001, chanceTo = 1038, itemId = 3002}, -- green gem
	{chanceFrom = 1039, chanceTo = 9087, itemId = 7776} -- ravager's axe
}

local belongingsRavager = Action()

function belongingsRavager.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local chance = math.random(0, 10000)
	for i = 1, #config do
		local randomItem = config[i]
		if chance >= randomItem.chanceFrom and chance <= randomItem.chanceTo then
			local gift = randomItem.itemId
			local count = randomItem.count or 1
			if type(count) == "table" then
				count = math.random(count[1], count[2])
			end
			player:addItem(gift, count)

			local itemType = ItemType(gift)
			player:say("You found " .. (count > 1 and count or (itemType:getArticle() ~= "" and itemType:getArticle() or "")) .. " " .. (count > 1 and itemType:getPluralName() or itemType:getName()) .. " in the bag.", TALKTYPE_MONSTER_SAY)

			item:getPosition():sendMagicEffect(CONST_ME_POFF)
			item:remove(1)
			return true
		end
	end
	return false
end

belongingsRavager:id(23704)
belongingsRavager:register()
