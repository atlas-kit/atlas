local function saveAccountsStorage()
	local transaction = DBTransaction()
	if not transaction:begin() then
		return false
	end

	local result = db.query("DELETE FROM `account_storage`")
	if not result then
		return false
	end

	local accountsStorage = Game.getAccountsStorage()
	for accountId, accountStorage in pairs(accountsStorage) do
		local query = DBInsert("INSERT INTO `account_storage` (`account_id`, `key`, `value`) VALUES")
		for key, value in pairs(accountStorage) do
			local success = query:addRow(accountId .. ", " .. key .. ", " .. value)
			if not success then
				return false
			end
		end

		if not query:execute() then
			return false
		end
	end

	return transaction:commit()
end

local event = Event()

event.onGameSave = function()
	local success = saveAccountsStorage()
	if not success then
		print("Failed to save account-level storage values.")
	end
end

event:register()
