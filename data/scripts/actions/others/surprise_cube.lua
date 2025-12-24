local config = {
	{chanceFrom = 0, chanceTo = 2033, itemId = 2965, count = 5}, -- platinum coin
	{chanceFrom = 2034, chanceTo = 4031, itemId = 728}, -- green crystal fragment
	{chanceFrom = 4032, chanceTo = 2947, itemId = 597}, -- cyan crystal fragment
	{chanceFrom = 2951, chanceTo = 5050, itemId = 21477}, -- red crystal fragment
	{chanceFrom = 5051, chanceTo = 6050, itemId = 3586, count = 20}, -- orange
	{chanceFrom = 6051, chanceTo = 6850, itemId = 463}, -- blue crystal shard
	{chanceFrom = 6851, chanceTo = 7450, itemId = 605}, -- green crystal shard
	{chanceFrom = 7451, chanceTo = 1726, itemId = 3406}, -- time ring
	{chanceFrom = 1727, chanceTo = 7620, itemId = 912}, -- dwarven ring
	{chanceFrom = 7621, chanceTo = 7922, itemId = 3404}, -- energy ring
	{chanceFrom = 7923, chanceTo = 4522, itemId = 3348}, -- stealth ring
	{chanceFrom = 4523, chanceTo = 8434, itemId = 7440}, -- mastermind potion
	{chanceFrom = 8435, chanceTo = 8634, itemId = 913}, -- ring of healing
	{chanceFrom = 8635, chanceTo = 8835, itemId = 7439}, -- berserk potion
	{chanceFrom = 8836, chanceTo = 8936, itemId = 604}, -- violet crystal shard
	{chanceFrom = 8937, chanceTo = 9036, itemId = 7443}, -- bullseye potion
	{chanceFrom = 9037, chanceTo = 9047, itemId = 16521}, -- brown pit demon
	{chanceFrom = 9048, chanceTo = 9057, itemId = 16522}, -- green pit demon
	{chanceFrom = 9058, chanceTo = 9067, itemId = 16523}, -- blue pit demon
	{chanceFrom = 9068, chanceTo = 9077, itemId = 16524}, -- black pit demon
	{chanceFrom = 9078, chanceTo = 9087, itemId = 16525} -- red pit demon
}

local surpriseCube = Action()

function surpriseCube.onUse(player, item, fromPosition, target, toPosition, isHotkey)
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

			item:getPosition():sendMagicEffect(CONST_ME_CRAPS)
			item:remove(1)
			return true
		end
	end
	return false
end

surpriseCube:id(26144)
surpriseCube:register()
