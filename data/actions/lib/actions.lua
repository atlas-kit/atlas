local wildGrowth = {3013, 2130} -- wild growth destroyable by machete
local jungleGrass = { -- grass destroyable by machete
	[427] = 2878,
	[4118] = 2936,
	[13000] = 14353
}
local groundIds = {354, 355} -- pick usable ground
local sandIds = {231, 231} -- desert sand
local holeId = { -- usable rope holes, for rope spots see global.lua
	294, 369, 370, 387, 394, 414, 415, 416, 437, 438, 439, 440, 7881, 594, 595, 615,
	609, 610, 615, 1401, 2724, 615, 608, 859, 4815, 868, 1082, 4813, 157, 158,
	159, 160, 162, 163, 164, 369, 370, 414, 416, 437, 438, 684, 1212,
	653, 671, 683, 684, 4438, 7862, 7881, 11247, 11974, 15894, 6767, 19025,
	20993
}
local holes = {593, 606, 608, 21341} -- holes opened by shovel
local fruits = {3584, 3585, 3586, 3587, 3588, 3589, 3590, 3591, 3592, 3593, 3595, 3596, 5096, 8011, 8012, 8013} -- fruits to make decorated cake with knife

function destroyItem(player, target, toPosition)
	if type(target) ~= "userdata" or not target:isItem() then
		return false
	end

	if target:hasAttribute(ITEM_ATTRIBUTE_UNIQUEID) or target:hasAttribute(ITEM_ATTRIBUTE_ACTIONID) then
		return false
	end

	if toPosition.x == CONTAINER_POSITION then
		player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	local destroyId = ItemType(target.itemid):getDestroyId()
	if destroyId == 0 then
		return false
	end

	if math.random(7) == 1 then
		local item = Game.createItem(destroyId, 1, toPosition)
		if item then
			item:decay()
		end

		-- Move items outside the container
		if target:isContainer() then
			for i = target:getSize() - 1, 0, -1 do
				local containerItem = target:getItem(i)
				if containerItem then
					containerItem:moveTo(toPosition)
				end
			end
		end

		target:remove(1)
	end

	toPosition:sendMagicEffect(CONST_ME_POFF)
	return true
end

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
	local targetId = target.itemid
	if not targetId then
		return true
	end

	if table.contains(wildGrowth, targetId) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		target:remove()
		return true
	end

	local grass = jungleGrass[targetId]
	if grass then
		target:transform(grass)
		target:decay()
		player:addAchievementProgress("Nothing Can Stop Me", 100)
		return true
	end

	return destroyItem(player, target, toPosition)
end

function onUsePick(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid == 9393 then -- shiny stone refining
		local chance = math.random(1, 100)
		if chance == 1 then
			player:addItem(ITEM_CRYSTAL_COIN) -- 1% chance of getting crystal coin
		elseif chance <= 6 then
			player:addItem(ITEM_GOLD_COIN) -- 5% chance of getting gold coin
		elseif chance <= 51 then
			player:addItem(ITEM_PLATINUM_COIN) -- 45% chance of getting platinum coin
		else
			player:addItem(3945) -- 49% chance of getting small diamond
		end
		player:addAchievementProgress("Petrologist", 100)
		target:getPosition():sendMagicEffect(CONST_ME_BLOCKHIT)
		target:remove(1)
		return true
	end

	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()
	if not ground then
		return false
	end

	if table.contains(groundIds, ground.itemid) and ground.actionid == actionIds.pickHole then
		ground:transform(394)
		ground:decay()
		toPosition:sendMagicEffect(CONST_ME_POFF)

		toPosition.z = toPosition.z + 1
		tile:relocateTo(toPosition)
		return true
	end

	-- Ice fishing hole
	if ground.itemid == 7200 then
		ground:transform(7236)
		ground:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
		return true
	end

	return false
end

function onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()

	if ground and table.contains(ropeSpots, ground:getId()) or tile:getItemById(8240) then
		tile = Tile(toPosition:moveUpstairs())
		if not tile then
			return false
		end

		if tile:hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
			player:sendCancelMessage(RETURNVALUE_PLAYERISPZLOCKED)
			return true
		end

		player:teleportTo(toPosition, false)
		return true
	end

	if table.contains(holeId, target.itemid) then
		toPosition.z = toPosition.z + 1
		tile = Tile(toPosition)
		if not tile then
			return false
		end

		local thing = tile:getTopVisibleThing()
		if not thing then
			return true
		end

		if thing:isPlayer() then
			if Tile(toPosition:moveUpstairs()):queryAdd(thing) ~= RETURNVALUE_NOERROR then
				return false
			end

			return thing:teleportTo(toPosition, false)
		elseif thing:isItem() and thing:getType():isMovable() then
			return thing:moveTo(toPosition:moveUpstairs())
		end

		return true
	end

	return false
end

function onUseShovel(player, item, fromPosition, target, toPosition, isHotkey)
	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()
	if not ground then
		return false
	end

	local groundId = ground:getId()
	if table.contains(holes, groundId) then
		ground:transform(groundId + 1)
		ground:decay()

		toPosition.z = toPosition.z + 1
		tile:relocateTo(toPosition)
		player:addAchievementProgress("The Undertaker", 500)
	elseif target.itemid == 867 then -- large hole
		target:transform(868)
		target:decay()
		player:addAchievementProgress("The Undertaker", 500)
	elseif target.itemid == 15418 then -- swamp digging
		if (player:getStorageValue(PlayerStorageKeys.swampDigging)) <= os.time() then
			local chance = math.random(100)
			if chance >= 1 and chance <= 42 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a dead snake.")
				player:addItem(4389)
			elseif chance >= 43 and chance <= 79 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a small diamond.")
				player:addItem(3945)
			elseif chance >= 80 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You dug up a leech.")
				player:addItem(13883)
			end
			player:setStorageValue(PlayerStorageKeys.swampDigging, os.time() + 7 * 24 * 60 * 60)
			player:getPosition():sendMagicEffect(CONST_ME_GREEN_RINGS)
		end
	elseif table.contains(sandIds, groundId) then
		local randomValue = math.random(1, 100)
		if target.actionid == actionIds.sandHole and randomValue <= 20 then
			ground:transform(615)
			ground:decay()
		elseif randomValue == 1 then
			Game.createItem(3959, 1, toPosition)
			player:addAchievementProgress("Gold Digger", 100)
		elseif randomValue > 95 then
			Game.createMonster("Scarab", toPosition)
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
	else
		return false
	end

	return true
end

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({2550, 10513}, item.itemid) then
		return false
	end

	if target.itemid == 1897 then -- wheat
		target:transform(3651)
		target:decay()
		Game.createItem(2636, 1, toPosition) -- bunch of wheat
		player:addAchievementProgress("Happy Farmer", 200)
		return true
	end
	if target.itemid == 5463 then -- burning sugar cane
		target:transform(5463)
		target:decay()
		Game.createItem(5464, 1, toPosition) -- bunch of sugar cane
		player:addAchievementProgress("Natural Sweetener", 50)
		return true
	end
	return destroyItem(player, target, toPosition)
end

function onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({2416, 10515}, item.itemid) then
		return false
	end

	return destroyItem(player, target, toPosition)
end

function onUseKitchenKnife(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({2566, 10511, 10515}, item.itemid) then
		return false
	end

	if table.contains(fruits, target.itemid) and player:removeItem(6278, 1) then
		target:remove(1)
		player:addItem(6276, 1)
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_GREEN)
		return true
	end

	return false
end
