local meltingHorn = Action()

function meltingHorn.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if not player:isPremium() then
		player:sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
		return true
	end
	if player:hasMount(38) then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You already have the obedience of the ursagrodon.")
		return true
	end

	if target.itemid == 20358 or target.itemid == 20359 or target.itemid == 20360 then
		if math.random(100) > 32 then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The ice cracked and the frozen creature with it - be more careful next time!")
			item:remove(1)
			target:transform(20361)
		else
			if target.itemid == 20358 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You managed to melt about half of the ice blook. Quickly now, it's ice cold here and the ice block could freeze over again.")
			elseif target.itemid == 20359 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You managed to melt almost the whole block, only the feet of the creature are still stuck in the ice. Finish the job!")
			elseif target.itemid == 20360 then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The freed ursagrodon look at you with glowing, obedient eyes.")
				item:remove(1)
				player:addMount(38)
				player:addAchievement("Natural Born Cowboy")
				player:addAchievement("Icy Glare")
			end
			target:transform(target.itemid + 1)
		end
		target:decay()
	end
	return true
end

meltingHorn:id(20355)
meltingHorn:register()
