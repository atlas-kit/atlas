local transformID = {
	[20366] = 20365, [20365] = 20366, -- rift carpet
	[21166] = 21165, [21165] = 21166, -- void carpet
	[21060] = 21082, [21082] = 21060, -- yalaharian carpet
	[21061] = 21083, [21083] = 21061, -- white fur carpet
	[21062] = 21084, [21084] = 21062, -- bamboo mat carpet
	[21344] = 21336, [21336] = 21344, -- crimson carpet
	[21339] = 21345, [21345] = 21339, -- azure carpet
	[21340] = 21346, [21346] = 21340, -- emerald carpet
	[21341] = 21347, [21347] = 21341, -- light parquet carpet
	[21342] = 21348, [21348] = 21342, -- dark parquet carpet
	[21343] = 21349, [21349] = 21343, -- marble floor
	[16951] = 16959, [16959] = 16951, -- colourful carpet
	[16952] = 16960, [16960] = 16952, -- flowery carpet
	[16953] = 16961, [16961] = 16953, -- striped carpet
	[16954] = 16962, [16962] = 16954, -- fur carpet
	[16955] = 16963, [16963] = 16955, -- diamond carpet
	[16956] = 16964, [16964] = 16956, -- patterned carpet
	[16957] = 16965, [16965] = 16957, -- night sky carpet
	[16958] = 16966, [16966] = 16958, -- star carpet
	[23458] = 23459, [23459] = 23458, -- verdant carpet
	[23460] = 23461, [23461] = 23460, -- shaggy carpet
	[23462] = 23463, [23463] = 23462, -- mystic carpet
	[23464] = 23465, [23465] = 23464, -- stone tiles
	[23466] = 23467, [23467] = 23466, -- wooden planks
	[23494] = 23495, [23495] = 23494, -- wheat carpet
	[23496] = 23497, [23497] = 23496, -- crested carpet
	[23498] = 23499, [23499] = 23498, -- decorated carpet
	[28810] = 28812, [28812] = 28810, -- tournament carpet
	[28811] = 28813, [28813] = 28811, -- sublime tournament carpet
	[33231] = 33232, [33232] = 33231, -- lilac carpet
	[33233] = 33234, [33234] = 33233, -- colourful pom-pom carpet
	[33235] = 33236, [33236] = 33235, -- natural pom-pom carpet
	[33237] = 33238, [33238] = 33237, -- owin rug
	[33239] = 33240, [33240] = 33239, -- midnight panther rug
	[33241] = 33242, [33242] = 33241, -- moon carpet
	[33243] = 33244, [33244] = 33243, -- romantic carpet
	[33285] = 33286, [33286] = 33285, -- grandiose carpet
	[36496] = 34182, [34182] = 36496, -- eldritch carpet
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
	[35107] = 35108, [35108] = 35107, -- Zaoan bamboo tiles
	[35109] = 35110, [35110] = 35109, -- Zaoan bamboo tiles
	[35111] = 35112, [35112] = 35111, -- Zaoan bamboo tiles
	[35113] = 35114, [35114] = 35113, -- Zaoan bamboo tiles
	[35115] = 35116, [35116] = 35115, -- Zaoan bamboo tiles
	[35117] = 35118, [35118] = 35117, -- Zaoan bamboo tiles
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
