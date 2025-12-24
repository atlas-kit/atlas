local instruments = {
	[4129] = {effect = CONST_ME_SOUND_GREEN}, -- wooden flute
	[4130] = {effect = CONST_ME_SOUND_GREEN}, -- lyre
	[4131] = {effect = CONST_ME_SOUND_GREEN}, -- lute
	[14219] = {effect = CONST_ME_SOUND_GREEN}, -- drum
	[4134] = {effect = CONST_ME_SOUND_GREEN}, -- panpipes
	[4135] = {effect = CONST_ME_SOUND_GREEN}, -- simple fanfare
	[4136] = {effect = CONST_ME_SOUND_GREEN}, -- fanfare
	[4137] = {effect = CONST_ME_SOUND_GREEN}, -- royal fanfare
	[3252] = {effect = CONST_ME_SOUND_GREEN}, -- post horn
	[4139] = {effect = CONST_ME_SOUND_GREEN}, -- war horn
	[4139] = {effects = {failure = CONST_ME_SOUND_PURPLE, success = CONST_ME_SOUND_GREEN}, chance = 50}, -- piano
	[4141] = {effects = {failure = CONST_ME_SOUND_PURPLE, success = CONST_ME_SOUND_GREEN}, chance = 50}, -- piano
	[4142] = {effects = {failure = CONST_ME_SOUND_PURPLE, success = CONST_ME_SOUND_GREEN}, chance = 50}, -- piano
	[4143] = {effects = {failure = CONST_ME_SOUND_PURPLE, success = CONST_ME_SOUND_GREEN}, chance = 50}, -- piano
	[4143] = {effect = CONST_ME_SOUND_GREEN}, -- harp
	[4145] = {effect = CONST_ME_SOUND_GREEN}, -- harp
	[566] = {effect = CONST_ME_SOUND_GREEN}, -- Waldo's post horn
	[3255] = {effect = CONST_ME_SOUND_GREEN}, -- drum (immovable)
	[3256] = {effect = CONST_ME_SOUND_GREEN}, -- simple fanfare (immovable)
	[3257] = {effect = CONST_ME_SOUND_YELLOW, itemId = 2059, itemCount = 10, chance = 80, remove = true}, -- cornucopia (immovable)
	[3258] = {effect = CONST_ME_SOUND_GREEN}, -- lute (immovable)
	[3259] = {effect = CONST_ME_SOUND_BLUE}, -- the horn of sundering (actual effect is unknown; immovable)
	[3260] = {effect = CONST_ME_SOUND_GREEN}, -- lyre (immovable)
	[3261] = {effect = CONST_ME_SOUND_GREEN}, -- panpipes (immovable)
	[4132] = {effect = CONST_ME_SOUND_BLUE}, -- bongo drum (actual effect is unknown)
	[4146] = {effects = {failure = CONST_ME_POFF, success = CONST_ME_SOUND_GREEN}, chance = 20}, -- didgeridoo
	[4147] = {effect = CONST_ME_SOUND_RED}, -- war drum
	[4285] = {effect = CONST_ME_SOUND_YELLOW, itemId = 2059, itemCount = 10, chance = 80, remove = true}, -- cornucopia
	[5786] = {effects = {failure = CONST_ME_SOUND_RED, success = CONST_ME_SOUND_YELLOW}, monster = "war wolf", chance = 60, remove = true}, -- wooden whistle
	[6572] = {effect = CONST_ME_SOUND_GREEN, text = "TOOOOOOT", transformId = 6572, decayId = 6572}, -- party trumpet
	[6572] = {effect = CONST_ME_SOUND_GREEN, text = "TOOOOOOT", transformId = 6572, decayId = 6572}, -- party trumpet
	[11646] = {effect = CONST_ME_SOUND_BLUE}, -- small whistle (actual effect is unknown)
	[19204] = {effect = CONST_ME_SOUND_WHITE} -- small crystal bell
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local instrument = instruments[item:getId()]
	local chance
	if instrument.chance then
		chance = instrument.chance >= math.random(1, 100)

		if instrument.monster and chance then
			local monster = Game.createMonster(instrument.monster, player:getPosition(), true)
			if monster then
				player:addSummon(monster)
			end
		elseif instrument.itemId and chance then
			player:addItem(instrument.itemId, instrument.itemCount)
		end
	end

	item:getPosition():sendMagicEffect(instrument.effect or instrument.effects and chance and instrument.effects.success or instrument.effects.failure)

	if instrument.transformId then
		player:say(instrument.text, TALKTYPE_MONSTER_SAY, false, nil, item:getPosition())
		item:transform(instrument.transformId)
		item:decay(instrument.decayId)
	end

	if not chance and instrument.remove then
		item:remove()
	end
	player:addAchievementProgress("Rockstar", 10000)
	return true
end
