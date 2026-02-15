local event = Event()

event.onGameStartup = function()
	local resultId = db.storeQuery("SELECT `account_id`, `key`, `value` FROM `account_storage`")
	if not resultId then
		return
	end

	repeat
		local accountId = result.getNumber(resultId, "account_id")
		local key = result.getNumber(resultId, "key")
		local value = result.getNumber(resultId, "value")
		Game.setAccountStorageValue(accountId, key, value)
	until not result.next(resultId)
	result.free(resultId)
end

event:register()
