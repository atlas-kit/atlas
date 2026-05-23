-- player_lost_chase_on_remove.lua
-- Cleans up the player's chase target when they are removed from the
-- game world (logout, kick). Ensures the chased creature's follower
-- list is properly updated.

local event = Event()

-- When a player is removed (logout, kick), clear their chase target
-- to update the followed creature's follower list.
function event.onCreatureRemove(creature)
	-- Only apply to players.
	local player = creature:asPlayer()
	if not player then
		return
	end

	-- Clear the chase target so the followed creature no longer tracks
	-- this player as a follower.
	if player:getChaseCreature() then
		player:setChaseCreature(nil)
	end
end

event:register()
