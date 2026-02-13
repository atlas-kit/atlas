local event = Event()

function event.onCreatureThink(creature, interval)
	local monster = creature:asMonster()
	if not monster then
		return
	end

	if monster:isIdle() then
		return
	end

	local master = monster:getMaster()
	if not master then
		return
	end

	local position = monster:getPosition()
	if not monster:isInSpawnRange(position) then
		return
	end

	local chaseCreature = monster:getChaseCreature()

	local targetCreature = monster:getTargetCreature()
	if not targetCreature then
		local masterTargetCreature = master:getTargetCreature()
		if masterTargetCreature then
			-- this happens if the monster is summoned during combat.
			monster:selectTarget(masterTargetCreature)
		elseif master ~= chaseCreature then
			-- our master has not ordered us to attack anything, lets follow him around instead.
			monster:setChaseCreature(master)
		end
	elseif targetCreature == monster then
		monster:setChaseCreature(nil)
	elseif targetCreature ~= chaseCreature then
		-- this happens just after a master orders an attack, so lets follow it as well.
		monster:setChaseCreature(targetCreature)
	end
end

event:register()
