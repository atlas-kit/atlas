-- Creature.hasTargetCreature(self)
-- Returns true if the creature currently has an attack target set.
function Creature.hasTargetCreature(self)
	return self:getTargetCreature() ~= nil
end

-- Creature.hasChaseCreature(self)
-- Returns true if the creature is currently chasing/following another creature.
function Creature.hasChaseCreature(self)
	return self:getChaseCreature() ~= nil
end

-- Creature.hasAggressiveCondition(self)
-- Returns true if the creature has at least one active aggressive
-- condition (fire, poison, energy, etc.).
function Creature.hasAggressiveCondition(self)
	for _, cond in ipairs(self:getConditions()) do
		if cond:isAggressive() then
			return true
		end
	end
	return false
end
