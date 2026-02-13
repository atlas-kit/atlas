local event = Event()

function event.onCreatureTargetCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	local targetCreature = player:getTargetCreature()
	if not targetCreature then
		return
	end

	player:setChaseCreature(player:hasSecureMode() and targetCreature or nil)
end

event:register()
