-- Outfit and podium window builders.
--
-- This file builds outgoing packets:
-- - 0xC8: Outfit Window (Player.sendOutfitWindow)
-- - 0xD8: Podium Window (Player.sendPodiumWindow)
local function canWearOutfit(player, outfit)
    if not outfit.unlocked then
        return player:hasOutfit(outfit.lookType)
    end

    return not outfit.premium or player:isPremium()
end

local function getAvailableOutfits(player)
    local availableOutfits = {}

    local isAccessPlayer = player:getGroup():getAccess()
    if isAccessPlayer then
        -- Add GM outfit for staff members.
        local gamemaster = {
            name = "Gamemaster",
            lookType = 75,
            addons = 0
        }
        table.insert(availableOutfits, gamemaster)
    end

    local outfits = Game.getOutfits(player:getSex())
    for _, outfit in ipairs(outfits) do
        if isAccessPlayer then
            table.insert(availableOutfits, {
                name = outfit.name,
                lookType = outfit.lookType,
                addons = 3
            })
        elseif canWearOutfit(player, outfit) then
            local addons = player:getOutfitAddons(outfit.lookType)
            table.insert(availableOutfits, {
                name = outfit.name,
                lookType = outfit.lookType,
                addons = addons
            })
        end
    end

    return availableOutfits
end

local function getAvailableMounts(player)
    local mounts = Game.getMounts()

    local isAccessPlayer = player:getGroup():getAccess()

    local availableMounts = {}
    for _, mount in ipairs(mounts) do
        if isAccessPlayer or player:hasMount(mount.lookType) then
            table.insert(availableMounts, {
                lookType = mount.lookType,
                name = mount.name
            })
        end
    end
    return availableMounts
end

local function getAvailableFamiliars(player)
    return {}
end

