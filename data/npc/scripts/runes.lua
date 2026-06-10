local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)			npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)	end
function onThink()							npcHandler:onThink()						end

local voices = { {text = "Runes, wands, rods, health and mana potions! Have a look!"} }
npcHandler:addModule(VoiceModule:new(voices))

local shopModule = ShopModule:new()
npcHandler:addModule(shopModule)

keywordHandler:addKeyword({'stuff'}, StdModule.say, {npcHandler = npcHandler, text = 'Just ask me for a {trade} to see my offers.'})
keywordHandler:addAliasKeyword({'wares'})
keywordHandler:addAliasKeyword({'offer'})

shopModule:addBuyableItem({'spellbook'}, 3059, 150, 'spellbook')
shopModule:addBuyableItem({'magic lightwand'}, 3046, 400, 'magic lightwand')

shopModule:addBuyableItem({'small health'}, 7876, 20, 1, 'small health potion')
shopModule:addBuyableItem({'health potion'}, 266, 45, 1, 'health potion')
shopModule:addBuyableItem({'mana potion'}, 268, 50, 1, 'mana potion')
shopModule:addBuyableItem({'strong health'}, 236, 100, 1, 'strong health potion')
shopModule:addBuyableItem({'strong mana'}, 237, 80, 1, 'strong mana potion')
shopModule:addBuyableItem({'great health'}, 239, 190, 1, 'great health potion')
shopModule:addBuyableItem({'great mana'}, 238, 120, 1, 'great mana potion')
shopModule:addBuyableItem({'great spirit'}, 7642, 190, 1, 'great spirit potion')
shopModule:addBuyableItem({'ultimate health'}, 7643, 310, 1, 'ultimate health potion')
shopModule:addBuyableItem({'antidote potion'}, 7644, 50, 1, 'antidote potion')

shopModule:addSellableItem({'empty potion flask', 'empty flask'}, 285, 5, 'empty small potion flask')
shopModule:addSellableItem({'strong potion flask', 'strong flask'}, 283, 10, 'empty strong potion flask')
shopModule:addSellableItem({'great potion flask', 'great flask'}, 284, 15, 'empty great potion flask')

shopModule:addBuyableItem({'intense healing'}, 3152, 95, 1, 'intense healing rune')
shopModule:addBuyableItem({'ultimate healing'}, 3160, 175, 1, 'ultimate healing rune')
shopModule:addBuyableItem({'magic wall'}, 3180, 350, 3, 'magic wall rune')
shopModule:addBuyableItem({'destroy field'}, 3148, 45, 3, 'destroy field rune')
shopModule:addBuyableItem({'light magic missile'}, 3174, 40, 10, 'light magic missile rune')
shopModule:addBuyableItem({'heavy magic missile'}, 3198, 120, 10, 'heavy magic missile rune')
shopModule:addBuyableItem({'great fireball'}, 3191, 180, 4, 'great fireball rune')
shopModule:addBuyableItem({'explosion'}, 3200, 250, 6, 'explosion rune')
shopModule:addBuyableItem({'sudden death'}, 3155, 350, 3, 'sudden death rune')
shopModule:addBuyableItem({'paralyze'}, 3165, 700, 1, 'paralyze rune')
shopModule:addBuyableItem({'animate dead'}, 3203, 377, 1, 'animate dead rune')
shopModule:addBuyableItem({'convince creature'}, 3177, 80, 1, 'convince creature rune')
shopModule:addBuyableItem({'chameleon'}, 3178, 210, 1, 'chameleon rune')
shopModule:addBuyableItem({'disintegrate'}, 3197, 80, 3, 'disintegrate rune')

shopModule:addBuyableItemContainer({'bp ap'}, 2869, 7644, 2867, 1, 'backpack of antidote potions')
shopModule:addBuyableItemContainer({'bp slhp'}, 2867, 7876, 400, 1, 'backpack of small health potions')
shopModule:addBuyableItemContainer({'bp hp'}, 2867, 266, 1109, 1, 'backpack of health potions')
shopModule:addBuyableItemContainer({'bp mp'}, 2868, 268, 1000, 1, 'backpack of mana potions')
shopModule:addBuyableItemContainer({'bp shp'}, 2867, 236, 2867, 1, 'backpack of strong health potions')
shopModule:addBuyableItemContainer({'bp smp'}, 2868, 237, 1600, 1, 'backpack of strong mana potions')
shopModule:addBuyableItemContainer({'bp ghp'}, 2867, 239, 2297, 1, 'backpack of great health potions')
shopModule:addBuyableItemContainer({'bp gmp'}, 2868, 238, 2400, 1, 'backpack of great mana potions')
shopModule:addBuyableItemContainer({'bp gsp'}, 2866, 7642, 2297, 1, 'backpack of great spirit potions')
shopModule:addBuyableItemContainer({'bp uhp'}, 2867, 7643, 6199, 1, 'backpack of ultimate health potions')

