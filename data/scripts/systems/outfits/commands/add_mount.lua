-- /addmount <player>, <mountName|lookType>
-- Grants the mount to the target player if not already owned.
local talkaction = TalkAction("/addmount")

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

    local mount = Game.getMount(split[2])
    if not mount then
        player:sendCancelMessage("Mount " .. split[2] .. " does not exist.")
        return false
    end

    if target:hasMount(mount.lookType) then
        player:sendCancelMessage("Target already has this mount.")
        return false
    end

    if not target:addMount(mount.lookType) then
        player:sendCancelMessage("Failed to add the mount.")
        return false
    end

    player:sendTextMessage(MESSAGE_INFO_DESCR,
        "You have granted mount " .. mount.name .. " to " .. target:getName() .. ".")

    if Outfits.PrintCommandsToConsole then
        print(player:getName() .. " has granted mount " .. mount.name .. " to " .. target:getName() .. ".")
    end

    return true
end

talkaction:separator(" ")
talkaction:accountType(ACCOUNT_TYPE_GAMEMASTER)
talkaction:register()
