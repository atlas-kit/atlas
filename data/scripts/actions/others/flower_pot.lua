local flowers = {
	{itemid = 306, watered = false, advance = false, msg = "You should plant some seeds first."},
	{itemid = 316, watered = true, advance = false, msg = "You watered your plant.", after = 324},
	{itemid = 324, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 321, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 331, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 333, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 335, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 337, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 339, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 341, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 1461, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 1464, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 7063, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 655, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 343, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 345, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 7066, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 653, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 340, watered = true, advance = false, msg = "You watered your plant.", after = 339},
	{itemid = 342, watered = true, advance = false, msg = "You watered your plant.", after = 341},
	{itemid = 344, watered = true, advance = false, msg = "You watered your plant.", after = 343},
	{itemid = 346, watered = true, advance = false, msg = "You watered your plant.", after = 345},
	{itemid = 110, watered = true, advance = false, msg = "You watered your plant.", after = 9077},
	{itemid = 654, watered = true, advance = false, msg = "You watered your plant.", after = 9075},
	{itemid = 325, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 339},
	{itemid = 326, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 341},
	{itemid = 327, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 343},
	{itemid = 328, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 345},
	{itemid = 7065, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 9077},
	{itemid = 7066, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 9075},
	{itemid = 330, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {324, 7670}, chance = 80},
	{itemid = 332, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {331, 7688}, chance = 80},
	{itemid = 334, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {333, 7690}, chance = 80},
	{itemid = 336, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {335, 7692}, chance = 80},
	{itemid = 338, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {337, 7694}, chance = 80},
	{itemid = 7064, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {9069, 9990}, chance = 80},
	{itemid = 2715, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {9073, 9988}, chance = 80},
	{itemid = 329, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {321, 7680, 7682, 7684, 7686, 9982, 9986}, chance = 80},
	{itemid = 3062, watered = true, advance = false, msg = "You finally remembered to water your plant and it recovered.", after = 14031},
	{itemid = 3060, watered = true, advance = true, msg = {"You watered your plant.", "Your plant has grown to the next stage!"}, after = {14031, 15441}, chance = 80},
	{itemid = 3061, watered = false, advance = false, msg = "Your plant doesn't need water."},
	{itemid = 3059, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 3063, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 3064, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 558, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 3066, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 3067, watered = false, advance = false, msg = "This plant can't wither anymore."},
	{itemid = 3068, watered = false, advance = false, msg = "This plant can't wither anymore."}
}

local flowerPot = Action()

function flowerPot.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	for _, flower in pairs(flowers) do
		if target.itemid == flower.itemid then
			if not flower.watered and not flower.advance then
				player:say(flower.msg, TALKTYPE_MONSTER_SAY)
			elseif flower.watered and not flower.advance then
				target:transform(flower.after)
				player:say(flower.msg, TALKTYPE_MONSTER_SAY)
				toPosition:sendMagicEffect(CONST_ME_LOSEENERGY)
				target:decay()
			elseif flower.watered and flower.advance then
				local i = 1
				if math.random(100) <= flower.chance then
					i = 2
					target:transform(flower.after[math.random(2, #flower.after)])
				else
					target:transform(flower.after[i])
				end
				toPosition:sendMagicEffect(CONST_ME_LOSEENERGY)
				player:say(flower.msg[i], TALKTYPE_MONSTER_SAY)
				target:decay()
			end
		break
		end
	end
	return true
end

flowerPot:id(7734) -- watering can
flowerPot:register()
