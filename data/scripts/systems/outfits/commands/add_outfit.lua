-- /addoutfit <player>, <outfitName|lookType>
-- Grants the outfit to the target player if not already owned.
local talkaction = TalkAction("/addoutfit")

function talkaction.onSay(player, words, param)
    local split = param:splitTrimmed(",")
    if #split < 2 then
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

    if target:hasOutfit(outfit.lookType) then
        player:sendCancelMessage("Target already has this outfit.")
        return false
    end

    if not target:addOutfit(outfit.lookType) then
        player:sendCancelMessage("Failed to add the outfit.")
        return false
    end

    player:sendTextMessage(MESSAGE_INFO_DESCR,
        "You have granted outfit " .. outfit.name .. " to " .. target:getName() .. ".")

    if Outfits.PrintCommandsToConsole then
        print(player:getName() .. " has granted outfit " .. outfit.name .. " to " .. target:getName() .. ".")
    end

    return true
end

talkaction:separator(" ")
talkaction:accountType(ACCOUNT_TYPE_GAMEMASTER)
talkaction:register()
