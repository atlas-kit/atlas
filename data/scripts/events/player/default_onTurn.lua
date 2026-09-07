local event = Event()

function event.onPlayerTurn(player, direction)
	if player:getDirection() == direction then
		return false
	end

	player:resetIdleTime()
	return true
end

event:register()
