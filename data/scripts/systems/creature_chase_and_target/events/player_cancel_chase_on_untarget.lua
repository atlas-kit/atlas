-- player_cancel_chase_on_untarget.lua
local event = Event()

-- Triggered whenever a creature's target changes.
-- Target-loss scripts clear chase explicitly when the lost target is also the chase creature.
function event.onCreatureTargetCreatureChanged(creature)
end

event:register()
