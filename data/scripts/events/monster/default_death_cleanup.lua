local event = Event()

event.onCreatureDeath = function(self, corpse, killer, mostDamageKiller, lastHitUnjustified, mostDamageUnjustified)
	if not self:isMonster() then
		return
	end

	self:setTarget(nil)

	for _, summon in ipairs(self:getSummons()) do
		summon:changeHealth(-summon:getHealth())
		summon:removeMaster()
	end
	self:clearSummons()

	self:clearTargetList()
	self:clearFriendList()
end

-- Runs last so any other onCreatureDeath handler still observes the dying
-- monster's summons, target and friend lists intact, matching the original
-- engine order where this cleanup happened after the death event.
event:register(math.huge)
