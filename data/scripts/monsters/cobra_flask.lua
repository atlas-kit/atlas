local cobras = {"cobra scout", "cobra vizier", "cobra assassin"}

local event = Event()

event.onMonsterSpawn = function(self, position, startup, artificial)
	if table.contains(cobras, self:getName():lower()) then
		local storage = Game.getStorageValue(GlobalStorageKeys.cobraBastionFlask)
		if storage then
			if storage >= os.time() then
				self:setHealth(self:getMaxHealth() * 0.75)
				self:getPosition():sendMagicEffect(CONST_ME_GREEN_RINGS)
			else
				Game.setStorageValue(GlobalStorageKeys.cobraBastionFlask, nil)
			end
		end
	end
	return true
end

event:register(-1)

local cobraFlask = Action()

function cobraFlask.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if table.contains({31284, 31285, 31286, 31287}, target:getId()) then
		target:getPosition():sendMagicEffect(CONST_ME_GREENSMOKE)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You carefully pour just a tiny, little, finely dosed... and there goes the whole content of the bottle. Stand back!")
		item:transform(31297)
		Game.setStorageValue(GlobalStorageKeys.cobraBastionFlask, os.time() + 30 * 60)
	end
	return true
end

cobraFlask:id(31296)
cobraFlask:register()
