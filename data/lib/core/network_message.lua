function NetworkMessage:getBool()
	local value = self:getByte()
	if value > 1 then
		print("[Warning - NetworkMessage::getBool] Invalid boolean value received: " .. value)
	end
	return value ~= 0
end

function NetworkMessage:addBool(value)
	self:addByte(value and 1 or 0)
end

function NetworkMessage:addItemId(itemId)
	local it = ItemType(itemId)
	self:addU16(it:getClientId())
end

function NetworkMessage:addOutfit(outfit)
	self:addU16(outfit.lookType)
	if outfit.lookType ~= 0 then
		self:addByte(outfit.lookHead)
		self:addByte(outfit.lookBody)
		self:addByte(outfit.lookLegs)
		self:addByte(outfit.lookFeet)
		self:addByte(outfit.lookAddons)
	else
		self:addItemId(outfit.lookTypeEx)
	end
end
