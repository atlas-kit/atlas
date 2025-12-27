function Game:onStartup()
	if Event.onGameStartup then
		Event.onGameStartup(self)
	end
end

function Game:onShutdown()
	if Event.onGameShutdown then
		Event.onGameShutdown(self)
	end
end

function Game:onSave()
	if Event.onGameSave then
		Event.onGameSave(self)
	end
end
