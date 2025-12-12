local handler = PacketHandler(0xCF)

function handler.onReceive(player)
	local blessings = Game.getBlessings()

	local msg = NetworkMessage()

	msg:addByte(0x9B)
	msg:addByte(SERVER_BLESSINGS_COUNT)

	for i = 1, SERVER_BLESSINGS_COUNT do
		msg:addU16(blessings[i])
		msg:addByte(player:hasBlessing(i) and 1 or 0)
		msg:addByte(0) -- store bless
	end

	local premiumReduction = 0
	if player:isPremium() and player:isPromoted() then
		premiumReduction = 30
	end

	local isPromoted = player:isPromoted()
	msg:addByte(isPromoted and 0x01 or 0x00)
	msg:addByte(premiumReduction) -- exp loss lower

	local lossPercents = player:getLossPercent()
	local expLost = premiumReduction + lossPercents.skills
	msg:addByte(expLost) -- exp skill loss min pvp death
	msg:addByte(expLost) -- exp skill loss max pvp death
	msg:addByte(expLost) -- exp skill pve death

	local hasSkull = player:getSkull() == SKULL_RED or player:getSkull() == SKULL_BLACK
	local containerLossPercent = lossPercents.container
	if hasSkull then
		containerLossPercent = 100
	end

	msg:addByte(containerLossPercent)
	msg:addByte(containerLossPercent)
	msg:addByte(hasSkull and 0x01 or 0x00)

	local amulet = player:getSlotItem(CONST_SLOT_NECKLACE)
	if amulet and amulet:getId() == ITEM_AMULETOFLOSS then
		msg:addByte(0x01)
	else
		msg:addByte(0x00)
	end

	-- History
	local historyAmount = 1
	msg:addByte(historyAmount)

	for i = 1, historyAmount do
		msg:addU32(os.time())
		msg:addByte(0) -- color message (0 - red | 1 = white)
		msg:addString("Blessing Purchased")
	end

	msg:sendToPlayer(player)
	msg:delete()
end

handler:register()
