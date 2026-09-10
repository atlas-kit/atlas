local rewards = { -- chanceMin, chanceMax, itemID, count
	{1, 36}, -- nothing
	{37, 46, 3031, 80}, -- gold coin
	{47, 55, 3031, 50}, -- gold coin
	{56, 64, 3582, 5}, -- ham
	{65, 73, 3725, 5}, -- brown mushroom
	{74, 81, 7620}, -- mana potion
	{82, 87, 266}, -- health potion
	{88, 92, 8897}, -- rusty legs (common)
	{93, 96, 8894}, -- rusty armor (common)
	{97, 100, 3097} -- dwarven ring
}

local crateUsable = Action()

function crateUsable.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if (player:getStorageValue(PlayerStorageKeys.crateUsable)) <= os.time() then
		local totalChance = math.random(100)
		for i = 1, #rewards do
			local reward = rewards[i]
			if totalChance >= reward[1] and totalChance <= reward[2] then
				if reward[3] then
					local item = ItemType(reward[3])
					local count = reward[4] or 1
					player:addItem(reward[3], count)
					local str = ("You found %s %s!"):format(count > 1 and count or item:getArticle(), count > 1 and item:getPluralName() or item:getName())
					player:say(str, TALKTYPE_MONSTER_SAY, false, player, toPosition)
					player:setStorageValue(PlayerStorageKeys.crateUsable, os.time() + 1 * 60 * 60)
					player:addAchievementProgress("Free Items!", 50)
				else
					player:say("You found nothing useful.", TALKTYPE_MONSTER_SAY, false, player, toPosition)
				end
				break
			end
		end
	else
		player:say("You found nothing useful.", TALKTYPE_MONSTER_SAY, false, player, toPosition)
	end
	return true
end

crateUsable:id(8745)
crateUsable:register()
