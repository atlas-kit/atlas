-- 0xA1: Attack / Set Target (client -> server)
-- Payload:
-- - creatureId:u32 (0 to stop attacking)
--
-- Validates line-of-sight and combat rules (via Combat.canTargetCreature)
-- before accepting the target. If the target is unreachable or the attack
-- is not allowed (e.g., protection zone, wrong PVP mode), the request
-- is rejected and both chase and target are cleared.
local handler = PacketHandler(0xA1)

function handler.onReceive(player, msg)
	local targetCreatureId = msg:getU32()

	if targetCreatureId == 0 then
		player:setChaseCreature(nil)
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
		player:setChaseCreature(nil)
		player:setTargetCreature(nil)
		return
	end

	player:setTargetCreature(targetCreature)
end

handler:register()
