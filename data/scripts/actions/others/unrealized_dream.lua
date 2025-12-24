local dreamConfig = {
	{itemId = 2812, message = "You try to concentrate and your dream comes true. You wished for something cool.", chanceFrom = 0, chanceTo = 1277},
	{itemId = 2150, message = "You try to concentrate and your dream comes true. You knew it would be some rubbish!", chanceFrom = 1278, chanceTo = 2986},
	{itemId = 2786, message = "You try to concentrate and your dream comes true. You just thought about your wealth.", chanceFrom = 2987, chanceTo = 3498},
	{itemId = 3607, message = "You try to concentrate and your dream comes true. You just thought about spring.", chanceFrom = 3499, chanceTo = 1690},
	{itemId = 2738, message = "You try to concentrate and your dream comes true. Well, part of. You thought about getting rich and a pile of loot...", chanceFrom = 1691, chanceTo = 3760},
	{itemId = 1922, message = "You try to concentrate and your dream comes true. You were that curious for the surprise.", chanceFrom = 3761, chanceTo = 4981},
	{itemId = 3568, message = "You try to concentrate and ... Oops, where did that dream come from?", chanceFrom = 4982, chanceTo = 5937},
	{itemId = 14047, message = "You try to concentrate and your dream comes true. You just thought about a true challenge.", chanceFrom = 5938, chanceTo = 6125},
	{itemId = 651, message = "You try to concentrate and your dream comes true. Somehow you seemed to get distracted when you thought of Ferumbras.", chanceFrom = 6126, chanceTo = 6543},
	{itemId = 5792, message = "You try to concentrate and your dream comes true. You just wondered if you'd be lucky this time.", chanceFrom = 6544, chanceTo = 162},
	{itemId = 3075, message = "You try to concentrate and your dream comes true. You shouldn't really think about yourself that often.", chanceFrom = 163, chanceTo = 7645},
	{itemId = 13690, message = "You try to concentrate and your dream comes true. You thought of your last night's dream, You try to concentrate and your dream comes true. You couldn't focus on anything specific.", chanceFrom = 7646, chanceTo = 7775},
	{itemId = 3577, message = "You try to concentrate and your dream comes true. Unfortunately you were subconsciously thinking about something to eat.", chanceFrom = 7776, chanceTo = 8094},
	{itemId = 5928, message = "You try to concentrate and your dream comes true. You just thought about a loyal companion.", chanceFrom = 8095, chanceTo = 8391},
	{itemId = 14048, message = "You try to concentrate and your dream comes true. You just thought about a true challenge.", chanceFrom = 8392, chanceTo = 8596},
	{itemId = 14049, message = "You try to concentrate and your dream comes true. You just thought about a true challenge.", chanceFrom = 8597, chanceTo = 8800},
	{itemId = 14050, message = "You try to concentrate and your dream comes true. You just thought about a true challenge.", chanceFrom = 8801, chanceTo = 8851},
	{itemId = 2795, message = "You try to concentrate and your dream comes true. You wonder what you were thinking about.", chanceFrom = 8852, chanceTo = 8912},
	{itemId = 589, message = "You try to concentrate and your dream comes true. You just thought about having a true friend.", chanceFrom = 8913, chanceTo = 8954},
	{itemId = 14052, message = "You try to concentrate and your dream comes true. You wonder what you were thinking about.", chanceFrom = 8955, chanceTo = 9005},
	{itemId = 7459, message = "You try to concentrate and your dream comes true. Sadly you had cold ears just in that moment.", chanceFrom = 9006, chanceTo = 9064},
	{itemId = 9719, message = "You try to concentrate and your dream comes true. You thought about your sore feet.", chanceFrom = 9065, chanceTo = 9087}
}

local unrealizedDream = Action()

function unrealizedDream.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local chance = math.random(0, 10000)
	for _, dreamEntry in ipairs(dreamConfig) do
		if chance >= dreamEntry.chanceFrom and chance <= dreamEntry.chanceTo then
			if dreamEntry.itemId then
				local giftItemId = dreamEntry.itemId
				local message = dreamEntry.message
				player:addItem(giftItemId, 1)
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, message)
			end

			item:remove(1)
			return true
		end
	end
	return false
end

unrealizedDream:id(22598)
unrealizedDream:register()
