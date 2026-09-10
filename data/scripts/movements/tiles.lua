local increasing = {[419] = 420, [431] = 430, [452] = 453, [563] = 564, [549] = 562, [10145] = 10146}
local decreasing = {[420] = 419, [430] = 431, [453] = 452, [564] = 563, [562] = 549, [10146] = 10145}

do
	local event = MoveEvent()

	function event.onStepIn(creature, item, position, fromPosition)
		if not increasing[item.itemid] then
			return true
		end

		if not creature:isPlayer() or creature:isInGhostMode() then
			return true
		end

		item:transform(increasing[item.itemid])

		if item.actionid >= actionIds.levelDoor then
			if creature:getLevel() < item.actionid - actionIds.levelDoor then
				creature:teleportTo(fromPosition, false)
				position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
				creature:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The tile seems to be protected against unwanted intruders.")
			end
			return true
		end

		if Tile(position):hasFlag(TILESTATE_PROTECTIONZONE) then
			local lookPosition = creature:getPosition()
			lookPosition:getNextPosition(creature:getDirection())
			local depotItem = Tile(lookPosition):getItemByType(ITEM_TYPE_DEPOT)
			if depotItem then
				local depotItems = creature:getDepotChest(getDepotId(depotItem:getUniqueId()), true):getItemHoldingCount()
				creature:sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your depot contains " .. depotItems .. " item" .. (depotItems ~= 1 and "s." or "."))
				creature:addAchievementProgress("Safely Stored Away", 1000)
				creature:setSpecialContainersAvailable(true)
				return true
			end
		end

		if item.actionid ~= 0 and creature:getStorageValue(item.actionid) <= 0 then
			creature:teleportTo(fromPosition, false)
			position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
			creature:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The tile seems to be protected against unwanted intruders.")
			return true
		end
		return true
	end

	event:id(unpack(table.keys(increasing)))
	event:register()
end

do
	local event = MoveEvent()

	function event.onStepOut(creature, item, position, fromPosition)
		if not decreasing[item.itemid] then
			return true
		end

		local isPlayer = creature:isPlayer()
		if isPlayer then
			creature:setSpecialContainersAvailable(false)
		end

		if isPlayer and creature:isInGhostMode() then
			return true
		end

		item:transform(decreasing[item.itemid])
		return true
	end

	event:id(unpack(table.keys(decreasing)))
	event:register()
end
