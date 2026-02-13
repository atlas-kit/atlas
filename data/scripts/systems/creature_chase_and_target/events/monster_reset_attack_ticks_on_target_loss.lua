local event = Event()

function event.onCreatureTargetCreatureChanged(creature)
	local monster = creature:asMonster()
	if not monster then
		return
	end

	if not monster:hasTargetCreature() then
		monster:resetAttackTicks()
	end
end

event:register()
