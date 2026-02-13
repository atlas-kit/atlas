local event = Event()

function event.onCreatureTargetCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	if not player:hasTargetCreature() then
		player:stopWalk()
	end
end

event:register()
