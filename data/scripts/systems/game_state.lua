-- Game-state plumbing previously living inside Game::setGameState() and
-- Game::reload() in src/game.cpp.
--
-- What stays in C++:
--   * The state-machine guard inside Game::setGameState (gameState transitions,
--     dispatcher/scheduler shutdown, saveGameState bookkeeping).
--   * The INIT bootstrap (Groups::load, Chat::load, Map::spawns::startup) — these
--     load native XML/spawn structures and have no Lua API; the onStartup event
--     fires *after* them so scripts already have the canonical hook point.
--   * Game::cleanup() — internal decay-wheel maintenance run from checkDecay()
--     and from shutdown(); not safe to drive from Lua.
--   * The subsystem reload entry points themselves (Actions, Chat, Items,
--     Monsters, MoveEvents, Npcs, Spells, TalkActions, Weapons, Scripts) —
--     exposed as Game.reload<Subsystem>() so this file can compose them.

-- math.huge as triggerIndex forces these hooks to sort last in the
-- onGameSave/onGameShutdown chains. The previous C++ implementation ran the
-- full event chain and *then* kicked, so any third-party callback (no matter
-- its triggerIndex) used to run before the kick. Using math.huge preserves
-- that contract: every other subscriber finishes first.
local KICK_TRIGGER_INDEX = math.huge

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

	event:register(KICK_TRIGGER_INDEX)
end

do
	-- Server shutdown: kick every online player before the state is saved.
	local event = Event()

	event.onGameShutdown = function()
		for _, player in ipairs(Game.getPlayers()) do
			player:remove()
		end
	end

	event:register(KICK_TRIGGER_INDEX)
end

do
	-- Reload routing: maps each RELOAD_TYPE_X to the C++ subsystem binding that
	-- knows how to rebuild it. Mirrors the switch that used to live inside
	-- Game::reload(); RELOAD_TYPE_ALL and RELOAD_TYPE_QUESTS (and any unknown
	-- type) intentionally fall through to Game.reloadAll(), which preserves the
	-- original default-branch sweep order. The previous C++ switch had no case
	-- for QUESTS either, so /reload quests has always meant "do a full sweep";
	-- the per-type quest clear is handled by the /reload talkaction itself.
	local handlers = {
		[RELOAD_TYPE_ACTIONS] = Game.reloadActions,
		[RELOAD_TYPE_CHAT] = Game.reloadChat,
		[RELOAD_TYPE_CONFIG] = Game.reloadConfig,
		[RELOAD_TYPE_EVENTS] = Game.reloadEvents,
		[RELOAD_TYPE_GLOBAL] = Game.reloadGlobal,
		[RELOAD_TYPE_ITEMS] = Game.reloadItems,
		[RELOAD_TYPE_MONSTERS] = Game.reloadMonsters,
		[RELOAD_TYPE_MOVEMENTS] = Game.reloadMovements,
		[RELOAD_TYPE_NPCS] = Game.reloadNpcs,
		[RELOAD_TYPE_SCRIPTS] = Game.reloadScripts,
		[RELOAD_TYPE_SPELLS] = Game.reloadSpells,
		[RELOAD_TYPE_TALKACTIONS] = Game.reloadTalkActions,
		[RELOAD_TYPE_WEAPONS] = Game.reloadWeapons,
	}

	function Game.reload(reloadType)
		local handler = handlers[reloadType]
		if handler then
			return handler()
		end
		return Game.reloadAll()
	end
end
