local event = Event()

function event.onCreatureChaseCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	if not player:hasChaseCreature() then
		player:stopWalk()
	end
end

event:register()
