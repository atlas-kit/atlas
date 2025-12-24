local config = {
	{chanceFrom = 0, chanceTo = 216}, -- nothing
	{chanceFrom = 217, chanceTo = 2367, itemId = 2552}, -- worn leather boots
	{chanceFrom = 2368, chanceTo = 4251, itemId = 2551}, -- dirty cape
	{chanceFrom = 4252, chanceTo = 4454, itemId = 2787}, -- broken piggy bank
	{chanceFrom = 4455, chanceTo = 2117, itemId = 2945, count = 6}, -- gold coin
	{chanceFrom = 2117, chanceTo = 5545, itemId = 4150}, -- white mushroom
	{chanceFrom = 5546, chanceTo = 6025, itemId = 5890}, -- chicken feather
	{chanceFrom = 6026, chanceTo = 6512, itemId = 3106, count = 4}, -- worm
	{chanceFrom = 6513, chanceTo = 6999, itemId = 3546}, -- egg
	{chanceFrom = 7000, chanceTo = 7398, itemId = 5899}, -- turtle shell
	{chanceFrom = 7399, chanceTo = 725, itemId = 5894}, -- bat wing
	{chanceFrom = 726, chanceTo = 6286, itemId = 3291}, -- knife
	{chanceFrom = 6287, chanceTo = 7730, itemId = 883}, -- bunch of troll hair
	{chanceFrom = 7731, chanceTo = 8087, itemId = 3297}, -- spider fangs
	{chanceFrom = 8088, chanceTo = 8295, itemId = 5902}, -- honeycomb
	{chanceFrom = 8296, chanceTo = 8428, itemId = 3324}, -- scarf
	{chanceFrom = 8429, chanceTo = 8524, itemId = 451}, -- garlic necklace
	{chanceFrom = 7611, chanceTo = 8609, itemId = 9927}, -- plague bell
	{chanceFrom = 8610, chanceTo = 8682, itemId = 2940}, -- white pearl
	{chanceFrom = 8683, chanceTo = 8748, itemId = 5879}, -- spider silk
	{chanceFrom = 8749, chanceTo = 8813, itemId = 2786}, -- piggy bank
	{chanceFrom = 8814, chanceTo = 8878, itemId = 9926}, -- plague mask
	{chanceFrom = 8879, chanceTo = 8928, itemId = 829}, -- book of prayers
	{chanceFrom = 8929, chanceTo = 8972, itemId = 2782}, -- doll
	{chanceFrom = 8973, chanceTo = 9015, itemId = 5880}, -- iron ore
	{chanceFrom = 9016, chanceTo = 9055, itemId = 5895}, -- fish fin
	{chanceFrom = 9056, chanceTo = 9082, itemId = 9690}, -- slug drug
	{chanceFrom = 9083, chanceTo = 9084, itemId = 4391}, -- boots of haste
	{chanceFrom = 9085, chanceTo = 9086, itemId = 5885}, -- flask of warrior's sweat
	{chanceFrom = 9086, chanceTo = 9087, itemId = 5804} -- nose ring
}

local belongings = Action()

function belongings.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

				local itemType = ItemType(gift)
				player:say("You found " .. (count > 1 and count or (itemType:getArticle() ~= "" and itemType:getArticle() or "")) .. " " .. (count > 1 and itemType:getPluralName() or itemType:getName()) .. " in the bag.", TALKTYPE_MONSTER_SAY)
			else
				player:say("You found nothing useful.", TALKTYPE_MONSTER_SAY)
			end

			item:getPosition():sendMagicEffect(CONST_ME_POFF)
			item:remove(1)
			return true
		end
	end
	return false
end

belongings:id(13670)
belongings:register()
