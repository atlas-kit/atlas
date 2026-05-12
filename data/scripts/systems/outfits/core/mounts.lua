-- Mount ownership, selection, and toggling.
--
-- Persistent state:
-- - PlayerStorageKeys.mountsBase + lookType: mount ownership
-- - PlayerStorageKeys.currentMount: selected mount lookType (used when mounting)
-- - PlayerStorageKeys.randomizeMount: whether to select a random owned mount when mounting
--
-- Session-only state:
-- - lastMountToggle[playerId]: used for Outfits.ToggleMountCooldown
-- - wasMounted[playerId]: remembers intent while forcibly dismounted in protection zones
do
    local lastMountToggle = {}
    function Player.getLastMountToggle(self)
        return lastMountToggle[self:getId()] or 0
    end

    function Player.setLastMountToggle(self, time)
        lastMountToggle[self:getId()] = time
    end
end

do
    local wasMounted = {}
    function Player.getWasMounted(self)
        return wasMounted[self:getId()] or false
    end

    function Player.setWasMounted(self, mounted)
        wasMounted[self:getId()] = mounted or nil
    end
end

do
    local changingMountOutfit = {}
    function Player.isChangingMountOutfit(self)
        return changingMountOutfit[self:getId()] or false
    end

    function Player.setChangingMountOutfit(self, changing)
        changingMountOutfit[self:getId()] = changing or nil
    end
end

function Player.syncMountSpeed(self, oldLookMount, newLookMount)
    oldLookMount = oldLookMount or 0
    newLookMount = newLookMount or 0

    if oldLookMount == newLookMount then
        return
    end

    local oldMount = Game.getMountByLookType(oldLookMount)
    if oldMount ~= nil then
        self:changeSpeed(-oldMount.speed)
    end

    local newMount = Game.getMountByLookType(newLookMount)
    if newMount ~= nil then
        self:changeSpeed(newMount.speed)
    end
end

function Player.setOutfitWithMountSpeed(self, outfit)
    local oldLookMount = self:getOutfit().lookMount

    self:setChangingMountOutfit(true)
    local result = self:setOutfit(outfit)
    self:setChangingMountOutfit(false)

    if result then
        self:syncMountSpeed(oldLookMount, outfit.lookMount)
    end

    return result
end

function Player.restoreMountSpeed(self)
    local lookMount = self:getOutfit().lookMount
    local mount = Game.getMountByLookType(lookMount)
    if mount ~= nil then
        self:changeSpeed(mount.speed)
    end
end

function Player.addMount(self, mountId)
    if type(mountId) ~= "number" or mountId <= 0 or not Game.getMountByLookType(mountId) then
        return false
    end

    return self:setStorageValue(PlayerStorageKeys.mountsBase + mountId, 1)
end

function Player.addAllMounts(self)
    local mounts = Game.getMounts()
    for _, mount in ipairs(mounts) do
        self:addMount(mount.lookType)
    end
end

function Player.hasMount(self, mountId)
    if type(mountId) ~= "number" or mountId <= 0 then
        return false
    end

    local value = self:getStorageValue(PlayerStorageKeys.mountsBase + mountId)
    return value ~= nil and value ~= -1
end

function Player.removeMount(self, mountId)
    if type(mountId) ~= "number" or mountId <= 0 then
        return false
    end

    local value = self:removeStorageValue(PlayerStorageKeys.mountsBase + mountId)
    if self:getCurrentMount() == mountId and self:isMounted() then
        self:dismount()
    end
    return value
end

function Player.removeAllMounts(self)
    local mounts = Game.getMounts()
    for _, mount in ipairs(mounts) do
        self:removeMount(mount.lookType)
    end

    if self:isMounted() then
        self:dismount()
    end
end

-- Returns the selected mount lookType stored in PlayerStorageKeys.currentMount, or nil if unset.
function Player.getCurrentMount(self)
    local value = self:getStorageValue(PlayerStorageKeys.currentMount)
    if value == nil or value == -1 then
        return nil
    end
    return value
end

-- Sets the selected mount lookType (nil clears). For non-staff players, the mount must be owned.
function Player.setCurrentMount(self, mountId)
    if mountId == nil then
        return self:removeStorageValue(PlayerStorageKeys.currentMount)
    end

    if type(mountId) ~= "number" or mountId <= 0 then
        return false
    end

    local mount = Game.getMountByLookType(mountId)
    if not mount then
        return false
    end

    if not self:getGroup():getAccess() and not self:canRideMount(mount.lookType) then
        return false
    end

    return self:setStorageValue(PlayerStorageKeys.currentMount, mount.lookType)
