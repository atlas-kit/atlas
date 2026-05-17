-- /removeoutfit <player>, <outfitName|lookType>
-- Removes the owned outfit from the target player.
local talkaction = TalkAction("/removeoutfit")

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

    if not target:hasOutfit(outfit.lookType) then
        player:sendCancelMessage("Target does not have this outfit.")
        return false
    end

    if not target:removeOutfit(outfit.lookType) then
        player:sendCancelMessage("Failed to remove the outfit.")
        return false
    end

    player:sendTextMessage(MESSAGE_INFO_DESCR,
        "You have removed outfit " .. outfit.name .. " from " .. target:getName() .. ".")

    if Outfits.PrintCommandsToConsole then
        print(player:getName() .. " has removed outfit " .. outfit.name .. " from " .. target:getName() .. ".")
    end

    return true
end

talkaction:separator(" ")
talkaction:accountType(ACCOUNT_TYPE_GAMEMASTER)
talkaction:register()
