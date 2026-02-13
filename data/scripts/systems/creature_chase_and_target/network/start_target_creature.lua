local handler = PacketHandler(0xA1)

function handler.onReceive(player, msg)
	local targetId = msg:getU32()
	if targetId == 0 then
		return
	end

	local targetCreature = Creature(targetId)
	if not targetCreature then
		return
	end

	local position = player:getPosition()
	local targetPosition = targetCreature:getPosition()
	if position.z ~= targetPosition.z or not player:canSee(targetPosition) then
		player:sendCancelTarget()
		return
	end

	-- ReturnValue ret = Combat::canTargetCreature(player, targetCreature);
	-- if (ret != RETURNVALUE_NOERROR) {
	-- 	player->sendCancelMessage(ret);
	-- 	player->sendCancelTarget();
	-- 	player->setTargetCreature(nullptr);
	-- 	return;
	-- }

	player:setTargetCreature(targetCreature)
end

handler:register()