end

function Player.getRandomizeMount(self)
    local randomizeMount = self:getStorageValue(PlayerStorageKeys.randomizeMount)
    return randomizeMount ~= nil and randomizeMount ~= -1
end

function Player.setRandomizeMount(self, randomize)
    if randomize then
        return self:setStorageValue(PlayerStorageKeys.randomizeMount, 1)
    end
    return self:removeStorageValue(PlayerStorageKeys.randomizeMount)
end

-- Returns whether the player can ride the given mount lookType.
function Player.canRideMount(self, mountId)
    if self:getGroup():getAccess() then
        return type(mountId) == "number" and mountId > 0 and Game.getMountByLookType(mountId) ~= nil
    end

    if type(mountId) ~= "number" or mountId <= 0 then
        return false
    end

    local mount = Game.getMountByLookType(mountId)
    if not mount then
        return false
    end

    if mount.premium and not self:isPremium() then
        return false
    end

    return self:hasMount(mount.lookType)
end

function Player.isMounted(self)
    return self:getOutfit().lookMount ~= 0
end

-- Mounts the given mount object (from Game.getMountByLookType): updates outfit + speed.
function Player.mount(self, mount)
    if type(mount) ~= "table" or type(mount.lookType) ~= "number" or type(mount.speed) ~= "number" then
        return false
    end

    if not self:canRideMount(mount.lookType) then
        return false
    end

    local outfit = self:getDefaultOutfit()
    outfit.lookMount = mount.lookType
    local result = self:setOutfitWithMountSpeed(outfit)
    if result then
        self:setWasMounted(true)
    end

    return result
end

-- Dismounts the current mount: clears outfit mount + removes speed bonus.
function Player.dismount(self, keepWasMounted)
    local outfit = self:getDefaultOutfit()
    outfit.lookMount = 0
    local result = self:setOutfitWithMountSpeed(outfit)
    if result and not keepWasMounted then
        self:setWasMounted(false)
    end

    return result
end

local function getRandomMount(player)
    local mounts = Game.getMounts()

    local availableMounts = {}
    for _, mount in ipairs(mounts) do
        if player:hasMount(mount.lookType) then
            table.insert(availableMounts, mount.lookType)
        end
    end

    if #availableMounts == 0 then
        return nil
    end

    local idx = math.random(1, #availableMounts)
    return availableMounts[idx]
end

-- player:toggleMount(mounted)
-- Behavior:
-- - When mounted is true: mounts using the selected mount (or a random owned mount if randomize is enabled).
-- - When mounted is false: dismounts.
-- Enforces cooldown when mounting, protection-zone restriction, premium/ownership rules, and CONDITION_OUTFIT.
function Player.toggleMount(self, mounted, keepWasMounted)
    if mounted then
        if not self:getGroup():getAccess() and not self:getWasMounted() then
            local lastMountToggle = self:getLastMountToggle()
            if os.mtime() - lastMountToggle < Outfits.ToggleMountCooldown then
                return false
            end
        end

        if self:isMounted() then
            return false
        end

        local tile = self:getTile()
        if not self:getGroup():getAccess() and tile:hasFlag(TILESTATE_PROTECTIONZONE) then
            self:sendCancelMessage(RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE)
            return false
        end

        local lookMount = self:getCurrentMount()
        if not lookMount then
            self:sendOutfitWindow()
            return false
        end

        if self:getRandomizeMount() then
            lookMount = getRandomMount(self)
            if not lookMount then
                self:sendOutfitWindow()
                return false
            end
        end

        local currentMount = Game.getMountByLookType(lookMount)
        if not currentMount then
            return false
        end

        if not self:getGroup():getAccess() and currentMount.premium and not self:isPremium() then
            self:sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
            return false
        end

        if self:hasCondition(CONDITION_OUTFIT) then
            self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
            return false
        end

        if not self:mount(currentMount) then
            return false
        end
    else
        if not self:isMounted() then
            return false
        end

        if not self:dismount(keepWasMounted) then
            return false
        end
    end

    self:setLastMountToggle(os.mtime())
    return true
end

-- Deprecated helper; mount lookType is already the identifier.
function Game.getMountIdByLookType(lookType)
    print("Warning: Game.getMountIdByLookType is deprecated. Mounts are now identified by client ID.")
    return lookType
end
