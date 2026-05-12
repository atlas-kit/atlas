local handler = PacketHandler(0xD2)

-- 0xD2: Request Outfit Window (client -> server)
-- Payload: (empty)
-- Response: 0xC8 (Outfit Window) via Player.sendOutfitWindow in systems/outfits/core/windows.lua
function handler.onReceive(player, msg)
    if not Outfits.AllowChangeOutfit then
        return
    end

    player:sendOutfitWindow()
end

handler:register()
