do
	local storages = {}

	function Game.getStorageValue(key)
		return storages[key] or -1
	end

	function Game.setStorageValue(key, value)
		storages[key] = value
	end
end
