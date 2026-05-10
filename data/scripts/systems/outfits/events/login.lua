-- Restores the mount speed bonus for players who log in with a mounted outfit.
local event = Event()

function event.onPlayerLogin(player)
    if player:isMounted() then
        player:restoreMountSpeed()
    end

    return true
end

event:register()
