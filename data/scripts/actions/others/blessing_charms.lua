local items = {
	[9426] = {text = "The Spiritual Shielding protects you.", id = 1, effect = CONST_ME_LOSEENERGY},
	[10342] = {text = "The Embrace of the World surrounds you.", id = 2, effect = CONST_ME_MAGIC_BLUE},
	[10344] = {text = "The Fire of the Suns engulfs you.", id = 3, effect = CONST_ME_MAGIC_RED},
	[10345] = {text = "The Wisdom of Solitude inspires you.", id = 4, effect = CONST_ME_MAGIC_GREEN},
	[10341] = {text = "The Spark of the Phoenix emblazes you.", id = 5, effect = CONST_ME_FIREATTACK}
}

local blessingCharms = Action()

function blessingCharms.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local blessItem = items[item.itemid]
	if blessItem then
		if player:hasBlessing(blessItem.id) then
			player:say("You already possess this blessing.", TALKTYPE_MONSTER_SAY)
			return true
		end

		player:addBlessing(blessItem.id)
		player:sendBlessings()
		player:say(blessItem.text, TALKTYPE_MONSTER_SAY)
		player:getPosition():sendMagicEffect(blessItem.effect)
		player:sendSupplyUsed(item)

		item:remove(1)
	end
	return true
end

for k, v in pairs(items) do
	blessingCharms:id(k)
end
blessingCharms:register()
