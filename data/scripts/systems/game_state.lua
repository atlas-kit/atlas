-- Player-kicking on server close/shutdown. The C++ engine still drives the
-- state machine, thread lifecycle and saveGameState; only the player-facing
-- logic lives here.
--
-- triggerIndex 1 forces these to run after the default (index 0) save
-- subscribers (e.g. account storage persistence), preserving the exact
-- ordering the engine used before this logic moved to Lua:
--   onSave subscribers -> kick -> C++ saveGameState

do
	-- Server closed: kick everyone without the always-login flag.
	-- onSave also fires on shutdown and on manual saves (SIGUSR1, saveServer),
	-- so the state gate isolates the close transition exactly like the engine did.
	local event = Event()

	event.onGameSave = function()
		if Game.getGameState() ~= GAME_STATE_CLOSED then
			return
		end

		for _, player in ipairs(Game.getPlayers()) do
			if not player:hasFlag(PlayerFlag_CanAlwaysLogin) then
				player:remove()
			end
		end
	end

	event:register(1)
end

do
	-- Server shutdown: kick every online player before the state is saved.
	local event = Event()

	event.onGameShutdown = function()
		for _, player in ipairs(Game.getPlayers()) do
			player:remove()
		end
	end

	event:register(1)
end
