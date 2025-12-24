local decayItems = {
	[2435] = 1719, [2436] = 1718, -- cuckoo clock
	[2437] = 1721, [2438] = 1720, -- cuckoo clock
	[2461] = 3829, [2462] = 3828, -- candelabrum
	[2464] = 3832, [2465] = 3831, -- lamp
	[2467] = 3835, [2468] = 3834, -- candlestick
	[2504] = 3838, [2505] = 3837, -- torch
	[2506] = 3840, [2507] = 3839, -- torch
	[703] = 3842, [2509] = 3841, -- torch
	[3345] = 3964, [3346] = 3963, -- magic light wand
	[5812] = 5813, [5813] = 5812, -- skull candle
	[7183] = 7184, -- baby seal doll
	[427] = 8061, -- friendship amulet
	[2958] = 2243, -- Tibiora's box
	[15721] = 18238, -- shield of destiny
	[16366] = 18647 -- baby dragon
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local transformIds = decayItems[item:getId()]
	if not transformIds then
		return false
	end

	item:transform(transformIds)
	item:decay()
	return true
end
