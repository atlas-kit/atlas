local action = Action()

local decayItems = {
	[2660] = 2661, [2661] = 2660, -- cuckoo clock
	[2662] = 2663, [2663] = 2662, -- cuckoo clock
	[2911] = 2912, [2912] = 2911, -- candelabrum
	[2914] = 2915, [2915] = 2914, -- lamp
	[2917] = 2918, [2918] = 2917, -- candlestick
	[2920] = 2921, [2921] = 2920, -- torch
	[2922] = 2923, [2923] = 2922, -- torch
	[2924] = 2925, [2925] = 2924, -- torch
	[3046] = 3047, [3047] = 3046, -- magic light wand
	[5812] = 5813, [5813] = 5812, -- skull candle
	[7183] = 7184, -- baby seal doll
	[9802] = 9803, -- friendship amulet
	[3997] = 4010, -- Tibiora's box
	[22889] = 22890, -- shield of destiny
	[23442] = 23443 -- baby dragon
}

function action.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	item:transform(decayItems[item:getId()])
	item:decay()
	return true
end

for k, _ in pairs(decayItems) do action:id(k) end
action:register()
