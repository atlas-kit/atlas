do
	local storages = {}

	function Game.getAccountsStorage() return storages end

	function Game.clearAccountStorageValue(accountId, key)
		local storage = storages[accountId]
		if storage then
			storage[key] = nil
		end
	end

	function Game.getAccountStorageValue(accountId, key)
		local storage = storages[accountId]
		return storage and storage[key] or nil
	end

	function Game.setAccountStorageValue(accountId, key, value)
		if not storages[accountId] then
			storages[accountId] = {}
		end
		storages[accountId][key] = value
	end
end
