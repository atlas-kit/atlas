function Player.sendCancelTarget(self)
	local msg = NetworkMessage()
	msg:addByte(0xA3)
	msg:addU32(0x00)
	msg:sendToPlayer(self)
	msg:delete()
	return true
end
