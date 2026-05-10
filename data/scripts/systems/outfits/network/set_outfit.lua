local handler = PacketHandler(0xD3)

-- 0xD3: Set Outfit (client -> server)
-- Payload:
-- - outfitType:byte
-- - outfit (always):
--   - lookType:u16
--   - lookHead:byte, lookBody:byte, lookLegs:byte, lookFeet:byte
--   - lookAddons:byte
-- - branches:
--   - outfitType == 0 (Customize/Set outfit):
--     - lookMount:u16
--     - lookMountHead:byte, lookMountBody:byte, lookMountLegs:byte, lookMountFeet:byte
--     - familiarLookType:u16 (ignored)
--     - randomizeMount:bool
--   - outfitType == 1 (Store try-on):
--     - clears lookMount and reads 4 mount color bytes (client preview)
--   - outfitType == 2 (Podium edit):
--     - position:Position
--     - clientId:u16
--     - stackpos:byte
--     - lookMount:u16 + 4 mount color bytes
--     - direction:byte
--     - isVisible:bool
function handler.onReceive(player, msg)
    if not Outfits.AllowChangeOutfit then
        return
    end

    local outfitType = msg:getByte()

    local outfit = player:getCurrentOutfit()
    outfit.lookType = msg:getU16()
    outfit.lookHead = msg:getByte()
    outfit.lookBody = msg:getByte()
    outfit.lookLegs = msg:getByte()
    outfit.lookFeet = msg:getByte()
    outfit.lookAddons = msg:getByte()

    if outfitType == 0 then -- set outfit from customize character window
        local lookMount = msg:getU16()
        local lookMountHead = msg:getByte()
        local lookMountBody = msg:getByte()
        local lookMountLegs = msg:getByte()
        local lookMountFeet = msg:getByte()

        outfit.lookMount = lookMount
        if lookMount ~= 0 then -- only update colors if a mount with colors is selected
            outfit.lookMountHead = lookMountHead
            outfit.lookMountBody = lookMountBody
            outfit.lookMountLegs = lookMountLegs
            outfit.lookMountFeet = lookMountFeet
        end

        msg:getU16() -- familiar looktype
        local randomizeMount = msg:getBool()

        if not player:canWearOutfit(outfit.lookType, outfit.lookAddons) then
            return
        end

        if lookMount ~= 0 then
            if not player:canRideMount(lookMount) then
                player:setCurrentMount(nil)
                return
            end

            player:setCurrentMount(lookMount)
        end

        if player:setOutfitWithMountSpeed(outfit) then
            player:setWasMounted(lookMount ~= 0)
        end
        player:setRandomizeMount(randomizeMount)
    elseif outfitType == 1 then -- try outfit from store window
        outfit.lookMount = 0
        outfit.lookMountHead = msg:getByte()
        outfit.lookMountBody = msg:getByte()
        outfit.lookMountLegs = msg:getByte()
        outfit.lookMountFeet = msg:getByte()

        -- TODO: Implement store outfit preview window
    elseif outfitType == 2 then -- set podium outfit
        local position = msg:getPosition()
        local clientId = msg:getU16()
        local stackpos = msg:getByte()

        outfit.lookMount = msg:getU16()
        outfit.lookMountHead = msg:getByte()
        outfit.lookMountBody = msg:getByte()
        outfit.lookMountLegs = msg:getByte()
        outfit.lookMountFeet = msg:getByte()

        local direction = msg:getByte()
        local isVisible = msg:getBool()

        if not player:canWearOutfit(outfit.lookType, outfit.lookAddons) then
            return
        end

        if outfit.lookMount ~= 0 and not player:canRideMount(outfit.lookMount) then
            return
        end

        local tile = Tile(position)
        if not tile then
            return
        end

        local item = tile:getTopDownItem()
        if not item then
            return
        end

        if tile:getThingIndex(item) ~= stackpos then
            return
        end

        local it = Game.getItemTypeByClientId(clientId)
        if not it or it:getClientId() ~= clientId or it:getId() ~= item:getId() then
            return
        end

        player:onPodiumEdit(item, outfit, direction, isVisible)
    else
        print("Warning: Unknown outfitType received in set outfit message: " .. outfitType)
    end
end

handler:register()
