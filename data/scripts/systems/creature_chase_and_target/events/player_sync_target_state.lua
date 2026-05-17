-- player_sync_target_state.lua
-- Synchronizes the server-side target state with the game client.
-- Ensures the red attack square is cleared in the UI when the player
-- loses their target.

local event = Event()

-- Triggered when a player's combat target changes.
-- This ensures the client-side UI stays in sync with the server-side target state.
function event.onCreatureTargetCreatureChanged(creature)
	-- Ensure the entity is a player.
	local player = creature:asPlayer()
	if not player then
		return
	end

	-- If the player no longer has a target (target was lost or cleared):
	if not player:hasTargetCreature() then
		-- Force the game client to clear the red attack square.
		player:sendCancelTarget()
	end
end

event:register()
