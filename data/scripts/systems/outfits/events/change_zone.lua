-- Handles auto dismount/remount when crossing protection zones.
--
-- The flag wasMounted is used to remember the player's intent to stay mounted while
-- forcibly dismounted in protection zones, and to bypass the normal toggle cooldown
-- for these forced transitions.
local event = Event()

function event.onCreatureChangeZone(self, fromZone, toZone)
    if not self:isPlayer() then
        return
    end

    if toZone == ZONE_PROTECTION and not self:getGroup():getAccess() and self:isMounted() then
        if self:toggleMount(false, true) then
            self:setWasMounted(true)
        end
    elseif fromZone == ZONE_PROTECTION and self:getWasMounted() then
        if self:toggleMount(true, true) then
            self:setWasMounted(false)
        end
    end
end

event:register()
