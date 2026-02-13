local event = Event()

event.onPlayerFightModesChanged = function(self, mode, chase, secure)
	self:sendFightModes()
end

event:register()
