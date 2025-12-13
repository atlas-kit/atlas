local items = {
	[11258] = { name = "Spark of the Phoenix", text = "The Spark of the Phoenix emblazes you.", id = BLESSING_ID_SPARK_OF_THE_PHOENIX, effect = CONST_ME_FIREATTACK },
	[11259] = { name = "Embrace of the World", text = "The Embrace of the World surrounds you.", id = BLESSING_ID_EMBRACE_OF_THE_WORLD, effect = CONST_ME_MAGIC_BLUE },
	[11260] = { name = "Spiritual Shielding", text = "The Spiritual Shielding protects you.", id = BLESSING_ID_SPIRITUAL_SHIELDING, effect = CONST_ME_LOSEENERGY },
	[11261] = { name = "Fire of the Suns", text = "The Fire of the Suns engulfs you.", id = BLESSING_ID_FIRE_OF_THE_SUNS, effect = CONST_ME_MAGIC_RED },
	[11262] = { name = "Wisdom of Solitude", text = "The Wisdom of Solitude inspires you.", id = BLESSING_ID_WISDOM_OF_SOLITUDE, effect = CONST_ME_MAGIC_GREEN },
}

local blessingCharms = Action()

function blessingCharms.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local blessItem = items[item.itemid]
	if not blessItem then
		return true
	end

	if player:hasBlessing(blessItem.id) then
		player:say("You already possess this blessing.", TALKTYPE_MONSTER_SAY)
		return true
	end

	player:addBlessing(blessItem.id)
	player:sendBlessings()
	player:say(blessItem.text, TALKTYPE_MONSTER_SAY)
	player:getPosition():sendMagicEffect(blessItem.effect)
	player:sendSupplyUsed(item)
	player:addBlessingsHistory(string.format("\"%s\" gained from using a blessing charm.", blessItem.name), 1)

	item:remove(1)
	return true
end

for k, v in pairs(items) do
	blessingCharms:id(k)
end

blessingCharms:register()
