local handler = PacketHandler(0xBE)

function handler.onReceive(player, msg)
    player:setChaseCreature(nil)
    player:setTargetCreature(nil)
end

handler:register()
