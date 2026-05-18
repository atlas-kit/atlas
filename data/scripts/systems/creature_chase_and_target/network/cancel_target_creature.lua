local handler = PacketHandler(0xBE)

-- 0xBE: Cancel Target (client -> server)
-- Clears both attack target and chase/follow target.

function handler.onReceive(player, msg)
	player:setChaseCreature(nil)
	player:setTargetCreature(nil)
end

handler:register()
