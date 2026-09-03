local handler = PacketHandler(0x8A)

-- 0x8A: Update House Window (client -> server)
-- Payload:
-- - doorId:byte
-- - windowTextId:u32
-- - text:string
function handler.onReceive(player, msg)
	local doorId = msg:getByte()
	local windowTextId = msg:getU32()
	local text = msg:getString()

	local house, internalWindowTextId, internalListId = player:getEditHouse()
	if house and doorId == 0 and internalWindowTextId == windowTextId
			and house:canEditAccessList(internalListId, player) then
		house:setAccessList(internalListId, text)
	end

	player:setEditHouse(nil)
end

handler:register()
