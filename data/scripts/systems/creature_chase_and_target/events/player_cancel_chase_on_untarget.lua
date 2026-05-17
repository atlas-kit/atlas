-- player_cancel_chase_on_untarget.lua
local event = Event()

function event.onCreatureTargetCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	local targetCreature = player:getTargetCreature()
	if targetCreature then
		return
	end

	local chaseCreature = player:getChaseCreature()
	if chaseCreature then
		player:setChaseCreature(nil)
	end
end

event:register()
