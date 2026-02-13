local event = Event()

event.onPlayerFightModeChanged = function(self, stance, chase, secure)
	self:sendFightMode()
end

event:register()
