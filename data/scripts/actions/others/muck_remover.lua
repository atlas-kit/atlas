local config = { -- chance1, chance2, itemID, count
	{from = 1, to = 2344, itemId = 12293}, -- crystal backpack
	{from = 2345, to = 536, itemId = 3145}, -- blue gem
	{from = 537, to = 4714, itemId = 12290}, -- glowing mushroom
	{from = 4715, to = 6224, itemId = 604, count = 5}, -- violet crystal shard
	{from = 6225, to = 323, itemId = 596, count = 10}, -- blue crystal splinter
	{from = 324, to = 144, itemId = 463, count = 10}, -- blue crystal shard
	{from = 5025, to = 8661, itemId = 1944}, -- crystal mace
	{from = 8662, to = 8959, itemId = 3331}, -- crystalline armor
	{from = 8960, to = 9086, itemId = 2150} -- crystalline sword
}

local muckRemover = Action()

function muckRemover.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid ~= 18396 then -- mucus plug
		return false
	end

	local chance, randomItem = math.random(9999)
	for i = 1, #config do
		randomItem = config[i]
		if chance >= randomItem.from and chance <= randomItem.to then
			if toPosition.x == CONTAINER_POSITION then
				player:addItem(randomItem.itemId, randomItem.count or 1)
			else
				Game.createItem(randomItem.itemId, randomItem.count or 1, toPosition)
			end
			player:addAchievementProgress("Goo Goo Dancer", 100)
			target:getPosition():sendMagicEffect(CONST_ME_GREEN_RINGS)
			target:remove(1)
			player:sendSupplyUsed(item)
			item:remove(1)
			break
		end
	end
	return true
end

muckRemover:id(18395)
muckRemover:register()
