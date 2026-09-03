-- Inventory capacity formula extension point.
--
-- Issue #300 asks for capacity/weight/money logic to be customizable from Lua.
-- Weight tracking, money counting, item counting/removal and the free-capacity
-- subtraction stay in C++ on purpose: they run on the hottest paths of the
-- engine (every item move, pickup, loot and trade) and the native code is
-- already exposed to Lua read-only (Player:getCapacity/getFreeCapacity/getMoney,
-- Player:getItemCount, Item:getWeight/getWorth). Moving those loops into Lua
-- would add a C++ -> Lua round trip per item operation with no behavioral gain.
--
-- What this module adds is the missing piece: a single, well-defined seam where
-- a server owner can override the player's TOTAL capacity from Lua, recomputed
-- whenever the inventory changes. It is DISABLED by default, so out of the box
-- the engine's native formula remains 100% authoritative and there is zero
-- added runtime cost (the callback is never even registered).
--
-- Capacity is measured in internal units where 1 oz = 100 (the same unit
-- Player:getCapacity() / Player:setCapacity() use).

InventoryCapacity = {
	-- Leave false to keep the engine's native capacity behavior untouched.
	-- Set true only after you customize calculate() below.
	Enabled = false,
}

-- Return the new TOTAL capacity for the player, or nil to keep the engine value.
--
-- IMPORTANT - idempotency: this runs again on every inventory change, so the
-- result MUST depend only on stable inputs (level, vocation, skills, storages,
-- equipped item ids, ...). NEVER derive it from player:getCapacity() or
-- getFreeCapacity(): reading the value you are about to overwrite makes the
-- formula compound itself on every equip/unequip.
--
-- Example (commented out) - base from level/vocation plus a flat bonus item:
--
--   function InventoryCapacity.calculate(player)
--       local vocation = player:getVocation()
--       local perLevel = vocation and vocation:getCapacityGain() or 0
--       local base = (player:getLevel() - 1) * perLevel
--       if player:getItemCount(BACKPACK_OF_HOLDING) > 0 then
--           base = base + 50 * 100 -- +50 oz
--       end
--       return base
--   end
function InventoryCapacity.calculate(player)
	return nil
end

if InventoryCapacity.Enabled then
	local event = Event()

	-- Fired on equip/unequip (slot inventory change). Returning nil keeps the
	-- event chain going so other onInventoryUpdate observers are unaffected.
	function event.onPlayerInventoryUpdate(player, item, slot, equip)
		-- Let the engine own the special cases: PlayerFlag_CannotPickupItem
		-- forces 0 and PlayerFlag_HasInfiniteCapacity forces uint32 max.
		-- getCapacity() returns those sentinels; do not override them.
		local engineCapacity = player:getCapacity()
		if engineCapacity == 0 or engineCapacity == 0xFFFFFFFF then
			return
		end

		local newCapacity = InventoryCapacity.calculate(player)
		if type(newCapacity) ~= "number" then
			return
		end

		newCapacity = math.max(0, math.floor(newCapacity))
		if newCapacity ~= engineCapacity then
			player:setCapacity(newCapacity)
		end
	end

	event:register()
end
