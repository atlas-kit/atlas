function onUpdateDatabase()
	print("> Updating database to version 15 (moving groups to data/scripts/groups.lua)")

	db.query("ALTER TABLE players DROP FOREIGN KEY players_ibfk_2")
	db.query("DROP INDEX group_id ON players")

	db.query("ALTER TABLE accounts DROP FOREIGN KEY accounts_ibfk_1")
	db.query("DROP INDEX group_id ON accounts")
	db.query("ALTER TABLE `accounts` DROP `group_id`")

	local groupsFile = io.open("data/scripts/groups.lua", "w")
	if groupsFile then
		groupsFile:write("-- Player groups definition (generated from the legacy `groups` database table).\n")
		groupsFile:write("-- `flags` is the raw PlayerFlags bitmask; see data/scripts/groups.lua in a\n")
		groupsFile:write("-- fresh install for the human-readable flag-name form.\n\n")

		local resultId = db.storeQuery("SELECT `id`, `name`, `flags`, `access`, `maxdepotitems`, `maxviplist` FROM `groups` ORDER BY `id` ASC")
		if resultId then
			repeat
				groupsFile:write("Group{\n")
				groupsFile:write("\tid = " .. result.getNumber(resultId, "id") .. ",\n")
				groupsFile:write("\tname = " .. string.format("%q", result.getString(resultId, "name")) .. ",\n")
				groupsFile:write("\tflags = " .. string.format("%u", result.getNumber(resultId, "flags")) .. ",\n")
				groupsFile:write("\taccess = " .. (result.getNumber(resultId, "access") ~= 0 and "true" or "false") .. ",\n")
				groupsFile:write("\tmaxDepotItems = " .. result.getNumber(resultId, "maxdepotitems") .. ",\n")
				groupsFile:write("\tmaxVipEntries = " .. result.getNumber(resultId, "maxviplist") .. ",\n")
				groupsFile:write("}\n\n")
			until not result.next(resultId)
			result.free(resultId)
		end

		groupsFile:close()

		db.query("DROP TABLE `groups`")
	end
	return true
end
