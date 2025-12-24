local windows = {
	--[itemid] = {toItemid}
	[5301] = {6446}, -- white stone wall window
	[5302] = {6447}, -- white stone wall window
	[6436] = {6434}, -- framework window
	[6434] = {6436}, -- framework window
	[6437] = {6435}, -- framework window
	[6435] = {6437}, -- framework window
	[6440] = {6438}, -- brick window
	[6438] = {6440}, -- brick window
	[6441] = {6439}, -- brick window
	[6439] = {6441}, -- brick window
	[6444] = {6442}, -- stone window
	[6442] = {6444}, -- stone window
	[6445] = {6443}, -- stone window
	[6443] = {6445}, -- stone window
	[6446] = {5301}, -- marble window
	[6447] = {5302}, -- marble window
	[6450] = {6448}, -- tree window
	[6448] = {6450}, -- tree window
	[6451] = {6449}, -- tree window
	[6449] = {6451}, -- tree window
	[6454] = {6452}, -- sandstone window
	[6452] = {6454}, -- sandstone window
	[6455] = {6453}, -- sandstone window
	[6453] = {6455}, -- sandstone window
	[6458] = {6456}, -- bamboo window
	[6456] = {6458}, -- bamboo window
	[6459] = {6457}, -- bamboo window
	[6457] = {6459}, -- bamboo window
	[6462] = {6460}, -- sandstone window
	[6460] = {6462}, -- sandstone window
	[6463] = {6461}, -- sandstone window
	[6461] = {6463}, -- sandstone window
	[6466] = {6464}, -- stone window
	[6464] = {6466}, -- stone window
	[6467] = {6465}, -- stone window
	[6465] = {6467}, -- stone window
	[6470] = {6468}, -- wooden window
	[6468] = {6470}, -- wooden window
	[6471] = {6469}, -- wooden window
	[6469] = {6471}, -- wooden window
	[6790] = {6788}, -- fur wall window
	[6788] = {6790}, -- fur wall window
	[6791] = {6789}, -- fur wall window
	[6789] = {6791}, -- fur wall window
	[7027] = {7025}, -- nordic wall window
	[7025] = {7027}, -- nordic wall window
	[7028] = {7026}, -- nordic wall window
	[7026] = {7028}, -- nordic wall window
	[7051] = {7031}, -- ice wall window
	[7031] = {7051}, -- ice wall window
	[7052] = {7032}, -- ice wall window
	[7032] = {7052}, -- ice wall window
	[8431] = {8433}, -- framework window
	[8433] = {8431}, -- framework window
	[8432] = {8434}, -- framework window
	[8434] = {8432}, -- framework window
	[8655] = {8657}, -- limestone window
	[8657] = {8655}, -- limestone window
	[8656] = {8658}, -- limestone window
	[8658] = {8656}, -- limestone window
	[15742] = {15969}, -- window
	[15743] = {15970}, -- window
	[15963] = {15972}, -- window
	[15962] = {15971}, -- window
	[15965] = {15357}, -- small window
	[15966] = {15358}, -- small window
	[15967] = {15360}, -- small window
	[15968] = {15361}, -- small window
	[15969] = {15742}, -- window
	[15970] = {15743}, -- window
	[15971] = {15962}, -- window
	[15972] = {15963}, -- window
	[15069] = {15359}, -- window
	[15070] = {15362}, -- window
	[15357] = {15965}, -- window
	[15358] = {15966}, -- small window
	[15359] = {15069}, -- window
	[15360] = {15967}, -- window
	[15361] = {15968}, -- small window
	[15362] = {15070}, -- window
	[18161] = {18187}, -- window
	[18187] = {18161}, -- window
	[18160] = {18186}, -- window
	[18186] = {18160}, -- window
	[21484] = {21486}, -- window
	[21486] = {21484}, -- window
	[21485] = {21487}, -- window
	[21487] = {21485}, -- window
	[30972] = {30974}, -- window
	[30974] = {30972}, -- window
	[30973] = {30975}, -- window
	[30975] = {30973}, -- window
	[30982] = {30984}, -- window
	[30984] = {30982}, -- window
	[30983] = {30985}, -- window
	[30985] = {30983}, -- window
	[30986] = {30988}, -- window
	[30988] = {30986}, -- window
	[30987] = {30989}, -- window
	[30989] = {30987}, -- window
	[32270] = {32272}, -- window
	[32272] = {32270}, -- window
	[32271] = {34929}, -- window
	[34929] = {32271}, -- window
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
