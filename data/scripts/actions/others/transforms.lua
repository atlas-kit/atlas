local transformItems = {
	[2108] = 2109, [2109] = 2108, -- street lamp
	[2334] = 2335, [2335] = 2334, -- table
	[2336] = 2337, [2337] = 2336, -- table
	[2338] = 2339, [2339] = 2338, -- table
	[2340] = 2341, [2341] = 2340, -- table
	[2535] = 2536, [2536] = 2535, -- oven
	[2537] = 2538, [2538] = 2537, -- oven
	[2539] = 2540, [2540] = 2539, -- oven
	[2541] = 2542, [2542] = 2541, -- oven
	[2772] = 2773, [2773] = 2772, -- lever
	[2907] = 2908, [2908] = 2907, -- wall lamp
	[2909] = 2910, [2910] = 2909, -- wall lamp
	[2928] = 2929, [2929] = 2928, -- torch bearer
	[2930] = 2931, [2931] = 2930, -- torch bearer
	[2934] = 2935, [2935] = 2934, -- table lamp
	[2936] = 2937, [2937] = 2936, -- wall lamp
	[2938] = 2939, [2939] = 2938, -- wall lamp
	[2977] = 2978, [2978] = 2977, -- pumpkinhead
	[3481] = 3482, -- trap
	[2062] = 2063, [2063] = 2062, -- sacred statue
	[2064] = 2065, [2065] = 2064, -- sacred statue
	[2116] = 2117, [2117] = 2116, -- bamboo lamp
	[2940] = 2941, [2941] = 2940, -- torch bearer
	[2942] = 2943, [2943] = 2942, -- torch bearer
	[2944] = 2945, [2945] = 2944, -- wall lamp
	[2946] = 2947, [2947] = 2946, -- wall lamp
	[6488] = 6489, [6489] = 6488, -- christmas branch
	[7058] = 7059, [7059] = 7058, -- skull pillar
	[7856] = 7857, [7857] = 7856, -- chimney
	[7858] = 7859, [7859] = 7858, -- chimney
	[7860] = 7861, [7861] = 7860, -- chimney
	[7862] = 7863, [7863] = 7862, -- chimney
	[8659] = 8660, [8660] = 8659, -- street lamp (yalahar)
	[8661] = 8662, [8662] = 8661, -- street lamp (yalahar)
	[8663] = 8664, [8664] = 8663, -- street lamp (yalahar)
	[8665] = 8666, [8666] = 8665, -- street lamp (yalahar)
	[8832] = 8833, [8833] = 8832, -- wall lamp
	[8834] = 8835, [8835] = 8834, -- wall lamp
	[8911] = 8912, [8912] = 8911, -- lever
	[8913] = 8914, [8914] = 8913, -- lever
	[8924] = 8925, [8925] = 8924, -- wall lamp
	[8926] = 8927, [8927] = 8926, -- wall lamp
	[8928] = 8929, [8929] = 8928, -- wall lamp
	[8930] = 8931, [8931] = 8930, -- wall lamp
	[9063] = 9064, -- crystal pedestal
	[9064] = 9065, -- crystal pedestal
	[9065] = 9066, -- crystal pedestal
	[9066] = 9063, -- crystal pedestal
	[9110] = 9111, [9111] = 9110, -- lever
	[9125] = 9126, [9126] = 9125, -- lever
	[10054] = 10055, [10055] = 10054, -- wall lamp
	[10056] = 10057, [10057] = 10056, -- wall lamp
	[10082] = 10083, [10083] = 10082, -- dragon statue (lamp)
	[10084] = 10085, [10085] = 10084, -- dragon statue (lamp)
	[10959] = 10960, [10960] = 10959, -- dragon basin
	[11039] = 11040, [11040] = 11039, -- torch
	[11041] = 11042, [11042] = 11041, -- torch
	[11043] = 11044, [11044] = 11043, -- dragon statue (lamp)
	[11045] = 11046, [11046] = 11045, -- dragon statue (lamp)
	[11050] = 11048, [11048] = 11050, -- jade basin
	[11051] = 11052, [11052] = 11051, -- jade basin
	[11058] = 11060, [11060] = 11058, -- mystic floor lamp
	[11059] = 11061, [11061] = 11059, -- mystic floor lamp
	[11235] = 11236, [11236] = 11235, -- wall torch
	[11244] = 11245, [11245] = 11244, -- wall torch
	[11252] = 11253, [11253] = 11252, -- wall lamp
	[11254] = 11255, [11255] = 11254, -- wall lamp
	[12431] = 12432, [12432] = 12431, -- lamp
	[12433] = 12434, [12434] = 12433, -- lamp
	[17409] = 17410, [17410] = 17409, -- street lamp (venore)
	[17411] = 17412, [17412] = 17411, -- street lamp (venore)
	[20497] = 20498, [20498] = 20497, -- street lamp (oramond)
	[20499] = 20500, [20500] = 20499, -- street lamp (oramond)
	[20501] = 20502, [20502] = 20501, -- candle (oramond)
	[20503] = 20504, [20504] = 20503, -- candle (oramond)
	[21125] = 21129, [21129] = 21125, -- lever (red)
	[21126] = 21130, [21130] = 21126, -- lever (yellow)
	[21127] = 21131, [21131] = 21127, -- lever (green)
	[21128] = 21132, [21132] = 21128, -- lever (violet)
	[23434] = 23436, [23436] = 23434, -- predator lamp
	[23435] = 23437, [23437] = 23435, -- predator lamp
	[23440] = 23438, [23438] = 23440, -- protectress lamp
	[23441] = 23439, [23439] = 23441, -- protectress lamp
	[35973] = 35974, [35974] = 35973, -- toggle light on podium
}

local transformTo = Action()

function transformTo.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local transformIds = transformItems[item:getId()]
	if not transformIds then
		return false
	end

	item:transform(transformIds)
	return true
end

for i, v in pairs(transformItems) do
	transformTo:id(i)
end

transformTo:register()
