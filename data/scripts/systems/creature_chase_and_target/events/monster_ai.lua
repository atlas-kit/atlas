-- monster_ai.lua
-- Main AI loop for monsters: target acquisition, periodic target
-- re-evaluation, and chase/follow pathfinding. Runs every 1000ms via
-- the onCreatureThink event.
--
-- Also replaces the C++ Monster::searchTarget() and Monster::selectTarget()
-- with Lua implementations using Game.getSpectators() + monster:isTarget().
-- C++ targetList is preserved for defense/summon systems.

local function isInAttackRange(pos, creaturePos, monsterType)
	local distX = math.abs(pos.x - creaturePos.x)
	local distY = math.abs(pos.y - creaturePos.y)
	return distX <= monsterType:targetDistance() and distY <= monsterType:targetDistance()
end

function Monster.selectTarget(self, creature)
	if not self:isTarget(creature) then
		return false
	end

	if self:isSummon() then
		self:setTargetCreature(creature)
	else
		self:setTargetCreature(creature)
	end

	self:setChaseCreature(creature)
	return self:getChaseCreature() == creature
end

function Monster.searchTarget(self, searchType)
	searchType = searchType or TARGETSEARCH_DEFAULT
	local pos = self:getPosition()
	local chaseCreature = self:getChaseCreature()

	-- Tier 1: filter targets by canUseAttack.
	local resultList = {}
	for _, c in ipairs(self:getTargetList()) do
		if c ~= chaseCreature and self:isTarget(c) then
			if searchType == TARGETSEARCH_RANDOM or self:canAttack(c) then
				table.insert(resultList, c)
			end
		end
	end

	if searchType == TARGETSEARCH_NEAREST then
		-- Try canUseAttack-filtered list first.
		local best, bestDist = nil, 999999
		local searchList = #resultList > 0 and resultList or self:getTargetList()
		for _, c in ipairs(searchList) do
			if c ~= chaseCreature and self:isTarget(c) then
				local cp = c:getPosition()
				local d = math.abs(pos.x - cp.x) + math.abs(pos.y - cp.y)
				if d < bestDist then
					best, bestDist = c, d
				end
			end
		end
		if best and Monster.selectTarget(self, best) then
			return true
		end
	end

	if searchType == TARGETSEARCH_RANDOM then
		if #resultList > 0 then
			return Monster.selectTarget(self, resultList[math.random(#resultList)])
		end
		return false
	end

	if searchType == TARGETSEARCH_ATTACKRANGE then
		if #resultList > 0 then
			return Monster.selectTarget(self, resultList[math.random(#resultList)])
		end
		return false
	end

	-- Fallback: pick the first valid target in the full list.
	for _, c in ipairs(self:getTargetList()) do
		if c ~= chaseCreature and Monster.selectTarget(self, c) then
			return true
		end
	end
	return false
end

local function hasAggressiveCondition(creature)
	for _, cond in ipairs(creature:getConditions()) do
		if cond:isAggressive() then
			return true
		end
	end
	return false
end

local targetChangeTicks = {}

local function reevaluateTarget(monster, interval)
	local id = monster:getId()
	local monsterType = monster:getType()

	if monsterType:changeTargetSpeed() == 0 then
		return
	end

	local ticks = (targetChangeTicks[id] or 0) + interval
	if ticks < monsterType:changeTargetSpeed() then
		targetChangeTicks[id] = ticks
		return
	end

	targetChangeTicks[id] = 0

	if monsterType:changeTargetChance() < math.random(1, 100) then
		return
	end

	if monsterType:targetDistance() <= 1 then
		monster:searchTarget(TARGETSEARCH_RANDOM)
	else
		monster:searchTarget(TARGETSEARCH_NEAREST)
	end
end

do
	local event = Event()

	-- Clean up per-monster timing state when the creature is removed.
	function event.onCreatureRemoved(creature)
		targetChangeTicks[creature:getId()] = nil
	end

	event:register()
end

do
	local event = Event()

	-- Main AI tick: idle decision, target acquisition, re-evaluation,
	-- and chase/follow pathfinding.
	function event.onCreatureThink(creature, interval)
		local monster = creature:asMonster()
		if not monster then
			return
		end

		if monster:isIdle() then
			targetChangeTicks[monster:getId()] = nil
			return
		end

		local position = monster:getPosition()
		if not monster:isInSpawnRange(position) then
			targetChangeTicks[monster:getId()] = nil
			return
		end

		local hasMaster = monster:getMaster() ~= nil
		local targetCreature = monster:getTargetCreature()
		local hasTargets = monster:getTargetCount() > 0

		-- Return to spawn if there are no targets or friends nearby.
		-- Keep fighting if the monster has an active damage condition.
		if not hasMaster and not hasTargets and monster:getFriendCount() == 0
			and not monster:isFleeing() and not hasAggressiveCondition(monster) then
			if not monster:isWalkingToSpawn() then
				monster:walkToSpawn()
			end
			targetChangeTicks[monster:getId()] = nil
			return
		end

		-- Target acquisition.
		if not targetCreature and not monster:getChaseCreature() and hasTargets and not hasMaster then
			monster:searchTarget(TARGETSEARCH_NEAREST)
			targetCreature = monster:getTargetCreature()
		end

		-- Target re-evaluation or state cleanup.
		if not targetCreature then
			targetChangeTicks[monster:getId()] = nil
		elseif not hasMaster and hasTargets then
			reevaluateTarget(monster, interval)
		end

		-- Chase pathfinding.
		local chaseCreature = monster:getChaseCreature()
		if not chaseCreature then
			return
		end

		local chasePosition = chaseCreature:getPosition()
		if position.z ~= chasePosition.z then
			return
		end

		local dirs = monster:getPathTo(chasePosition, 1, 1, true, true, 0)
		if dirs then
			monster:startAutoWalk(dirs)
		end
	end

	event:register()
end
