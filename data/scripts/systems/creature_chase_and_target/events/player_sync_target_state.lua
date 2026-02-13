local event = Event()

function event.onCreatureTargetCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	if not player:hasTargetCreature() then
		player:sendCancelTarget()
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.")
	end
end

event:register()
