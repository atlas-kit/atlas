local KICK_AFTER_MINUTES = 15

local event = CreatureEvent("Idle Timeout")

function event.onThink(player, interval)
	if player:getGroup():getAccess() then
		return
	end

	local tile = player:getTile()
	if not tile or tile:hasFlag(TILESTATE_NOLOGOUT) then
		return
	end

	player:setIdleTime(player:getIdleTime() + interval)

	local idleTime = player:getIdleTime()
	if idleTime > (KICK_AFTER_MINUTES * 60000) + 60000 then
		player:remove()
	elseif player:hasClient() and (idleTime == 60000 * KICK_AFTER_MINUTES) then
		player:sendTextMessage(MESSAGE_STATUS_WARNING, "There was no variation in your behaviour for " .. KICK_AFTER_MINUTES .. " minutes. You will be disconnected in one minute if there is no change in your actions until then.");
	end
end

event:register()
