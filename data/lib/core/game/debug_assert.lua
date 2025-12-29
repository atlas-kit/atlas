do
	local debugAsserts = {}

	function Game.hasDebugAssert(playerGuid)
		return debugAsserts[playerGuid] ~= nil
	end

	function Game.addDebugAssert(playerGuid)
		debugAsserts[playerGuid] = os.time()
	end

	function Game.removeDebugAssert(playerGuid)
		debugAsserts[playerGuid] = nil
	end

	function Game.saveDebugAssert(playerName, assertLine, date, description, comment)
		local file = io.open(string.format("data/logs/debug_assert/%s.log", playerName), "a")
		if not file then
			return
		end

		file:write(string.format("[%s] assertLine: %s | description: %s | comment: %s\n", date, assertLine, description, comment))
		file:close()
	end
end
