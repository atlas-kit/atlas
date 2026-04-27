local handler = PacketHandler(0xA1)

function handler.onReceive(player, msg)
    local targetCreatureId = msg:getU32()

    if targetCreatureId == 0 then
        player:setTargetCreature(nil)
        return
    end

    local targetCreature = Creature(targetCreatureId)
    if not targetCreature then
        player:sendCancelTarget()
        return
    end

    local position = player:getPosition()
    local targetPosition = targetCreature:getPosition()

    if position.z ~= targetPosition.z or not player:canSee(targetPosition) then
        player:sendCancelTarget()
        return
    end

    local returnValue = Combat.canTargetCreature(player, targetCreature)
    if returnValue ~= RETURNVALUE_NOERROR then
        player:sendCancelMessage(returnValue)
        player:sendCancelTarget()
        player:setTargetCreature(nil)
        return
    end

    player:setTargetCreature(targetCreature)
end

handler:register()
