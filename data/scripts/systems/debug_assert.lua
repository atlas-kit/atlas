do
	local event = Event()

	event.onPlayerLogout = function(self)
		Game.removeDebugAssert(self:getGuid())
	end

	event:register()
end
