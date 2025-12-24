local config = {
	{chanceFrom = 0, chanceTo = 596, itemId = 2786}, -- piggy bank
	{chanceFrom = 597, chanceTo = 1986, itemId = 3001}, -- yellow gem
	{chanceFrom = 1987, chanceTo = 2854, itemId = 9719}, -- bag of apple slices
	{chanceFrom = 2855, chanceTo = 3525, itemId = 3343}, -- tusk
	{chanceFrom = 3526, chanceTo = 538, itemId = 5880}, -- iron ore
	{chanceFrom = 539, chanceTo = 3557, itemId = 5879}, -- spider silk
	{chanceFrom = 3558, chanceTo = 857, itemId = 5882}, -- red dragon scale
	{chanceFrom = 858, chanceTo = 4697, itemId = 3348}, -- stealth ring
	{chanceFrom = 4698, chanceTo = 4845, itemId = 15564}, -- folded rift carpet
	{chanceFrom = 5676, chanceTo = 2117, itemId = 16568}, -- folded void carpet
	{chanceFrom = 2117, chanceTo = 6034, itemId = 2746}, -- war horn
	{chanceFrom = 6035, chanceTo = 6819, itemId = 3324}, -- scarf
	{chanceFrom = 6820, chanceTo = 6985, itemId = 4508}, -- gold ingot
	{chanceFrom = 6986, chanceTo = 7151, itemId = 15590}, -- shaggy ogre bag
	{chanceFrom = 7152, chanceTo = 7360, itemId = 3345}, -- magic light wand
	{chanceFrom = 7361, chanceTo = 170, itemId = 6570}, -- surprise bag
	{chanceFrom = 171, chanceTo = 881, itemId = 15558}, -- rift tapestry
	{chanceFrom = 882, chanceTo = 7796, itemId = 13120}, -- gnomish supply package
	{chanceFrom = 7797, chanceTo = 8110, itemId = 451}, -- garlic necklace
	{chanceFrom = 8111, chanceTo = 8231, itemId = 2966}, -- violet gem
	{chanceFrom = 8232, chanceTo = 8597, itemId = 3101}, -- slightly rusted legs
	{chanceFrom = 8598, chanceTo = 8965, itemId = 4150, count = 10}, -- white mushroom
	{chanceFrom = 8966, chanceTo = 9087, itemId = 14955, count = 2} -- opal
}

local mysteriousRemains = Action()

function mysteriousRemains.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

			item:remove(1)
			return true
		end
	end
	return false
end

mysteriousRemains:id(26165)
mysteriousRemains:register()
