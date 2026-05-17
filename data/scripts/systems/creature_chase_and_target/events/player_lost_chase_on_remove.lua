-- player_lost_chase_on_remove.lua
-- Cancels the player's chase target when a creature is removed from the
-- game world (death, logout, teleport), but only if the player has a
-- pending walk action. Prevents the visual "sliding" toward a now-
-- removed entity.

local event = Event()

-- Triggered when a creature is removed from the game world (death, logout, or teleport).
function event.onCreatureRemoved(creature)  
	-- Ensure the entity being handled is a player.
	local player = creature:asPlayer()
	if not player then
		return
	end

	-- If the player does not have an active target, there is no chase to cancel.
	local targetCreature = player:getTargetCreature()
	if not targetCreature then
		return
	end

	-- Check if the player has a pending movement (next walk).
	-- If they aren't trying to move, we don't need to force a chase cancellation here.
	if not player:hasNextWalk() then
		return
	end
	
	-- Stop the player's chase movement.
	-- This prevents the player from "sliding" or walking toward a creature that was just removed.
	player:setChaseCreature(nil)
end

event:register()
