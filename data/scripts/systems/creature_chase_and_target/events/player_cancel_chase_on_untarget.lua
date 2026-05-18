-- player_cancel_chase_on_untarget.lua
-- Clears the player's chase/follow target when the attack target is
-- lost or cleared. Prevents the player from continuing to follow a
-- creature they are no longer attacking.

local event = Event()

-- When the player loses their attack target, also clear chase if
-- they were following the target.
function event.onCreatureTargetCreatureChanged(creature)
	local player = creature:asPlayer()
	if not player then
		return
	end

	-- If the player still has a target, no action needed.
	local targetCreature = player:getTargetCreature()
	if targetCreature then
		return
	end

	-- Target was cleared; also stop following if we were following them.
	local chaseCreature = player:getChaseCreature()
	if chaseCreature then
		player:setChaseCreature(nil)
	end
end

event:register()
