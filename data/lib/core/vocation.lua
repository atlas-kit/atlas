Vocation = {}
Vocation.__index = Vocation

setmetatable(Vocation, {
	__call = function(self, idOrName)
		if idOrName == nil then
			return setmetatable({}, Vocation)
		end
		local vocations = Game.getVocations()
		for _, voc in ipairs(vocations) do
			if voc:getId() == idOrName or voc:getName() == idOrName then
				return voc
			end
		end
		return nil
	end,
	__eq = function(a, b)
		return a.id == b.id
	end,
})

function Vocation:setId(id)
	self.id = id
	return self
end

function Vocation:setClientId(clientId)
	self.clientId = clientId
	return self
end

function Vocation:setName(name)
	self.name = name
	return self
end

function Vocation:setDescription(description)
	self.description = description
	return self
end

function Vocation:setMagicShield(magicShield)
	self.magicShield = magicShield
	return self
end

function Vocation:setCapacityGain(gainCap)
	self.gainCap = gainCap
	return self
end

function Vocation:setHealthGain(gainHP)
	self.gainHP = gainHP
	return self
end

function Vocation:setHealthGainTicks(gainHealthTicks)
	self.gainHealthTicks = gainHealthTicks
	return self
end

function Vocation:setHealthGainAmount(gainHealthAmount)
	self.gainHealthAmount = gainHealthAmount
	return self
end

function Vocation:setManaGain(gainMana)
	self.gainMana = gainMana
	return self
end

function Vocation:setManaGainTicks(gainManaTicks)
	self.gainManaTicks = gainManaTicks
	return self
end

function Vocation:setManaGainAmount(gainManaAmount)
	self.gainManaAmount = gainManaAmount
	return self
end

function Vocation:setManaMultiplier(manaMultiplier)
	self.manaMultiplier = manaMultiplier
	return self
end

function Vocation:setAttackSpeed(attackSpeed)
	self.attackSpeed = attackSpeed
	return self
end

function Vocation:setBaseSpeed(baseSpeed)
	self.baseSpeed = baseSpeed
	return self
end

function Vocation:setMaxSoul(soulMax)
	self.soulMax = soulMax
	return self
end

function Vocation:setSoulGainTicks(gainSoulTicks)
	self.gainSoulTicks = gainSoulTicks
	return self
end

function Vocation:setFromVocation(fromVocation)
	self.fromVocation = fromVocation
	return self
end

function Vocation:setAllowPvp(allowPvp)
	self.allowPvp = allowPvp
	return self
end

function Vocation:setNoPongKickTime(noPongKickTime)
	self.noPongKickTime = noPongKickTime
	return self
end

function Vocation:setFormula(meleeDamage, distDamage, defense, armor)
	self.formula = {meleeDamage = meleeDamage, distDamage = distDamage, defense = defense, armor = armor}
	return self
end

function Vocation:setSkillMultipliers(multipliers)
	self.skills = multipliers
	self.skillMultipliers = multipliers
	return self
end

function Vocation:register()
	return Game.registerVocation(self)
end

-- Getters
function Vocation:getId()
	return self.id
end

function Vocation:getClientId()
	return self.clientId
end

function Vocation:getName()
	return self.name
end

function Vocation:getDescription()
	return self.description
end

function Vocation:getCapacityGain()
	return self.gainCap
end

function Vocation:getHealthGain()
	return self.gainHP
end

function Vocation:getHealthGainTicks()
	return self.gainHealthTicks
end

function Vocation:getHealthGainAmount()
	return self.gainHealthAmount
end

function Vocation:getManaGain()
	return self.gainMana
end

function Vocation:getManaGainTicks()
	return self.gainManaTicks
end

function Vocation:getManaGainAmount()
	return self.gainManaAmount
end

function Vocation:getMaxSoul()
	return self.soulMax
end

function Vocation:getSoulGainTicks()
	return self.gainSoulTicks
end

function Vocation:getAttackSpeed()
	return self.attackSpeed
end

function Vocation:getBaseSpeed()
	return self.baseSpeed
end

function Vocation:allowsPvp()
	return self.allowPvp
end

function Vocation:getNoPongKickTime()
	return self.noPongKickTime
end

function Vocation:getFromVocation()
	return self.fromVocation
end

local SKILL_BASE = {50, 50, 50, 50, 30, 100, 20}
local MINIMUM_SKILL_LEVEL = 10

function Vocation:getRequiredSkillTries(skillType, skillLevel)
	local base = SKILL_BASE[skillType + 1]
	if not base then
		return 0
	end
	return math.floor(base * self.skillMultipliers[skillType + 1] ^ (skillLevel - (MINIMUM_SKILL_LEVEL + 1)))
end

function Vocation:getRequiredManaSpent(magicLevel)
	if magicLevel == 0 then
		return 0
	end
	return math.floor(1600 * self.manaMultiplier ^ (magicLevel - 1))
end

function Vocation:getDemotion()
	local fromId = self.fromVocation
	if fromId == 0 or fromId == self.id then
		return nil
	end
	return Vocation(fromId)
end

function Vocation:getPromotion()
	local vocations = Game.getVocations()
	for _, voc in ipairs(vocations) do
		if voc:getFromVocation() == self.id and voc:getId() ~= self.id then
			return voc
		end
	end
	return nil
end

function Vocation:getBase()
	local base = self
	while base:getDemotion() do
		base = base:getDemotion()
	end
	return base
end

function Vocation:getRelated()
	local vocations = {}
	local related = self:getBase()
	repeat
		vocations[#vocations + 1] = related
		related = related:getPromotion()
	until not related
	return vocations
end
