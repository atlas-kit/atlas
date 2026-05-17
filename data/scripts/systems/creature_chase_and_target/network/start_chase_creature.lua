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

    local position = player:getPosition()
    local chasePosition = chaseCreature:getPosition()

    if position.z ~= chasePosition.z or not player:canSee(chasePosition) then
        player:setTargetCreature(nil)
        player:setChaseCreature(nil)
        player:sendCancelTarget()
        player:sendCancelMessage(RETURNVALUE_THEREISNOWAY)
        player:stopWalk()
        return
    end

    local targetCreature = player:getTargetCreature()
    if targetCreature and targetCreature ~= chaseCreature then
        player:setTargetCreature(nil)
    end

    player:setChaseCreature(chaseCreature)
end

handler:register()
