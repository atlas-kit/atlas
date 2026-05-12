-- Restores the mount speed bonus for players who log in with a mounted outfit.
local event = Event()

function event.onPlayerLogin(player)
    if player:isMounted() then
        local outfit = player:getDefaultOutfit()
        local lookMount = outfit.lookMount

        if player:canRideMount(lookMount) then
            if not player:getCurrentMount() then
                player:setCurrentMount(lookMount)
            end
            player:restoreMountSpeed()
            player:setWasMounted(true)
        else
            outfit.lookMount = 0
            player:setDefaultOutfit(outfit)
            player:setCurrentOutfit(outfit)
            player:setWasMounted(false)
        end
    end

    return true
end

event:register()