-- player:sendOutfitWindow()
-- Builds and sends packet 0xC8 (Outfit Window).
-- Structure:
-- - 0xC8
-- - currentOutfit via msg:addOutfit
-- - if lookMount == 0: four mount color bytes (head/body/legs/feet)
-- - currentFamiliarLookType:u16 (sent as 0)
-- - outfits: count:u16 then {lookType:u16, name:string, addons:byte, mode:byte}
-- - mounts: count:u16 then {lookType:u16, name:string, mode:byte}
-- - familiars: count:u16 (currently 0)
-- - tryOutfitMode:byte (0)
-- - mounted:bool (uses getWasMounted to preserve intent while forcibly dismounted in PZ)
-- - randomizeMount:bool
function Player.sendOutfitWindow(self)
    local availableOutfits = getAvailableOutfits(self)
    if #availableOutfits == 0 then
        self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
        return
    end

    local currentOutfit = self:getDefaultOutfit()
    if currentOutfit.lookType == 0 then
        currentOutfit.lookType = availableOutfits[1].lookType
    end

    -- If the player was forcibly dismounted (e.g. PZ), keep the checkbox state coherent.
    local mounted = self:isMounted() or self:getWasMounted()

    local availableMounts = getAvailableMounts(self)
    local availableFamiliars = getAvailableFamiliars(self)

    local msg = NetworkMessage()
    msg:addByte(0xC8)

    msg:addOutfit(currentOutfit)

    msg:addU16(currentOutfit.lookMount)
    msg:addByte(currentOutfit.lookMountHead)
    msg:addByte(currentOutfit.lookMountBody)
    msg:addByte(currentOutfit.lookMountLegs)
    msg:addByte(currentOutfit.lookMountFeet)

    msg:addU16(0) -- current familiar looktype

    msg:addU16(#availableOutfits)
    for _, outfit in ipairs(availableOutfits) do
        msg:addU16(outfit.lookType)
        msg:addString(outfit.name)
        msg:addByte(outfit.addons)
        msg:addByte(0) -- mode: 0x00 - available, 0x01 store (requires U32 store offerId), 0x02 golden outfit tooltip (hardcoded)
    end

    msg:addU16(#availableMounts)
    for _, mount in ipairs(availableMounts) do
        msg:addU16(mount.lookType)
        msg:addString(mount.name)
        msg:addByte(0) -- mode: 0x00 - available, 0x01 store (requires U32 store offerlookType)
    end

    msg:addU16(#availableFamiliars)
    for _, familiar in ipairs(availableFamiliars) do
        msg:addU16(familiar.lookType)
        msg:addString(familiar.name)
        msg:addByte(0) -- mode: 0x00 - available, 0x01 store (requires U32 store offerId)
    end

    msg:addByte(0x00) -- try outfit mode (?)
    msg:addBool(mounted)
    msg:addBool(self:getRandomizeMount())
    msg:sendToPlayer(self)
    msg:delete()
end

-- player:sendPodiumWindow(item)
-- Builds and sends packet 0xD8 (Podium Window).
-- Structure:
-- - 0xD8
-- - currentPodiumOutfit via msg:addOutfit
-- - currentMount block: lookMount:u16 + four mount color bytes
-- - currentFamiliarLookType:u16 (0)
-- - outfits list (same layout as 0xC8)
-- - mounts list (same layout as 0xC8)
-- - familiar count:u16 (0)
-- - windowMode:byte (5)
-- - showMountCheckbox:bool
-- - unknown:u16 (0)
-- - position:Position + itemClientId:u16 + stackpos:byte
-- - showPlatform:bool + outfitCheckbox:bool (ignored by client) + direction:byte
function Player.sendPodiumWindow(self, item)
    local podium = item:getPodium()
    if not podium then
        return
    end

    local tile = item:getTile()
    if not tile then
        return
    end

    local it = ItemType(item:getId())
    if not it then
        return
    end

    local availableOutfits = getAvailableOutfits(self)
    if #availableOutfits == 0 then
        self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
        return
    end

    local stackpos = tile:getThingIndex(item)

    local podiumOutfit = podium:getOutfit()
    local playerOutfit = self:getDefaultOutfit()
    local isEmpty = podiumOutfit.lookType == 0 and podiumOutfit.lookMount == 0

    if podiumOutfit.lookType == 0 then
        -- Copy player outfit.
        podiumOutfit.lookType = playerOutfit.lookType
        podiumOutfit.lookHead = playerOutfit.lookHead
        podiumOutfit.lookBody = playerOutfit.lookBody
        podiumOutfit.lookLegs = playerOutfit.lookLegs
        podiumOutfit.lookFeet = playerOutfit.lookFeet
        podiumOutfit.lookAddons = playerOutfit.lookAddons
    end

    if podiumOutfit.lookMount == 0 then
        -- Copy player mount.
        podiumOutfit.lookMount = playerOutfit.lookMount
        podiumOutfit.lookMountHead = playerOutfit.lookMountHead
        podiumOutfit.lookMountBody = playerOutfit.lookMountBody
        podiumOutfit.lookMountLegs = playerOutfit.lookMountLegs
        podiumOutfit.lookMountFeet = playerOutfit.lookMountFeet
    end

    if not self:canWearOutfit(podiumOutfit.lookType) then
        -- select first outfit available when the one from podium is not unlocked
        podiumOutfit.lookType = availableOutfits[1].lookType
    end

    local availableMounts = getAvailableMounts(self)

    local msg = NetworkMessage()
    msg:addByte(0xD8)

    -- current outfit
    msg:addOutfit(podiumOutfit)

    -- current mount
    msg:addU16(podiumOutfit.lookMount)
    msg:addByte(podiumOutfit.lookMountHead)
    msg:addByte(podiumOutfit.lookMountBody)
    msg:addByte(podiumOutfit.lookMountLegs)
    msg:addByte(podiumOutfit.lookMountFeet)

    -- current familiar (not used in podium)
    msg:addU16(0)

    -- available outfits
    msg:addU16(#availableOutfits)
    for _, outfit in ipairs(availableOutfits) do
        msg:addU16(outfit.lookType)
        msg:addString(outfit.name)
        msg:addByte(outfit.addons)
        msg:addByte(0) -- mode: 0x00 - available, 0x01 store (requires U32 store offerId), 0x02 golden outfit tooltip (hardcoded)
    end

    -- available mounts
    msg:addU16(#availableMounts)
    for _, mount in ipairs(availableMounts) do
        msg:addU16(mount.lookType)
        msg:addString(mount.name)
        msg:addByte(0) -- mode: 0x00 - available, 0x01 store (requires U32 store offerId)
    end

    -- available familiars (not used in podium)
    msg:addU16(0)

    msg:addByte(0x05) -- "set outfit" window mode (5 = podium)
    msg:addBool((isEmpty and playerOutfit.lookMount ~= 0) or podium:hasFlag(PODIUM_SHOW_MOUNT)) -- "mount" checkbox
    msg:addU16(0) -- unknown
    msg:addPosition(item:getPosition())
    msg:addU16(it:getClientId())
    msg:addByte(stackpos)

    msg:addBool(podium:hasFlag(PODIUM_SHOW_PLATFORM)) -- is platform visible
    msg:addBool(true) -- "outfit" checkbox, ignored by the client
    msg:addByte(podium:getDirection())
    msg:sendToPlayer(self)
    msg:delete()
end
