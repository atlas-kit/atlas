local event = Event()

event.onMonsterDespawn = function(self)
	if configManager.getBoolean(configKeys.MONSTER_OVERSPAWN) then
		self:removeFromSpawn()
		return
	end

	self:getPosition():sendMagicEffect(CONST_ME_POFF)

	if configManager.getBoolean(configKeys.REMOVE_ON_DESPAWN) then
		self:remove()
	else
		self:teleportToSpawn()
		self:setIdle(true)
	end
end

event:register()
