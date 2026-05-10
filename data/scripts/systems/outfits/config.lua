-- Outfits & Mounts module configuration.
--
-- Networking entry points:
-- - 0xD2: systems/outfits/network/request_outfit_window.lua
-- - 0xD3: systems/outfits/network/set_outfit.lua
-- - 0xD4: systems/outfits/network/toggle_mount.lua
--
-- Core behavior:
-- - Mount selection/toggling: systems/outfits/core/mounts.lua
-- - Outfit/podium windows: systems/outfits/core/windows.lua
-- - Outfit ownership/addons: systems/outfits/core/outfits.lua
Outfits = {
    -- Set false to block outfit changes via incoming packets (0xD2/0xD3).
    AllowChangeOutfit = true,

    -- Set false to block mount toggles via incoming packets (0xD4).
    AllowToggleMount = true,

    -- Cooldown (ms) enforced for manual mount toggles (Ctrl+R). Forced zone toggles may bypass this.
    ToggleMountCooldown = 3000,

    -- Print outfit/mount command usage to console.
    PrintCommandsToConsole = true
}
