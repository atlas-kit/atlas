function Monster:onDropLoot(corpse)
	Event.onDropLoot(self, corpse)

	local player = Player(corpse:getCorpseOwner())
	if player then
		player:updateKillTracker(self, corpse)
	end
end

function Monster:onSpawn(position, startup, artificial)
	return Event.onSpawn(self, position, startup, artificial) or true
end
