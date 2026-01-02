do
	local debugAsserts = {}

	function Game.hasDebugAssert(playerGuid)
		return debugAsserts[playerGuid] ~= nil
	end

	function Game.removeDebugAssert(playerGuid)
		debugAsserts[playerGuid] = nil
	end

	function Game.saveDebugAssert(self, assertLine, date, description, comment)
		local file = io.open(string.format("data/logs/debug_assert/%s.log", self:getName()), "a")
		if not file then
			return
		end

		file:write(string.format("[%s]\n", date))
		file:write(string.format("assertLine: %s\n", assertLine))
		file:write(string.format("description: %s\n", description))
		file:write(string.format("comment: %s\n\n", comment))
		file:close()
	end
end
