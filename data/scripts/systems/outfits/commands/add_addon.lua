-- /addaddon <player>, <outfitName|lookType>, <addon>
-- Adds addon 1 or 2 to an owned outfit for the target player.
local talkaction = TalkAction("/addaddon")

function talkaction.onSay(player, words, param)
    local split = param:splitTrimmed(",")
    if #split < 3 then
        player:sendCancelMessage("Insufficient parameters.")
        return false
    end

    local target = Player(split[1])
    if not target then
        player:sendCancelMessage("A player with that name is not online.")
        return false
    end

    local outfit = Game.getOutfit(split[2], target:getSex())
    if not outfit then
        player:sendCancelMessage("Outfit " .. split[2] .. " does not exist.")
        return false
    end

    if not target:hasOutfit(outfit.lookType) then
        player:sendCancelMessage("Target does not have this outfit.")
        return false
    end

    local addon = tonumber(split[3])
    if addon ~= 1 and addon ~= 2 then
        player:sendCancelMessage("Invalid addon value.")
        return false
    end

    if target:hasOutfitAddon(outfit.lookType, addon) then
        player:sendCancelMessage("Target already has this outfit with this addon.")
        return false
    end

    if not target:addOutfitAddon(outfit.lookType, addon) then
        player:sendCancelMessage("Failed to add addon to the outfit.")
        return false
    end

    player:sendTextMessage(MESSAGE_INFO_DESCR, "You have added addon " .. addon .. " for outfit " .. outfit.name ..
        " to " .. target:getName() .. ".")

    if Outfits.PrintCommandsToConsole then
        print(player:getName() .. " has granted addon " .. addon .. " for outfit " .. outfit.name .. " to " ..
                  target:getName() .. ".")
    end

    return true
end

talkaction:separator(" ")
talkaction:accountType(ACCOUNT_TYPE_GAMEMASTER)
talkaction:register()
