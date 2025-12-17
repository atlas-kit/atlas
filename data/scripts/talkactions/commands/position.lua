local talk = TalkAction("/pos")

function talk.onSay(player, words, param)
    if param == "" then
        player:sendCancelMessage("Command requires parameters. Usage: /pos x,y,z")
        return false
    end

    local split = param:split(",")
    if #split ~= 3 then
        player:sendCancelMessage("Invalid position format. Use: /pos x,y,z")
        return false
    end

    local position = Position(tonumber(split[1]), tonumber(split[2]), tonumber(split[3]))
    if not position then
        player:sendCancelMessage("Invalid position values.")
        return false
    end

    player:teleportTo(position)
    player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You have been teleported to position: " .. position.x .. ", " .. position.y .. ", " .. position.z)
    return false
end

talk:separator(" ")
talk:access(true)
talk:accountType(ACCOUNT_TYPE_GOD)
talk:register()
