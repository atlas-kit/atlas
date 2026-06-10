local traps = {
	[2145] = { -- strange slits
		transformTo = 2146,
		damage = {-60, -60}
	},
	[2148] = { -- spikes
		damage = {-60, -60}
	},
	[3482] = { -- trap
		transformTo = 3481,
		damage = {-30, -30}
	},
	[3944] = { -- jungle maw
		transformTo = 3945,
		damage = {-30, -30},
		type = COMBAT_EARTHDAMAGE
	},
	[22675] = { -- lava (walkable)
		damage = {-500, -500},
		type = COMBAT_FIREDAMAGE
	}
}

do
	local event = MoveEvent()

	function event.onStepIn(creature, item, position, fromPosition)
		local trap = traps[item.itemid]
		if not trap then
			return true
		end

		if creature:isMonster() or creature:isPlayer() then
			doTargetCombat(0, creature, trap.type or COMBAT_PHYSICALDAMAGE, trap.damage[1], trap.damage[2],
				CONST_ME_NONE, true, false, false)
		end

		if trap.transformTo then
			item:transform(trap.transformTo)
		end
		return true
	end

	event:id(2145, 2148, 3482, 3944, 22675)
	event:register()
end

do
	local event = MoveEvent()

	function event.onStepOut(creature, item, position, fromPosition)
		item:transform(item.itemid - 1)
		return true
	end

	event:id(2146)
	event:register()
end

do
	local event = MoveEvent()

	function event.onRemoveItem(item, tile, position)
		local itemPosition = item:getPosition()
		if itemPosition:getDistance(position) > 0 then
			item:transform(item.itemid - 1)
			itemPosition:sendMagicEffect(CONST_ME_POFF)
		end
		return true
	end

	event:id(3482)
	event:register()
end
