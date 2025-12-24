local openOddDoors = {
	[11708] = { locked = 12250, closed = 11705 },
	[11716] = { locked = 12249, closed = 11714 },
	[13137] = { locked = 13136, closed = 13135 },
	[13144] = { locked = 13143, closed = 13142 },
	[23877] = { locked = 23874, closed = 23873 },
	[23878] = { locked = 23876, closed = 23875 },
	[28520] = { locked = 28519, closed = 24903 },
	[28368] = { locked = 28365, closed = 28364 },
	[28369] = { locked = 28367, closed = 28366 },
	[28885] = { locked = 28658, closed = 28658 },
	[28886] = { locked = 28659, closed = 28659 },
	[30035] = { locked = 30049, closed = 30033 },
	[30036] = { locked = 30050, closed = 30034 },
	[30039] = { locked = 30051, closed = 30037 },
	[30040] = { locked = 30052, closed = 30038 },
	[30776] = { locked = 30773, closed = 30772 },
	[30777] = { locked = 30775, closed = 30774 },
	[30837] = { locked = 30834, closed = 30833 },
	[30838] = { locked = 30836, closed = 30835 },
	[31496] = { locked = 31494, closed = 31494 },
	[31497] = { locked = 31495, closed = 31495 },
	[33636] = { locked = 33633, closed = 33632 },
	[33637] = { locked = 33635, closed = 33634 },
	[34940] = { locked = 34937, closed = 34936 },
	[34941] = { locked = 34939, closed = 34938 },
	[36463] = { locked = 36460, closed = 36459 },
	[36464] = { locked = 36462, closed = 36461 }
}
local closedOddDoors = {
	[11705] = { locked = 12250, open = 11708 },
	[11714] = { locked = 12249, open = 11716 },
	[13135] = { locked = 13136, open = 13137 },
	[13142] = { locked = 13143, open = 13144 },
	[23873] = { locked = 23874, open = 23877 },
	[23875] = { locked = 23876, open = 23878 },
	[24903] = { locked = 28519, open = 28520 },
	[28364] = { locked = 28365, open = 28368 },
	[28366] = { locked = 28367, open = 28369 },
	[28658] = { locked = 28658, open = 28885 },
	[28659] = { locked = 28659, open = 28886 },
	[30033] = { locked = 30049, open = 30035 },
	[30034] = { locked = 30050, open = 30036 },
	[30037] = { locked = 30051, open = 30039 },
	[30038] = { locked = 30052, open = 30040 },
	[30772] = { locked = 30773, open = 30776 },
	[30774] = { locked = 30775, open = 30777 },
	[30833] = { locked = 30834, open = 30837 },
	[30835] = { locked = 30836, open = 30838 },
	[31494] = { locked = 31494, open = 31496 },
	[31495] = { locked = 31495, open = 31497 },
	[33632] = { locked = 33633, open = 33636 },
	[33634] = { locked = 33635, open = 33637 },
	[34936] = { locked = 34937, open = 34940 },
	[34938] = { locked = 34939, open = 34941 },
	[36459] = { locked = 36460, open = 36463 },
	[36461] = { locked = 36462, open = 36464 }
}
local lockedOddDoors = {
	[12250] = { closed = 11705, open = 11708 },
	[12249] = { closed = 11714, open = 11716 },
	[13136] = { closed = 13135, open = 13137 },
	[13143] = { closed = 13142, open = 13144 },
	[23874] = { closed = 23873, open = 23877 },
	[23876] = { closed = 23875, open = 23878 },
	[28519] = { closed = 24903, open = 28520 },
	[28365] = { closed = 28364, open = 28368 },
	[28367] = { closed = 28366, open = 28369 },
	[30049] = { closed = 30033, open = 30035 },
	[30050] = { closed = 30034, open = 30036 },
	[30051] = { closed = 30037, open = 30039 },
	[30052] = { closed = 30038, open = 30040 },
	[30773] = { closed = 30772, open = 30776 },
	[30775] = { closed = 30774, open = 30777 },
	[30834] = { closed = 30833, open = 30837 },
	[30836] = { closed = 30835, open = 30838 },
	[33633] = { closed = 33632, open = 33636 },
	[33635] = { closed = 33634, open = 33637 },
	[34937] = { closed = 34936, open = 34940 },
	[34939] = { closed = 34938, open = 34941 },
	[36460] = { closed = 36459, open = 36463 },
	[36462] = { closed = 36461, open = 36464 }
}

local positionOffsets = {
	{x = 1, y = 0}, -- east
	{x = 0, y = 1}, -- south
	{x = -1, y = 0}, -- west
	{x = 0, y = -1}, -- north
}

