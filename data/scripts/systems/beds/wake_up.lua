local event = Event()

local function findRelevantBed(player)
    local pos = player:getPosition()
    local tile = Tile(pos)
    if not tile then
        return nil, nil
    end

    local headboard = tile:getHeadboard()
    if headboard then
        local footboard = headboard:getPartnerBed()
        if footboard and footboard:isFootboard() then
            return headboard, footboard
        end
    end

    local directions = {DIRECTION_NORTH, DIRECTION_EAST, DIRECTION_SOUTH, DIRECTION_WEST}
    local checkPos = Position(pos)
    for _, dir in ipairs(directions) do
        checkPos.x = pos.x
        checkPos.y = pos.y
        checkPos.z = pos.z
        checkPos:getNextPosition(dir)
        local adjacentTile = Tile(checkPos)
        if adjacentTile then
            local adjacentHeadboard = adjacentTile:getHeadboard()
            if adjacentHeadboard then
                local footboard = adjacentHeadboard:getPartnerBed()
                if footboard and footboard:isFootboard() then
                    return adjacentHeadboard, footboard
                end
            end
        end
    end

    return nil, nil
end

local function regeneratePlayer(player, sleptSeconds)
    if sleptSeconds <= 0 then
        return
    end

    local regenWindow = math.min(sleptSeconds, Beds.HealthManaMaxSeconds)
    local regenTicks = math.floor(regenWindow / Beds.HealthManaTickSeconds) * Beds.HealthManaPerTick
    if regenTicks > 0 then
        player:addHealth(regenTicks)
        player:addMana(regenTicks)
    end

    local soulTicks = math.floor(sleptSeconds / Beds.SoulTickSeconds) * Beds.SoulPerTick
    if soulTicks > 0 then
        player:addSoul(soulTicks)
    end
end

function event.onPlayerLogin(player)
    local headboard, footboard = findRelevantBed(player)
    if not headboard or not footboard or headboard:getSleeper() ~= player:getGuid() then
        return true
    end

    local lastLogout = player:getLastLogout()
    local sleptSeconds = lastLogout > 0 and math.max(0, os.time() - lastLogout) or 0

    regeneratePlayer(player, sleptSeconds)

    headboard:removeSleeper()
    footboard:removeSleeper()
    return true
end

event:register()
