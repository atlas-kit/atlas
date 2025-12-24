local transformItems = {
	[4170] = 4171, [4171] = 4170, -- street lamp
	[568] = 569, [569] = 568, -- table
	[570] = 571, [571] = 570, -- table
	[572] = 573, [573] = 572, -- table
	[574] = 575, [575] = 574, -- table
	[1524] = 1525, [1525] = 1524, -- oven
	[1526] = 1527, [1527] = 1526, -- oven
	[1528] = 1529, [1529] = 1528, -- oven
	[1530] = 1531, [1531] = 1530, -- oven
	[1976] = 1977, [1977] = 1976, -- lever
	[3824] = 3825, [3825] = 3824, -- wall lamp
	[3826] = 3827, [3827] = 3826, -- wall lamp
	[3845] = 3846, [3846] = 3845, -- torch bearer
	[3709] = 3848, [3848] = 3709, -- torch bearer
	[3850] = 3852, [3852] = 3850, -- table lamp
	[3853] = 3854, [3854] = 3853, -- wall lamp
	[3855] = 3856, [3856] = 3855, -- wall lamp
	[3894] = 3895, [3895] = 3894, -- pumpkinhead
	[2209] = 2210, -- trap
	[4113] = 4114, [4114] = 4113, -- sacred statue
	[4114] = 4116, [4116] = 4114, -- sacred statue
	[4111] = 4179, [4179] = 4111, -- bamboo lamp
	[3857] = 3858, [3858] = 3857, -- torch bearer
	[3859] = 3859, [3859] = 3859, -- torch bearer
	[3861] = 3862, [3862] = 3861, -- wall lamp
	[3863] = 3864, [3864] = 3863, -- wall lamp
	[6486] = 6487, [6487] = 6486, -- christmas branch
	[7058] = 7059, [7059] = 7058, -- skull pillar
	[927] = 928, [928] = 927, -- chimney
	[929] = 930, [930] = 929, -- chimney
	[931] = 932, [932] = 931, -- chimney
	[933] = 934, [934] = 933, -- chimney
	[754] = 755, [755] = 754, -- street lamp (yalahar)
	[756] = 757, [757] = 756, -- street lamp (yalahar)
	[758] = 759, [759] = 758, -- street lamp (yalahar)
	[760] = 761, [761] = 760, -- street lamp (yalahar)
	[1042] = 1043, [1043] = 1042, -- wall lamp
	[1044] = 1045, [1045] = 1044, -- wall lamp
	[2547] = 2548, [2548] = 2547, -- lever
	[2549] = 2550, [2550] = 2549, -- lever
	[2560] = 2561, [2561] = 2560, -- wall lamp
	[2562] = 2563, [2563] = 2562, -- wall lamp
	[2564] = 2565, [2565] = 2564, -- wall lamp
	[2566] = 2567, [2567] = 2566, -- wall lamp
	[6841] = 6841, -- crystal pedestal
	[6841] = 6841, -- crystal pedestal
	[6841] = 6841, -- crystal pedestal
	[6841] = 6841, -- crystal pedestal
	[6351] = 6393, [6393] = 6351, -- lever
	[229] = 7385, [7385] = 229, -- lever
	[8219] = 8220, [8220] = 8219, -- wall lamp
	[8221] = 8222, [8222] = 8221, -- wall lamp
	[8247] = 8248, [8248] = 8247, -- dragon statue (lamp)
	[8249] = 8250, [8250] = 8249, -- dragon statue (lamp)
	[9123] = 9124, [9124] = 9123, -- dragon basin
	[9204] = 9205, [9205] = 9204, -- torch
	[9206] = 9207, [9207] = 9206, -- torch
	[9208] = 9209, [9209] = 9208, -- dragon statue (lamp)
	[9210] = 9211, [9211] = 9210, -- dragon statue (lamp)
	[9215] = 9213, [9213] = 9215, -- jade basin
	[9216] = 9217, [9217] = 9216, -- jade basin
	[9223] = 9225, [9225] = 9223, -- mystic floor lamp
	[9224] = 9226, [9226] = 9224, -- mystic floor lamp
	[9401] = 9402, [9402] = 9401, -- wall torch
	[9410] = 9411, [9411] = 9410, -- wall torch
	[9418] = 9419, [9419] = 9418, -- wall lamp
	[9420] = 9421, [9421] = 9420, -- wall lamp
	[10519] = 10520, [10520] = 10519, -- lamp
	[10521] = 10522, [10522] = 10521, -- lamp
	[14402] = 14731, [14731] = 14402, -- street lamp (venore)
	[14732] = 14733, [14733] = 14732, -- street lamp (venore)
	[15699] = 15700, [15700] = 15699, -- street lamp (oramond)
	[15701] = 15703, [15703] = 15701, -- street lamp (oramond)
	[15704] = 15705, [15705] = 15704, -- candle (oramond)
	[15706] = 15707, [15707] = 15706, -- candle (oramond)
	[16528] = 16532, [16532] = 16528, -- lever (red)
	[16529] = 16533, [16533] = 16529, -- lever (yellow)
	[16530] = 16534, [16534] = 16530, -- lever (green)
	[16531] = 16535, [16535] = 16531, -- lever (violet)
	[16785] = 16787, [16787] = 16785, -- predator lamp
	[16786] = 16788, [16788] = 16786, -- predator lamp
	[16791] = 16789, [16789] = 16791, -- protectress lamp
	[16792] = 16790, [16790] = 16792, -- protectress lamp
	[33317] = 35974, [35974] = 33317, -- toggle light on podium
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
