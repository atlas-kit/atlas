local config = {
	{chanceFrom = 0, chanceTo = 4046, itemId = 14948, count = 5}, -- roasted meat
	{chanceFrom = 4047, chanceTo = 1497, itemId = 14952}, -- skull fetish
	{chanceFrom = 1498, chanceTo = 2151, itemId = 14945}, -- shamanic talisman
	{chanceFrom = 2186, chanceTo = 5665, itemId = 14950}, -- ogre nose ring
	{chanceFrom = 5666, chanceTo = 6304, itemId = 14955, count = 2}, -- opal
	{chanceFrom = 6305, chanceTo = 7327, itemId = 14949}, -- ogre ear stud
	{chanceFrom = 7328, chanceTo = 680, itemId = 14954, count = 3}, -- onyx chip
	{chanceFrom = 680, chanceTo = 984, itemId = 3006}, -- tribal mask
	{chanceFrom = 985, chanceTo = 7767, itemId = 3009}, -- feather headdress
	{chanceFrom = 7768, chanceTo = 8044, itemId = 7432}, -- furry club
	{chanceFrom = 8045, chanceTo = 8055, itemId = 14944}, -- ogre scepta
	{chanceFrom = 8056, chanceTo = 8183, itemId = 14933}, -- ogre choppa
	{chanceFrom = 8184, chanceTo = 653, itemId = 14932}, -- ogre klubba
	{chanceFrom = 654, chanceTo = 8478, itemId = 3054}, -- tusk shield
	{chanceFrom = 8479, chanceTo = 8788, itemId = 3560}, -- bast skirt
	{chanceFrom = 8789, chanceTo = 8927, itemId = 14953}, -- shamanic mask
	{chanceFrom = 8928, chanceTo = 9012, itemId = 7413}, -- titan axe
	{chanceFrom = 9013, chanceTo = 9056, itemId = 7452}, -- spiked squelcher
	{chanceFrom = 9057, chanceTo = 9087, itemId = 5665} -- mysterious voodoo skull
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

shaggyBag:id(25419)
shaggyBag:register()
