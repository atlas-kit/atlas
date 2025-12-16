function Creature:onChangeOutfit(outfit)
	if not Event.onChangeMount(self, outfit.lookMount) then
		return false
	end
	return Event.onChangeOutfit(self, outfit) or true
end

function Creature:onAreaCombat(tile, isAggressive)
	return Event.onAreaCombat(self, tile, isAggressive) or RETURNVALUE_NOERROR
end

function Creature:onTargetCombat(target)
	return Event.onTargetCombat(self, target) or RETURNVALUE_NOERROR
end

function Creature:onHear(speaker, words, type)
	Event.onHear(self, speaker, words, type)
end

function Creature:onChangeZone(fromZone, toZone)
	Event.onChangeZone(self, fromZone, toZone)
end

function Creature:onUpdateStorage(key, value, oldValue, isSpawn)
	Event.onUpdateStorage(self, key, value, oldValue, isSpawn)
end
