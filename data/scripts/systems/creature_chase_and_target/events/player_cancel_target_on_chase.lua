local event = Event()

function event.onCreatureChaseCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	if player:hasTargetCreature() then
		player:setTargetCreature(nil)
	end
end

event:register()
