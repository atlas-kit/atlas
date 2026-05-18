-- 0xA2: Follow / Start Chase (client -> server)
-- Payload:
-- - creatureId:u32 (0 to stop following)
--
-- Validates line-of-sight before starting. If the target is on a
-- different floor or out of sight, the request is rejected.
-- If already attacking a different creature, the attack target is
-- cleared so the player only follows the new chase target.
local handler = PacketHandler(0xA2)

-- Process the follow request: validate the target, set chase or reject.
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
