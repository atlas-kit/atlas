local event = Event()

function event.onCreatureRemoved(creature)	
	local player = creature:asPlayer()
	if not player then
		return
	end

	local targetCreature = player:getTargetCreature()
	if not targetCreature then
		return
	end

	if not player:hasNextWalk() then
		return
	end
	
	player:setChaseCreature(nil)
end

event:register()
