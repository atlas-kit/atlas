local event = Event()

event.onCreatureDeath = function(self, corpse, killer, mostDamageKiller, lastHitUnjustified, mostDamageUnjustified)
	if not corpse or not self:isMonster() then
		return
	end

	if mostDamageKiller then
		if mostDamageKiller:isPlayer() then
			corpse:setCorpseOwner(mostDamageKiller:getId())
		else
			local master = mostDamageKiller:getMaster()
			if master and master:isPlayer() then
				corpse:setCorpseOwner(master:getId())
			end
		end
	end
end

event:register()
