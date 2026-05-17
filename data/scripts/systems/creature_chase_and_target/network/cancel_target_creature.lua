-- 0xBE: Cancel Target (client -> server)
-- Clears both attack target and chase/follow target.
local handler = PacketHandler(0xBE)

function handler.onReceive(player, msg)
	player:setChaseCreature(nil)
	player:setTargetCreature(nil)
end

handler:register()
