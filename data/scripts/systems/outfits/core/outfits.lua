-- Outfit ownership and addon helpers.
--
-- This file manages outfit storage state using PlayerStorageKeys.outfitsBase.
-- Wearability checks (premium/unlocked/addons) are implemented in Player.canWearOutfit.
--
-- Addon representation:
-- - Storage value is a bitmask of unlocked addons for a given lookType.
--   - bit 0 => addon 1
--   - bit 1 => addon 2
-- - Functions that take `addon` expect an addon index (1 or 2), not a bitmask.
-- - Functions that take `addons` expect a bitmask (e.g. 1, 2, or 3).
-- player:addOutfit(lookType)
-- Grants outfit ownership for the given lookType.
function Player.addOutfit(self, lookType)
    local addons = self:getStorageValue(PlayerStorageKeys.outfitsBase + lookType)
    if addons ~= nil and addons ~= -1 then
        return true
    end
    return self:setStorageValue(PlayerStorageKeys.outfitsBase + lookType, 0)
end

-- player:addAllOutfits()
-- Grants all outfits available for the player's sex.
-- This grants ownership only (addon mask = 0); it does not grant addons.
function Player.addAllOutfits(self)
    local outfits = Game.getOutfits(self:getSex())
    for _, outfit in ipairs(outfits) do
        self:addOutfit(outfit.lookType)
    end
end

-- player:addOutfitAddon(lookType, addon)
-- Adds addon 1 or 2 to an already-owned outfit.
--
-- Note: `addon` is an index (1 or 2). Internally it sets the corresponding bit in storage.
function Player.addOutfitAddon(self, lookType, addon)
    local addons = self:getStorageValue(PlayerStorageKeys.outfitsBase + lookType)
    if addons == nil or addons == -1 then
        return false
    end

    addons = addons | (1 << (addon - 1))
    return self:setStorageValue(PlayerStorageKeys.outfitsBase + lookType, addons)
end

-- player:addAddonToAllOutfits(addon)
-- Adds addon 1 or 2 to all outfits for both sexes.
-- This function also grants missing outfits before setting the addon bit.
function Player.addAddonToAllOutfits(self, addon)
    for sex = 0, 1 do
        local outfits = Game.getOutfits(sex)
        for _, outfit in ipairs(outfits) do
            self:addOutfit(outfit.lookType)
            self:addOutfitAddon(outfit.lookType, addon)
        end
    end
end

-- player:getOutfitAddons(lookType) -> number
-- Returns the stored addon bitmask for an owned outfit.
-- Returns 0 when the player does not own the outfit or has no addons.
function Player.getOutfitAddons(self, lookType)
    local outfitAddons = self:getStorageValue(PlayerStorageKeys.outfitsBase + lookType)
    if outfitAddons == nil or outfitAddons == -1 then
        return 0
    end
    return outfitAddons
end

-- player:hasOutfit(lookType) -> boolean
-- Returns true if the player owns the outfit (regardless of addons).
function Player.hasOutfit(self, lookType)
    local addons = self:getStorageValue(PlayerStorageKeys.outfitsBase + lookType)
    return addons ~= nil and addons ~= -1
end

-- player:hasOutfitAddons(lookType, addons) -> boolean
-- Returns true if the player owns the outfit and has all addons in the given bitmask.
-- Example: addons == 3 means addon 1 and addon 2.
function Player.hasOutfitAddons(self, lookType, addons)
    local currentAddons = self:getStorageValue(PlayerStorageKeys.outfitsBase + lookType)
    if currentAddons == nil or currentAddons == -1 then
        return false
    end
    return (currentAddons & addons) == addons
end

-- player:hasOutfitAddon(lookType, addon) -> boolean
-- Returns true if the player has a specific addon (1 or 2) for the outfit.
function Player.hasOutfitAddon(self, lookType, addon)
    local addons = self:getOutfitAddons(lookType)
    return addons & (1 << (addon - 1)) ~= 0
end

-- player:removeOutfit(lookType)
-- Removes outfit ownership (and all addon bits) for the given lookType.
function Player.removeOutfit(self, lookType)
    return self:removeStorageValue(PlayerStorageKeys.outfitsBase + lookType)
end

-- player:removeOutfitAddon(lookType, addon)
-- Removes addon 1 or 2 from an owned outfit by clearing the corresponding bit.
function Player.removeOutfitAddon(self, lookType, addon)
    local addons = self:getOutfitAddons(lookType)
    addons = addons & ~(1 << (addon - 1))
    return self:setStorageValue(PlayerStorageKeys.outfitsBase + lookType, addons)
end

-- player:canWearOutfit(lookType[, addons]) -> boolean
-- Returns whether the player is allowed to wear an outfit with the requested addons.
--
-- Parameters:
-- - lookType: outfit lookType (client id)
-- - addons: addon bitmask (0, 1, 2, or 3). This is not an addon index.
--
-- Checks:
-- - staff bypass
-- - valid outfit
-- - premium requirement
-- - ownership requirement for locked outfits
-- - addon bitmask subset requirement
function Player.canWearOutfit(self, lookType, addons)
    if self:getGroup():getAccess() then
        return true
    end

    local outfit = Game.getOutfitByLookType(lookType)
    if not outfit then
        return false
    end

    if outfit.sex ~= self:getSex() then
        return false
    end

    if outfit.premium and not self:isPremium() then
        return false
    end

    if not outfit.unlocked and not self:hasOutfit(lookType) then
        return false
    end

    if addons == nil or addons == 0 then
        return true
    end

    return self:getOutfitAddons(lookType) & addons == addons
end
