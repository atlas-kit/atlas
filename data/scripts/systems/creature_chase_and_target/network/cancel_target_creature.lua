local handler = PacketHandler(0xBE)

function handler.onReceive(player, msg)
	player:setTargetCreature(nil)	
end

handler:register()
