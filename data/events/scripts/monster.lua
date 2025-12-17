function Monster:onDropLoot(corpse)
	if Event.onDropLoot then
		Event.onDropLoot(self, corpse)
	end

	local player = Player(corpse:getCorpseOwner())
	if player then
		player:updateKillTracker(self, corpse)
	end
end

function Monster:onSpawn(position, startup, artificial)
	return Event.onSpawn and Event.onSpawn(self, position, startup, artificial)
end
