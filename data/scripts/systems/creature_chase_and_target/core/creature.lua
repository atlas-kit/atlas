function Creature.hasTargetCreature(self)
	return self:getTargetCreature() ~= nil
end

function Creature.hasChaseCreature(self)
	return self:getChaseCreature() ~= nil
end