--[[
When closing a door with a creature in it findPushPosition will find the most appropriate
adjacent position following a prioritization order.
The function returns the position of the first tile that fulfills all the checks in a round.
The function loops trough east -> south -> west -> north on each following line in that order.
In round 1 it checks if there's an unhindered walkable tile without any creature.
In round 2 it checks if there's a tile with a creature.
In round 3 it checks if there's a tile blocked by a movable tile-blocking item.
In round 4 it checks if there's a tile blocked by a magic wall or wild growth.
]]
local function findPushPosition(creature, round)
	local pos = creature:getPosition()
	for _, offset in ipairs(positionOffsets) do
		local offsetPosition = Position(pos.x + offset.x, pos.y + offset.y, pos.z)
		local tile = Tile(offsetPosition)
		if tile then
			local creatureCount = tile:getCreatureCount()
			if round == 1 then
				if tile:queryAdd(creature) == RETURNVALUE_NOERROR and creatureCount == 0 then
					if not tile:hasFlag(TILESTATE_PROTECTIONZONE) or (tile:hasFlag(TILESTATE_PROTECTIONZONE) and creature:canAccessPz()) then
						return offsetPosition
					end
				end
			elseif round == 2 then
				if creatureCount > 0 then
					if not tile:hasFlag(TILESTATE_PROTECTIONZONE) or (tile:hasFlag(TILESTATE_PROTECTIONZONE) and creature:canAccessPz()) then
						return offsetPosition
					end
				end
			elseif round == 3 then
				local topItem = tile:getTopDownItem()
				if topItem then
					if topItem:getType():isMovable() then
						return offsetPosition
					end
				end
			else
				if tile:getItemById(ITEM_MAGICWALL) or tile:getItemById(ITEM_WILDGROWTH) then
					return offsetPosition
				end
			end
		end
	end
	if round < 4 then
		return findPushPosition(creature, round + 1)
	end
end

local door = Action()

function door.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local itemId = item:getId()
	local transformTo = 0
	if table.contains(closedQuestDoors, itemId) then
		if player:getStorageValue(item.actionid) ~= -1 or player:getGroup():getAccess() then
			item:transform(itemId + 1)
			player:teleportTo(toPosition, true)
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The door seems to be sealed against unwanted intruders.")
		end
		return true
	elseif table.contains(closedLevelDoors, itemId) then
		if item.actionid > 0 and player:getLevel() >= item.actionid - actionIds.levelDoor or player:getGroup():getAccess() then
			item:transform(itemId + 1)
			player:teleportTo(toPosition, true)
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
		end
		return true
	elseif table.contains(keys, itemId) then
		local tile = Tile(toPosition)
		if not tile then
			return false
		end
		target = tile:getTopVisibleThing()
		if target.actionid == 0 then
			return false
		end
		if table.contains(keys, target.itemid) then
			return false
		end
		if not table.contains(openDoors, target.itemid) and not table.contains(closedDoors, target.itemid) and not table.contains(lockedDoors, target.itemid) then
			return false
		end
		if item.actionid ~= target.actionid then
			player:sendTextMessage(MESSAGE_STATUS_SMALL, "The key does not match.")
			return true
		end
		if lockedOddDoors[target.itemid] then
			transformTo = lockedOddDoors[target.itemid].open
		else
			transformTo = target.itemid + 2
		end
		if table.contains(openDoors, target.itemid) then
			if openOddDoors[target.itemid] then
				transformTo = openOddDoors[target.itemid].locked
			else
				transformTo = target.itemid - 2
			end
		elseif table.contains(closedDoors, target.itemid) then
			if closedOddDoors[target.itemid] then
				transformTo = closedOddDoors[target.itemid].locked
			else
				transformTo = target.itemid - 1
			end
		end
		target:transform(transformTo)
		return true
	elseif table.contains(lockedDoors, itemId) then
		player:sendTextMessage(MESSAGE_INFO_DESCR, "It is locked.")
		return true
	elseif table.contains(openDoors, itemId) or table.contains(openExtraDoors, itemId) or table.contains(openHouseDoors, itemId) then
		local creaturePositionTable = {}
		local doorCreatures = Tile(toPosition):getCreatures()
		if doorCreatures and #doorCreatures > 0 then
			for _, doorCreature in pairs(doorCreatures) do
				local pushPosition = findPushPosition(doorCreature, 1)
				if not pushPosition then
					player:sendCancelMessage(RETURNVALUE_NOTENOUGHROOM)
					return true
				end
				table.insert(creaturePositionTable, {creature = doorCreature, position = pushPosition})
			end
			for _, tableCreature in ipairs(creaturePositionTable) do
				tableCreature.creature:teleportTo(tableCreature.position, true)
			end
		end
		if openOddDoors[itemId] then
			transformTo = openOddDoors[itemId].closed
		else
			transformTo = itemId - 1
		end
		item:transform(transformTo)
		return true
	elseif table.contains(closedDoors, itemId) or table.contains(closedExtraDoors, itemId) or table.contains(closedHouseDoors, itemId) then
		if closedOddDoors[itemId] then
			transformTo = closedOddDoors[itemId].open
		else
			transformTo = itemId + 1
		end
		item:transform(transformTo)
		return true
	end
	return false
end

local doorTables = {keys, openDoors, closedDoors, lockedDoors, openExtraDoors, closedExtraDoors, openHouseDoors, closedHouseDoors, closedQuestDoors, closedLevelDoors}
for _, doors in pairs(doorTables) do
	for _, doorId in pairs(doors) do
		door:id(doorId)
	end
end
door:register()
