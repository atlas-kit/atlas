-- Creature.canTarget(self, creature)
-- Checks whether a given creature can be selected as an attack target.
-- Returns false if the creature is removed, not attackable, inside a
-- protection zone, out of sight, or on a different floor level.
function Creature.canTarget(self, creature)
	if creature:isRemoved() then
		return false
	end

	if not creature:isAttackable() then
		return false
	end

	if creature:getZone() == ZONE_PROTECTION then
		return false
	end

	if not self:canSeeCreature(creature) then
		return false
	end

	return self:getPosition().z == creature:getPosition().z
end
