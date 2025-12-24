local traps = {
	[3028] = { -- strange slits
		transformTo = 3029,
		damage = {-60, -60}
	},
	[3031] = { -- spikes
		damage = {-60, -60}
	},
	[1571] = { -- trap
		transformTo = 1570,
		damage = {-30, -30}
	},
	[2941] = { -- jungle maw
		transformTo = 2942,
		damage = {-30, -30},
		type = COMBAT_EARTHDAMAGE
	},
	[8770] = { -- lava (walkable)
		damage = {-500, -500},
		type = COMBAT_FIREDAMAGE
	}
}

function onStepIn(creature, item, position, fromPosition)
	local trap = traps[item.itemid]
	if not trap then
		return true
	end

	if creature:isMonster() or creature:isPlayer() then
		doTargetCombat(0, creature, trap.type or COMBAT_PHYSICALDAMAGE, trap.damage[1], trap.damage[2], CONST_ME_NONE, true, false, false)
	end

	if trap.transformTo then
		item:transform(trap.transformTo)
	end
	return true
end

function onStepOut(creature, item, position, fromPosition)
	item:transform(item.itemid - 1)
	return true
end

function onRemoveItem(item, tile, position)
	local itemPosition = item:getPosition()
	if itemPosition:getDistance(position) > 0 then
		item:transform(item.itemid - 1)
		itemPosition:sendMagicEffect(CONST_ME_POFF)
	end
	return true
end
