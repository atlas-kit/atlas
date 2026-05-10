-- CONDITION_OUTFIT replaces the visible outfit. If it is applied while mounted,
-- clear the stored mounted outfit too so the player does not remount when it ends.
local event = Event()

function event.onCreatureChangeOutfit(self, outfit)
    if not self:isPlayer() or self:isChangingMountOutfit() then
        return true
    end

    if self:isMounted() and outfit.lookMount == 0 then
        self:dismount()
    end

    return true
end

event:register()
