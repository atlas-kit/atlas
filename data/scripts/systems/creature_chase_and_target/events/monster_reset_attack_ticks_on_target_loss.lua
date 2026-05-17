-- monster_reset_attack_ticks_on_target_loss.lua
-- Resets the monster's attack cooldown (attack ticks) when the current
-- target is lost or cleared. Prevents the monster from attacking
-- immediately upon re-targeting after losing the previous target.

local event = Event()

-- Triggered whenever the creature's target changes (lost, gained, or swapped).
function event.onCreatureTargetCreatureChanged(creature)
	-- Ensure the logic only applies if the creature is a monster.
	local monster = creature:asMonster()
	if not monster then
		return
	end

	-- Check if the monster currently has no target.
	-- If the target was lost or cleared, we reset the attack cooldowns (ticks).
	if not monster:hasTargetCreature() then
		monster:resetAttackTicks()
	end
end

event:register()
