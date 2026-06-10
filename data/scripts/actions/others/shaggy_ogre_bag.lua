local config = {
	{chanceFrom = 0, chanceTo = 2865, itemId = 22187, count = 5}, -- roasted meat
	{chanceFrom = 2866, chanceTo = 3408, itemId = 22191}, -- skull fetish
	{chanceFrom = 3409, chanceTo = 4356, itemId = 22184}, -- shamanic talisman
	{chanceFrom = 4357, chanceTo = 5666, itemId = 22189}, -- ogre nose ring
	{chanceFrom = 5667, chanceTo = 6305, itemId = 22194, count = 2}, -- opal
	{chanceFrom = 6306, chanceTo = 7327, itemId = 22188}, -- ogre ear stud
	{chanceFrom = 7328, chanceTo = 7764, itemId = 22193, count = 3}, -- onyx chip
	{chanceFrom = 7764, chanceTo = 8339, itemId = 3403}, -- tribal mask
	{chanceFrom = 8340, chanceTo = 8595, itemId = 3406}, -- feather headdress
	{chanceFrom = 8596, chanceTo = 8872, itemId = 7432}, -- furry club
	{chanceFrom = 8873, chanceTo = 8883, itemId = 22183}, -- ogre scepta
	{chanceFrom = 8884, chanceTo = 9011, itemId = 22172}, -- ogre choppa
	{chanceFrom = 9012, chanceTo = 9075, itemId = 22171}, -- ogre klubba
	{chanceFrom = 9076, chanceTo = 9394, itemId = 3443}, -- tusk shield
	{chanceFrom = 9395, chanceTo = 9703, itemId = 3560}, -- bast skirt
	{chanceFrom = 9704, chanceTo = 9841, itemId = 22192}, -- shamanic mask
	{chanceFrom = 9842, chanceTo = 9926, itemId = 7413}, -- titan axe
	{chanceFrom = 9927, chanceTo = 9969, itemId = 7452}, -- spiked squelcher
	{chanceFrom = 9970, chanceTo = 10000, itemId = 5668} -- mysterious voodoo skull
}

local shaggyBag = Action()

function shaggyBag.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

shaggyBag:id(22763)
shaggyBag:register()
