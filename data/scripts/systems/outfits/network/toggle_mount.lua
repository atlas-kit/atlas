local handler = PacketHandler(0xD4)

-- 0xD4: Toggle Mount (client -> server)
-- Payload: mounted:bool (true = mount, false = dismount)
-- Notes:
-- - Cooldown is enforced by Player.toggleMount (systems/outfits/core/mounts.lua).
-- - Mounting from a protection zone is rejected by Player.toggleMount.
function handler.onReceive(player, msg)
    if not Outfits.AllowToggleMount then
        return
    end

    local mounted = msg:getBool()
    player:toggleMount(mounted)
end

handler:register()