shopModule:addBuyableItem({'wand of vortex', 'vortex'}, 3074, 500, 'wand of vortex')
shopModule:addBuyableItem({'wand of dragonbreath', 'dragonbreath'}, 3075, 1245, 'wand of dragonbreath')
shopModule:addBuyableItem({'wand of decay', 'decay'}, 3072, 4999, 'wand of decay')
shopModule:addBuyableItem({'wand of draconia', 'draconia'}, 8093, 142, 'wand of draconia')
shopModule:addBuyableItem({'wand of cosmic energy', 'cosmic energy'}, 3073, 9087, 'wand of cosmic energy')
shopModule:addBuyableItem({'wand of inferno', 'inferno'}, 3071, 13526, 'wand of inferno')
shopModule:addBuyableItem({'wand of starstorm', 'starstorm'}, 8092, 15468, 'wand of starstorm')
shopModule:addBuyableItem({'wand of voodoo', 'voodoo'}, 8094, 19666, 'wand of voodoo')

shopModule:addBuyableItem({'snakebite rod', 'snakebite'}, 3066, 500, 'snakebite rod')
shopModule:addBuyableItem({'moonlight rod', 'moonlight'}, 3070, 1245, 'moonlight rod')
shopModule:addBuyableItem({'necrotic rod', 'necrotic'}, 3069, 4999, 'necrotic rod')
shopModule:addBuyableItem({'northwind rod', 'northwind'}, 8083, 142, 'northwind rod')
shopModule:addBuyableItem({'terra rod', 'terra'}, 3065, 9087, 'terra rod')
shopModule:addBuyableItem({'hailstorm rod', 'hailstorm'}, 3067, 13526, 'hailstorm rod')
shopModule:addBuyableItem({'springsprout rod', 'springsprout'}, 8084, 15468, 'springsprout rod')
shopModule:addBuyableItem({'underworld rod', 'underworld'}, 8082, 19666, 'underworld rod')

shopModule:addSellableItem({'wand of vortex', 'vortex'}, 3074, 250, 'wand of vortex')
shopModule:addSellableItem({'wand of dragonbreath', 'dragonbreath'}, 3075, 500, 'wand of dragonbreath')
shopModule:addSellableItem({'wand of decay', 'decay'}, 3072, 3394, 'wand of decay')
shopModule:addSellableItem({'wand of draconia', 'draconia'}, 8093, 2247, 'wand of draconia')
shopModule:addSellableItem({'wand of cosmic energy', 'cosmic energy'}, 3073, 4999, 'wand of cosmic energy')
shopModule:addSellableItem({'wand of inferno', 'inferno'}, 3071, 142, 'wand of inferno')
shopModule:addSellableItem({'wand of starstorm', 'starstorm'}, 8092, 8172, 'wand of starstorm')
shopModule:addSellableItem({'wand of voodoo', 'voodoo'}, 8094, 10083, 'wand of voodoo')

shopModule:addSellableItem({'snakebite rod', 'snakebite'}, 3066, 250, 'snakebite rod')
shopModule:addSellableItem({'moonlight rod', 'moonlight'}, 3070, 500, 'moonlight rod')
shopModule:addSellableItem({'necrotic rod', 'necrotic'}, 3069, 3394, 'necrotic rod')
shopModule:addSellableItem({'northwind rod', 'northwind'}, 8083, 2247, 'northwind rod')
shopModule:addSellableItem({'terra rod', 'terra'}, 3065, 4999, 'terra rod')
shopModule:addSellableItem({'hailstorm rod', 'hailstorm'}, 3067, 142, 'hailstorm rod')
shopModule:addSellableItem({'springsprout rod', 'springsprout'}, 8084, 8172, 'springsprout rod')
shopModule:addSellableItem({'underworld rod', 'underworld'}, 8082, 10083, 'underworld rod')

function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then
		return false
	end

	local player = Player(cid)
	local vocationId = player:getVocation():getId()
	local items = {
		[VOCATION_SORCERER] = 3074,
		[VOCATION_DRUID] = 3066,
		[VOCATION_MASTER_SORCERER] = 3074,
		[VOCATION_ELDER_DRUID] = 3066
	}

	if msgcontains(msg, 'first rod') or msgcontains(msg, 'first wand') then
		if items[vocationId] then
			if not player:getStorageValue(PlayerStorageKeys.firstRod) then
				selfSay('So you ask me for a {' .. ItemType(items[vocationId]):getName() .. '} to begin your adventure?', cid)
				npcHandler.topic[cid] = 1
			else
				selfSay('What? I have already gave you one {' .. ItemType(items[vocationId]):getName() .. '}!', cid)
			end
		else
			selfSay('Sorry, you aren\'t a druid either a sorcerer.', cid)
		end
	elseif msgcontains(msg, 'yes') then
		if npcHandler.topic[cid] == 1 then
			player:addItem(items[vocationId], 1)
			selfSay('Here you are young adept, take care yourself.', cid)
			player:setStorageValue(PlayerStorageKeys.firstRod, 1)
		end
		npcHandler.topic[cid] = 0
	elseif msgcontains(msg, 'no') and npcHandler.topic[cid] == 1 then
		selfSay('Ok then.', cid)
		npcHandler.topic[cid] = 0
	end

	return true
end

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
