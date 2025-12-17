function Creature:onChangeOutfit(outfit)
	if Event.onChangeMount and not Event.onChangeMount(self, outfit.lookMount) then
		return false
	end
	return Event.onChangeOutfit and Event.onChangeOutfit(self, outfit) or true
end

function Creature:onAreaCombat(tile, isAggressive)
	return Event.onAreaCombat and Event.onAreaCombat(self, tile, isAggressive) or RETURNVALUE_NOERROR
end

function Creature:onTargetCombat(target)
	return Event.onTargetCombat and Event.onTargetCombat(self, target) or RETURNVALUE_NOERROR
end

function Creature:onHear(speaker, words, type)
	if Event.onHear then
		Event.onHear(self, speaker, words, type)
	end
end

function Creature:onChangeZone(fromZone, toZone)
	if Event.onChangeZone then
		Event.onChangeZone(self, fromZone, toZone)
	end
end

function Creature:onUpdateStorage(key, value, oldValue, isSpawn)
	if Event.onUpdateStorage then
		Event.onUpdateStorage(self, key, value, oldValue, isSpawn)
	end
end
