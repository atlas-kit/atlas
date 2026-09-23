local event = Event()

event.onPlayerLogout = function(self)
	local playerId = self:getId()
	if nextUseStaminaTime[playerId] then
		nextUseStaminaTime[playerId] = nil
	end

	return true
end

event:register()
