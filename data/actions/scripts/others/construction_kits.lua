local constructionKits = {
	[2004] = 3262, [2005] = 3266, [2006] = 3248, [2007] = 3270, [2008] = 3254,
	[2009] = 3306, [2010] = 3310, [2011] = 3206, [395] = 4167, [8373] = 10856,
	[2064] = 3201, [2065] = 3234, [2088] = 3235, [2089] = 3237, [2090] = 3321,
	[2091] = 3329, [2092] = 3337, [2093] = 3424, [2094] = 3423, [2095] = 3377,
	[2096] = 3359, [2097] = 4157, [2098] = 4160, [2099] = 4114, [2100] = 1574,
	[2101] = 4179, [2102] = 3333, [2103] = 2895, [2104] = 2899, [2105] = 2900,
	[2106] = 4085, [3704] = 1602, [3705] = 4139, [3706] = 4143, [3707] = 3314,
	[4310] = 3239, [864] = 4163, [395] = 3240, [5086] = 5046, [5087] = 5055,
	[5088] = 5056, [6114] = 6111, [6115] = 6109, [6369] = 6354, [6370] = 6369,
	[935] = 785, [1754] = 6841, [8376] = 9293, [8383] = 9295, [8373] = 9290,
	[19148] = 9369, [2062] = 3203, [2063] = 3202, [12275] = 13208, [12292] = 13278,
	[14030] = 15494, [14031] = 15496, [14033] = 15498
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local kit = constructionKits[item.itemid]
	if not kit then
		return false
	end

	local tile = Tile(item:getPosition())
	if tile and tile:getHouse() then
		if fromPosition.x ~= CONTAINER_POSITION or item:getParent():getId() == ITEM_BROWSEFIELD then
			item:transform(kit)
			fromPosition:sendMagicEffect(CONST_ME_POFF)
			player:addAchievementProgress("Interior Decorator", 1000)
		else
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "Put the construction kit on the floor first.")
		end
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "You may construct this only inside a house.")
	end
	return true
end
