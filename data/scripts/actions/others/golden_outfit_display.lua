local transformDisplay = {
	[28853] = 31500,
	[31500] = 28849,
	[28849] = 31513,
	[31513] = 28853,
	[28854] = 31502,
	[31502] = 28850,
	[28850] = 31514,
	[31514] = 28854,
	[28855] = 31503,
	[31503] = 28851,
	[28851] = 28859,
	[28859] = 28855,
	[31512] = 31504,
	[31504] = 28852,
	[28852] = 28860,
	[28860] = 31512
}

local goldenOutfitDisplay = Action()

function goldenOutfitDisplay.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local transformIds = transformDisplay[item:getId()]
	if not transformIds then
		return false
	end

	if player:getStorageValue(PlayerStorageKeys.goldenOutfit) == 3 then
		item:transform(transformIds)
		item:getPosition():sendMagicEffect(CONST_ME_EARLY_THUNDER)
	else
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You need Full Golden Outfit to use it.")
		item:getPosition():sendMagicEffect(CONST_ME_POFF)
	end
	return true
end

for index, value in pairs(transformDisplay) do
	goldenOutfitDisplay:id(index)
end

goldenOutfitDisplay:register()
