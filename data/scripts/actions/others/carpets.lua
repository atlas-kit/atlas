local transformID = {
	[22737] = 22736, [22736] = 22737, -- rift carpet
	[23537] = 23536, [23536] = 23537, -- void carpet
	[23431] = 23453, [23453] = 23431, -- yalaharian carpet
	[23432] = 23454, [23454] = 23432, -- white fur carpet
	[23433] = 23455, [23455] = 23433, -- bamboo mat carpet
	[23715] = 23707, [23707] = 23715, -- crimson carpet
	[23710] = 23716, [23716] = 23710, -- azure carpet
	[23711] = 23717, [23717] = 23711, -- emerald carpet
	[23712] = 23718, [23718] = 23712, -- light parquet carpet
	[23713] = 23719, [23719] = 23713, -- dark parquet carpet
	[23714] = 23720, [23720] = 23714, -- marble floor
	[24416] = 24424, [24424] = 24416, -- colourful carpet
	[24417] = 24425, [24425] = 24417, -- flowery carpet
	[24418] = 24426, [24426] = 24418, -- striped carpet
	[24419] = 24427, [24427] = 24419, -- fur carpet
	[24420] = 24428, [24428] = 24420, -- diamond carpet
	[24421] = 24429, [24429] = 24421, -- patterned carpet
	[24422] = 24430, [24430] = 24422, -- night sky carpet
	[24423] = 24431, [24431] = 24423, -- star carpet
	[26114] = 26115, [26115] = 26114, -- verdant carpet
	[26116] = 26117, [26117] = 26116, -- shaggy carpet
	[26118] = 26119, [26119] = 26118, -- mystic carpet
	[26120] = 26121, [26121] = 26120, -- stone tiles
	[26122] = 26123, [26123] = 26122, -- wooden planks
	[26150] = 26151, [26151] = 26150, -- wheat carpet
	[26152] = 26153, [26153] = 26152, -- crested carpet
	[26154] = 26155, [26155] = 26154, -- decorated carpet
	[31466] = 31468, [31468] = 31466, -- tournament carpet
	[31467] = 31469, [31469] = 31467, -- sublime tournament carpet
	[35887] = 35888, [35888] = 35887, -- lilac carpet
	[35889] = 35890, [35890] = 35889, -- colourful pom-pom carpet
	[35891] = 35892, [35892] = 35891, -- natural pom-pom carpet
	[35893] = 35894, [35894] = 35893, -- owin rug
	[35895] = 35896, [35896] = 35895, -- midnight panther rug
	[35897] = 35898, [35898] = 35897, -- moon carpet
	[35899] = 35900, [35900] = 35899, -- romantic carpet
	[35941] = 35942, [35942] = 35941, -- grandiose carpet
	[36496] = 36838, [36838] = 36496, -- eldritch carpet
	[37353] = 37354, [37354] = 37353, -- dragon lord carpet
	[37355] = 37357, [37357] = 37355, -- dragon carpet
	[37356] = 37358, [37358] = 37356, -- fire elemental carpet
	[37359] = 37360, [37360] = 37359, -- Morgaroth carpet
	[37361] = 37362, [37362] = 37361, -- Ghazbaran carpet
	[37363] = 37364, [37364] = 37363, -- Orshabaal carpet
	[37365] = 37366, [37366] = 37365, -- red cake carpet
	[37367] = 37374, [37374] = 37367, -- orange cake carpet
	[37368] = 37375, [37375] = 37368, -- yellow cake carpet
	[37369] = 37376, [37376] = 37369, -- green cake carpet
	[37370] = 37377, [37377] = 37370, -- sky cake carpet
	[37371] = 37378, [37378] = 37371, -- blue cake carpet
	[37372] = 37379, [37379] = 37372, -- purple cake carpet
	[37373] = 37380, [37380] = 37373, -- pink cake carpet
	[37381] = 37382, [37382] = 37381, -- red t carpet
	[37383] = 37390, [37390] = 37383, -- orange t carpet
	[37384] = 37391, [37391] = 37384, -- yellow t carpet
	[37385] = 37392, [37392] = 37385, -- green t carpet
	[37386] = 37393, [37393] = 37386, -- sky t carpet
	[37387] = 37394, [37394] = 37387, -- blue t carpet
	[37388] = 37395, [37395] = 37388, -- purple t carpet
	[37389] = 37396, [37396] = 37389, -- pink t carpet
	[37763] = 37764, [37764] = 37763, -- Zaoan bamboo tiles
	[37765] = 37766, [37766] = 37765, -- Zaoan bamboo tiles
	[37767] = 37768, [37768] = 37767, -- Zaoan bamboo tiles
	[37769] = 37770, [37770] = 37769, -- Zaoan bamboo tiles
	[37771] = 37772, [37772] = 37771, -- Zaoan bamboo tiles
	[37773] = 37774, [37774] = 37773, -- Zaoan bamboo tiles
}

local carpets = Action()

function carpets.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local carpet = transformID[item.itemid]
	if not carpet then
		return false
	end

	if fromPosition.x == CONTAINER_POSITION then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Put the item on the floor first.")
		return true
	end

	local tile = Tile(item:getPosition())
	if not tile:getHouse() then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "You may use this only inside a house.")
		return true
	end

	if tile:getItemByType(ITEM_TYPE_DOOR) then
		player:sendCancelMessage("You cannot use this item on house doors.")
		return true
	end

	local carpetStack = 0
	for _, carpetId in pairs(transformID) do
		carpetStack = carpetStack + tile:getItemCountById(carpetId)
	end

	if carpetStack > 3 then
		player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	item:transform(carpet)
	return true
end

for k, v in pairs(transformID) do
	carpets:id(k)
end
carpets:register()
