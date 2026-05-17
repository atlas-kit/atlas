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
