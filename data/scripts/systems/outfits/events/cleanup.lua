-- Clears per-session mount bookkeeping on logout.
--
-- These values are not persisted and should not be carried between sessions.
local event = Event()

function event.onPlayerLogout(self)
    self:setLastMountToggle(nil)
    self:setWasMounted(nil)
    self:setChangingMountOutfit(nil)
    return true
end

event:register()
