-- player_lost_chase_on_remove.lua
-- Cleans up the player's chase target when they are removed from the
-- game world (logout, kick). Ensures the chased creature's follower
-- list is properly updated.

local event = Event()

function event.onCreatureRemoved(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	if player:getChaseCreature() then
		player:setChaseCreature(nil)
	end
end

event:register()
