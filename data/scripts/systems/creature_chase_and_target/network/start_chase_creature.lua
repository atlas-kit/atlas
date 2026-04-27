local handler = PacketHandler(0xA2)

function handler.onReceive(player, msg)
    local chaseCreatureId = msg:getU32()

    if chaseCreatureId == 0 then
        player:setChaseCreature(nil)
        return
    end

    local chaseCreature = Creature(chaseCreatureId)
    if not chaseCreature then
        return
    end

    player:setChaseCreature(chaseCreature)
end

handler:register()
