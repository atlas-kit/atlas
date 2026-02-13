local event = Event()

event.onPlayerFightModesChanged = function(self, mode, chase, secure)
	local targetCreature = self:getTarget()
	if not targetCreature then
		return
	end

	if self:isChasingEnabled() then
		local chaseCreature = self:getFollowCreature()
		if not chaseCreature then
			self:setFollowCreature(targetCreature)
		end
	else
		self:setFollowCreature(nil)
		self:stopWalk()
	end
end

event:register()
