local condition = Condition(CONDITION_INFIGHT, CONDITIONID_DEFAULT)	

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

	if player:hasFlag(PlayerFlag_NotGainInFight) then
		return
	end
	
	condition:setTicks(configManager.getNumber(configKeys.PZ_LOCKED))
	player:addCondition(condition)
end

event:register()
