-- Player.sendCancelTarget(self)
-- Sends packet 0xA3 to the client, instructing it to clear the red
-- attack square from the UI. This is called server-side whenever the
-- player loses their target (death, teleport, zone change, etc.).
function Player.sendCancelTarget(self)
	local msg = NetworkMessage()
	msg:addByte(0xA3)
	msg:addU32(0x00)
	msg:sendToPlayer(self)
	msg:delete()
	return true
end
