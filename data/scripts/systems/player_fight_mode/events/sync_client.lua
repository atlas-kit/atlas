local event = Event()

event.onPlayerFightingModesChanged = function(self, mode, chase, secure)
	self:sendFightingModes()
end

event:register()
