local tinderBox = Action()

function tinderBox.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid == 18075 then -- fireproof horn
		item:remove(1)
		target:transform(20355) -- melting horn
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You ignite the tinder in the fireproof horn to create an effective device to melt just about anything.")
	end
	return true
end

tinderBox:id(22728)
tinderBox:register()
