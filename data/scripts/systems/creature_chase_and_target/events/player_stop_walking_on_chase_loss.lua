-- player_stop_walking_on_chase_loss.lua
-- Forces the player to stop walking when their chase/follow target is
-- lost or cleared. Prevents the player from continuing toward the last
-- known position of a target that no longer exists.

local event = Event()

-- Triggered whenever the player's chase/follow target changes.
function event.onCreatureChaseCreatureChanged(creature)
	-- Verify that the entity is a player.
	local player = creature:asPlayer()
	if not player then
		return
	end

	-- Check if the player has lost or cleared their chase target.
	-- This happens when the target is out of range, changes floor, or the player stops following.
	if not player:hasChaseCreature() then
		-- Force the player character to stop walking immediately.
		-- This prevents the player from continuing to walk toward the last known chase position.
		player:stopWalk()
	end
end

event:register()
