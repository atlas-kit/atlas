local foods = {
	[3250] = {5, "Crunch."}, -- carrot
	[3577] = {15, "Munch."}, -- meat
	[3578] = {12, "Munch."}, -- fish
	[3579] = {10, "Mmmm."}, -- salmon
	[3580] = {17, "Munch."}, -- northern pike
	[3581] = {4, "Gulp."}, -- shrimp
	[3582] = {30, "Chomp."}, -- ham
	[3583] = {60, "Chomp."}, -- dragon ham
	[3584] = {5, "Yum."}, -- pear
	[3585] = {6, "Yum."}, -- red apple
	[3586] = {13, "Yum."}, -- orange
	[2488] = {8, "Yum."}, -- banana
	[2489] = {1, "Yum."}, -- blueberry
	[2490] = {18, "Slurp."}, -- coconut
	[2491] = {1, "Yum."}, -- cherry
	[2492] = {2, "Yum."}, -- strawberry
	[2493] = {9, "Yum."}, -- grapes
	[2494] = {20, "Yum."}, -- melon
	[2495] = {17, "Munch."}, -- pumpkin
	[695] = {5, "Crunch."}, -- carrot
	[2497] = {6, "Munch."}, -- tomato
	[115] = {9, "Crunch."}, -- corncob
	[2499] = {2, "Crunch."}, -- cookie
	[2500] = {2, "Munch."}, -- candy cane
	[2501] = {10, "Crunch."}, -- bread
	[2502] = {3, "Crunch."}, -- roll
	[4943] = {8, "Crunch."}, -- brown bread
	[2637] = {6, "Gulp."}, -- egg
	[2638] = {9, "Smack."}, -- cheese
	[2969] = {9, "Munch."}, -- white mushroom
	[2970] = {4, "Munch."}, -- red mushroom
	[2971] = {22, "Munch."}, -- brown mushroom
	[2972] = {30, "Munch."}, -- orange mushroom
	[2973] = {9, "Munch."}, -- wood mushroom
	[2974] = {6, "Munch."}, -- dark mushroom
	[2975] = {12, "Munch."}, -- some mushrooms
	[2976] = {3, "Munch."}, -- some mushrooms
	[2977] = {36, "Munch."}, -- fire mushroom
	[2978] = {5, "Munch."}, -- green mushroom
	[5094] = {4, "Yum."}, -- mango
	[5678] = {8, "Gulp."}, -- tortoise egg
	[6125] = {8, "Gulp."}, -- tortoise egg from Nargor
	[6275] = {10, "Mmmm."}, -- cake
	[6276] = {15, "Mmmm."}, -- decorated cake
	[6390] = {12, "Mmmm."}, -- valentine's cake
	[6391] = {15, "Mmmm."}, -- cream cake
	[6498] = {20, "Mmmm."}, -- gingerbread man
	[6541] = {6, "Gulp."}, -- coloured egg (yellow)
	[6542] = {6, "Gulp."}, -- coloured egg (red)
	[6543] = {6, "Gulp."}, -- coloured egg (blue)
	[6544] = {6, "Gulp."}, -- coloured egg (green)
	[6545] = {6, "Gulp."}, -- coloured egg (purple)
	[6569] = {1, "Mmmm."}, -- candy
	[6574] = {5, "Mmmm."}, -- bar of chocolate
	[7158] = {15, "Munch."}, -- rainbow trout
	[7159] = {13, "Munch."}, -- green perch
	[229] = {2, "Yum."}, -- ice cream cone (crispy chocolate chips)
	[7373] = {2, "Yum."}, -- ice cream cone (velvet vanilla)
	[7374] = {2, "Yum."}, -- ice cream cone (sweet strawberry)
	[7375] = {2, "Yum."}, -- ice cream cone (chilly cherry)
	[7376] = {2, "Yum."}, -- ice cream cone (mellow melon)
	[7377] = {2, "Yum."}, -- ice cream cone (blue-barian)
	[1227] = {4, "Crunch."}, -- walnut
	[841] = {4, "Crunch."}, -- peanut
	[1355] = {60, "Munch."}, -- marlin
	[169] = {9, "Urgh."}, -- scarab cheese
	[1293] = {10, "Gulp."}, -- potato
	[1294] = {5, "Yum."}, -- plum
	[1295] = {1, "Yum."}, -- raspberry
	[1296] = {1, "Urgh."}, -- lemon
	[1297] = {7, "Munch."}, -- cucumber
	[1298] = {5, "Crunch."}, -- onion
	[1314] = {1, "Gulp."}, -- jalapeño pepper
	[1332] = {5, "Munch."}, -- beetroot
	[1334] = {11, "Yum."}, -- chocolate cake
	[457] = {7, "Slurp."}, -- yummy gummy worm
	[6554] = {5, "Crunch."}, -- bulb of garlic
	[144] = {0, "Slurp."}, -- banana chocolate shake
	[7793] = {0, "Your head begins to feel better."}, -- headache pill
	[8496] = {15, "Yum."}, -- rice ball
	[8620] = {3, "Urgh."}, -- terramite eggs
	[8386] = {10, "Mmmm."}, -- crocodile steak
	[9586] = {20, "Yum."}, -- pineapple
	[9587] = {10, "Munch."}, -- aubergine
	[9588] = {8, "Crunch."}, -- broccoli
	[9589] = {9, "Crunch."}, -- cauliflower
	[9808] = {55, "Gulp."}, -- ectoplasmic sushi
	[9809] = {18, "Yum."}, -- dragonfruit
	[9810] = {2, "Munch."}, -- peas
	[10437] = {20, "Crunch."}, -- haunch of boar
	[431] = {55, "Munch."}, -- sandfish
	[6554] = {14, "Urgh."}, -- larvae
	[6555] = {15, "Munch."}, -- deepling filet
	[12196] = {60, "Mmmm."}, -- anniversary cake
	[13283] = {33, "Munch."}, -- mushroom pie
	[13314] = {10, "Urgh."}, -- insectoid eggs
	[13777] = {15, "Smack."}, -- soft cheese
	[13778] = {12, "Smack."}, -- rat cheese
	[16546] = {15, "Munch."}, -- glooth sandwich
	[16547] = {7, "Slurp."}, -- bowl of glooth soup
	[16548] = {6, "Burp."}, -- bottle of glooth wine
	[16549] = {25, "Chomp."}, -- glooth steak
	[17571] = {12, "Yum."}, -- prickly pear
	[17573] = {60, "Chomp."}, -- roasted meat
	[18847] = {25, "Mmmm."}, -- energy bar
	[18857] = {15, "Mmmm."}, -- energy drink
	[14650] = {20, "Urgh."}, -- bug meat
	[14651] = {10, "Gulp."}, -- cave turnip
	[14664] = {60, "Mmmm."}, -- birthday cake
	[14736] = {4, "Slurp."}, -- shell
	[19958] = {10, "Slurp."}, -- bottle of Wine
	[20665] = {15, "Mmmmm!"}, -- fresh fruit
	[24886] = {40, "Mmmm."}, -- meringue cake
	[30202] = {15, "Slurp."}, -- winterberry liquor
	[28904] = {40, "Slurp."}, -- goanna meat
	[29413] = {15, "Slurp."}, -- candy floss
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local food = foods[item.itemid]
	if not food then
		return false
	end

	local condition = player:getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT)
	if condition and math.floor(condition:getTicks() / 1000 + (food[1] * 12)) >= 1200 then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "You are full.")
	else
		player:feed(food[1] * 12)
		player:say(food[2], TALKTYPE_MONSTER_SAY)
		player:sendSupplyUsed(item)
		item:remove(1)
	end
	return true
end
