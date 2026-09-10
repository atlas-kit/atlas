local windows = {
	--[itemid] = {toItemid}
	[5302] = {6447}, -- white stone wall window
	[5303] = {6448}, -- white stone wall window
	[6437] = {6435}, -- framework window
	[6435] = {6437}, -- framework window
	[6438] = {6436}, -- framework window
	[6436] = {6438}, -- framework window
	[6441] = {6439}, -- brick window
	[6439] = {6441}, -- brick window
	[6442] = {6440}, -- brick window
	[6440] = {6442}, -- brick window
	[6445] = {6443}, -- stone window
	[6443] = {6445}, -- stone window
	[6446] = {6444}, -- stone window
	[6444] = {6446}, -- stone window
	[6447] = {5302}, -- marble window
	[6448] = {5303}, -- marble window
	[6451] = {6449}, -- tree window
	[6449] = {6451}, -- tree window
	[6452] = {6450}, -- tree window
	[6450] = {6452}, -- tree window
	[6455] = {6453}, -- sandstone window
	[6453] = {6455}, -- sandstone window
	[6456] = {6454}, -- sandstone window
	[6454] = {6456}, -- sandstone window
	[6459] = {6457}, -- bamboo window
	[6457] = {6459}, -- bamboo window
	[6460] = {6458}, -- bamboo window
	[6458] = {6460}, -- bamboo window
	[6463] = {6461}, -- sandstone window
	[6461] = {6463}, -- sandstone window
	[6464] = {6462}, -- sandstone window
	[6462] = {6464}, -- sandstone window
	[6467] = {6465}, -- stone window
	[6465] = {6467}, -- stone window
	[6468] = {6466}, -- stone window
	[6466] = {6468}, -- stone window
	[6471] = {6469}, -- wooden window
	[6469] = {6471}, -- wooden window
	[6472] = {6470}, -- wooden window
	[6470] = {6472}, -- wooden window
	[6790] = {6788}, -- fur wall window
	[6788] = {6790}, -- fur wall window
	[6791] = {6789}, -- fur wall window
	[6789] = {6791}, -- fur wall window
	[7025] = {7027}, -- nordic wall window
	[7027] = {7025}, -- nordic wall window
	[7026] = {7028}, -- nordic wall window
	[7028] = {7026}, -- nordic wall window
	[7029] = {7051}, -- ice wall window
	[7051] = {7029}, -- ice wall window
	[7030] = {7052}, -- ice wall window
	[7052] = {7030}, -- ice wall window
	[9347] = {9349}, -- framework window
	[9349] = {9347}, -- framework window
	[9348] = {9350}, -- framework window
	[9350] = {9348}, -- framework window
	[9571] = {9573}, -- limestone window
	[9573] = {9571}, -- limestone window
	[9572] = {9574}, -- limestone window
	[9574] = {9572}, -- limestone window
	[17147] = {17167}, -- window
	[17148] = {17168}, -- window
	[17161] = {17170}, -- window
	[17160] = {17169}, -- window
	[17163] = {17900}, -- small window
	[17164] = {17901}, -- small window
	[17165] = {17903}, -- small window
	[17166] = {17904}, -- small window
	[17167] = {17147}, -- window
	[17168] = {17148}, -- window
	[17169] = {17160}, -- window
	[17170] = {17161}, -- window
	[17694] = {17902}, -- window
	[17695] = {17905}, -- window
	[17900] = {17163}, -- window
	[17901] = {17164}, -- small window
	[17902] = {17694}, -- window
	[17903] = {17165}, -- window
	[17904] = {17166}, -- small window
	[17905] = {17695}, -- window
	[20442] = {20468}, -- window
	[20468] = {20442}, -- window
	[20441] = {20467}, -- window
	[20467] = {20441}, -- window
	[23853] = {23855}, -- window
	[23855] = {23853}, -- window
	[23854] = {23856}, -- window
	[23856] = {23854}, -- window
	[33628] = {33630}, -- window
	[33630] = {33628}, -- window
	[33629] = {33631}, -- window
	[33631] = {33629}, -- window
	[33638] = {33640}, -- window
	[33640] = {33638}, -- window
	[33639] = {33641}, -- window
	[33641] = {33639}, -- window
	[33642] = {33644}, -- window
	[33644] = {33642}, -- window
	[33643] = {33645}, -- window
	[33645] = {33643}, -- window
	[34926] = {34928}, -- window
	[34928] = {34926}, -- window
	[34927] = {34929}, -- window
	[34929] = {34927}, -- window
}

local window = Action()

function window.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local window = windows[item:getId()]
	if not window then
		return false
	end

	local tile = Tile(fromPosition)
	local house = tile and tile:getHouse()
	if not house then
		fromPosition.y = fromPosition.y - 1
		tile = Tile(fromPosition)
		house = tile and tile:getHouse()
		if not house then
			fromPosition.y = fromPosition.y + 1
			fromPosition.x = fromPosition.x - 1
			tile = Tile(fromPosition)
			house = tile and tile:getHouse()
		end
	end

	if house and player:getTile():getHouse() ~= house and player:getAccountType() < ACCOUNT_TYPE_GAMEMASTER then
		return false
	end

	player:addAchievementProgress("Do Not Disturb", 100)
	player:addAchievementProgress("Let the Sunshine In", 100)
	item:transform(window[1])
	return true
end

for k, v in pairs(windows) do
	window:id(k)
end
window:register()
