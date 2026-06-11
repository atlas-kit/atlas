math.randomseed(os.time())
dofile('data/lib/lib.lua')

ropeSpots = {
	386, 421, 7762, 12202, 12935, 12936, 13381, 14238, 17238, 21965, 21966, 21967, 21968, 23363
}

keys = {
	2967, 2968, 2969, 2970, 2971, 2972, 2973
}

openDoors = {
	1630, 1633, 1652, 1655, 1670, 1673, 1684, 1693, 4914, 4911, 5099, 5108, 5117, 5126, 5135, 5138, 5141,
	5144, 5279, 5282, 5734, 5737, 6193, 6196, 6250, 6253, 6893, 6902, 7035, 7044, 7713, 7716, 8251, 8254,
	8353, 8356, 9353, 9356, 9553, 9562, 9860, 9869, 11138, 11145, 11234, 11243, 11708, 11716, 13137, 13144, 17562,
	17571, 17702, 17711, 17995, 18004, 20445, 20454, 22629, 22636, 23877, 23878, 28520, 28368, 28369, 30035, 30036, 30039,
	30040, 30776, 30777, 30837, 30838, 33636, 33637, 34213, 34216, 34637, 34640, 34643, 34646, 34940, 34941, 36463, 36464
}
closedDoors = {
	1629, 1632, 1651, 1654, 1669, 1672, 1683, 1692, 5006, 5007, 5098, 5107, 5116, 5125, 5134, 5137, 5140,
	5143, 5278, 5281, 5733, 5736, 6192, 6195, 6249, 6252, 6892, 6901, 7034, 7043, 7712, 7715, 8250, 8253,
	8352, 8355, 9352, 9355, 9552, 9561, 9859, 9868, 11137, 11144, 11233, 11242, 11705, 11714, 13135, 13142, 17561,
	17570, 17701, 17710, 17994, 18003, 20444, 20453, 22628, 22635, 23873, 23875, 24903, 28364, 28366, 30033, 30034, 30037,
	30038, 30772, 30774, 30833, 30835, 33632, 33634, 34212, 34215, 34636, 34639, 34642, 34645, 34936, 34938, 36459, 36461
}
lockedDoors = {
	1628, 1631, 1650, 1653, 1668, 1671, 1682, 1691, 4913, 4912, 5097, 5106, 5115, 5124, 5133, 5136, 5139,
	5142, 5277, 5280, 5732, 5735, 6191, 6194, 6248, 6251, 6891, 6900, 7033, 7042, 7711, 7714, 8249, 8252,
	8351, 8354, 9351, 9354, 9551, 9560, 9858, 9867, 11136, 11143, 11232, 11241, 12249, 12250, 13136, 13143, 17560,
	17569, 17700, 17709, 17993, 18002, 20443, 20452, 22627, 22634, 23874, 23876, 28519, 28365, 28367, 30049, 30050, 30051,
	30052, 30773, 30775, 30834, 30836, 33633, 33635, 34211, 34214, 34635, 34638, 34641, 34644, 34937, 34939, 36460, 36462
}

openExtraDoors = {
	2178, 2180, 6796, 6798, 6800, 6802, 6960, 6962, 7055, 7057, 22503, 22505, 24542, 24544, 24587, 24589, 28885,
	28886, 31496, 31497, 34222, 34224, 36548
}
closedExtraDoors = {
	2177, 2179, 6795, 6797, 6799, 6801, 6959, 6961, 7054, 7056, 22502, 22504, 24541, 24543, 24586, 24588, 28658,
	28659, 31494, 31495, 34221, 34223, 36547
}

openHouseDoors = {
	1639, 1641, 1657, 1659, 1686, 1695, 5083, 5085, 5101, 5110, 5119, 5128, 5284, 5286, 5515, 5517, 6198,
	6200, 6255, 6257, 6895, 6904, 7037, 7046, 7718, 7720, 8256, 8258, 8358, 8360, 9358, 9360, 9555, 9564,
	12034, 12036, 15891, 15893, 15688, 17564, 17573, 17704, 17713, 17997, 18006, 20447, 20456, 33272, 33274
}
closedHouseDoors = {
	1638, 1640, 1656, 1658, 1685, 1694, 5082, 5084, 5100, 5109, 5118, 5127, 5283, 5285, 5514, 5516, 6197,
	6199, 6254, 6256, 6894, 6903, 7036, 7045, 7717, 7719, 8255, 8257, 8357, 8359, 9357, 9359, 9554, 9563,
	12033, 12035, 15890, 15892, 15687, 17563, 17572, 17703, 17712, 17996, 18005, 20446, 20455, 33271, 33273
}

openQuestDoors = {
	1643, 1645, 1661, 1663, 1675, 1677, 1690, 1699, 5105, 5114, 5123, 5132, 5288, 5290, 5746, 5748, 6202,
	6204, 6259, 6261, 6899, 6908, 7041, 7050, 7722, 7724, 8260, 8262, 8362, 8364, 9362, 9364, 9559, 9568,
	9866, 9875, 11142, 11149, 11238, 11247, 13141, 13148, 17568, 17577, 17708, 17717, 18001, 18010, 20451, 20460, 22507,
	22509, 22633, 22640, 30042, 30044, 30046, 30048, 31664, 31666, 31569, 31571
}
closedQuestDoors = {
	1642, 1644, 1660, 1662, 1674, 1676, 1689, 1698, 5104, 5113, 5122, 5131, 5287, 5289, 5745, 5749, 6201,
	6203, 6258, 6260, 6898, 6907, 7040, 7049, 7721, 7723, 8259, 8261, 8361, 8363, 9361, 9363, 9558, 9567,
	9865, 9874, 11141, 11148, 11237, 11246, 13140, 13147, 17567, 17576, 17707, 17716, 18000, 18009, 20450, 20459, 22506,
	22508, 22632, 22639, 30041, 30043, 30045, 30047, 31663, 31665, 31568, 31570
}

openLevelDoors = {
	1647, 1649, 1665, 1667, 1679, 1681, 1688, 1697, 5103, 5112, 5121, 5130, 5292, 5294, 6206, 6208, 6263,
	6265, 6897, 6906, 7039, 7048, 7726, 7728, 8264, 8266, 8366, 8368, 9366, 9368, 9557, 9566, 9864, 9873,
	11140, 11147, 11240, 11249, 17566, 17575, 17706, 17715, 17999, 18008, 20449, 20458, 22631, 22638
}
closedLevelDoors = {
	1646, 1648, 1664, 1666, 1678, 1680, 1687, 1696, 5102, 5111, 5120, 5129, 5291, 5293, 6205, 6207, 6262,
	6264, 6896, 6905, 7038, 7047, 7725, 7727, 8263, 8265, 8365, 8367, 9365, 9367, 9556, 9565, 9863, 9872,
	11139, 11146, 11239, 11248, 17565, 17574, 17705, 17714, 17998, 18007, 20448, 20457, 22630, 22637
}

function getDistanceBetween(firstPosition, secondPosition)
	local xDif = math.abs(firstPosition.x - secondPosition.x)
	local yDif = math.abs(firstPosition.y - secondPosition.y)
	local posDif = math.max(xDif, yDif)
	if firstPosition.z ~= secondPosition.z then
		posDif = posDif + 15
	end
	return posDif
end

function getLootRandom()
	return math.random(0, MAX_LOOTCHANCE) / configManager.getNumber(configKeys.RATE_LOOT)
end

table.contains = function(array, value)
	for _, targetColumn in pairs(array) do
		if targetColumn == value then
			return true
		end
	end
	return false
end

table.slice = function(array, from, to)
	return {unpack(array, from or 1, to or #array)}
end

string.split = function(str, sep)
	local res = {}
	for v in str:gmatch("([^" .. sep .. "]+)") do
		res[#res + 1] = v
	end
	return res
end

string.splitTrimmed = function(str, sep)
	local res = {}
	for v in str:gmatch("([^" .. sep .. "]+)") do
		res[#res + 1] = v:trim()
	end
	return res
end

string.trim = function(str)
	return str:match'^()%s*$' and '' or str:match'^%s*(.*%S)'
end

if not nextUseStaminaTime then
	nextUseStaminaTime = {}
end

function getPlayerDatabaseInfo(name_or_guid)
	local sql_where = ""

	if type(name_or_guid) == 'string' then
		sql_where = "WHERE `p`.`name`=" .. db.escapeString(name_or_guid) .. ""
	elseif type(name_or_guid) == 'number' then
		sql_where = "WHERE `p`.`id`='" .. name_or_guid .. "'"
	else
		return false
	end

	local sql_query = [[
		SELECT
			`p`.`id` as `guid`,
			`p`.`name`,
			CASE WHEN `po`.`player_id` IS NULL
				THEN 0
				ELSE 1
			END AS `online`,
			`p`.`group_id`,
			`p`.`level`,
			`p`.`experience`,
			`p`.`vocation`,
			`p`.`maglevel`,
			`p`.`skill_fist`,
			`p`.`skill_club`,
			`p`.`skill_sword`,
			`p`.`skill_axe`,
			`p`.`skill_dist`,
			`p`.`skill_shielding`,
			`p`.`skill_fishing`,
			`p`.`town_id`,
			`p`.`balance`,
			`gm`.`guild_id`,
			`gm`.`nick`,
			`g`.`name` AS `guild_name`,
			CASE WHEN `p`.`id` = `g`.`ownerid`
				THEN 1
				ELSE 0
			END AS `is_leader`,
			`gr`.`name` AS `rank_name`,
			`gr`.`level` AS `rank_level`,
			`h`.`id` AS `house_id`,
			`h`.`name` AS `house_name`,
			`h`.`town_id` AS `house_town`
		FROM `players` AS `p`
		LEFT JOIN `players_online` AS `po`
			ON `p`.`id` = `po`.`player_id`
		LEFT JOIN `guild_membership` AS `gm`
			ON `p`.`id` = `gm`.`player_id`
		LEFT JOIN `guilds` AS `g`
			ON `gm`.`guild_id` = `g`.`id`
		LEFT JOIN `guild_ranks` AS `gr`
			ON `gm`.`rank_id` = `gr`.`id`
		LEFT JOIN `houses` AS `h`
			ON `p`.`id` = `h`.`owner`
	]] .. sql_where

	local query = db.storeQuery(sql_query)
	if not query then
		return false
	end

	local info = {
		["guid"] = result.getNumber(query, "guid"),
		["name"] = result.getString(query, "name"),
		["online"] = result.getNumber(query, "online"),
		["group_id"] = result.getNumber(query, "group_id"),
		["level"] = result.getNumber(query, "level"),
		["experience"] = result.getNumber(query, "experience"),
		["vocation"] = result.getNumber(query, "vocation"),
		["maglevel"] = result.getNumber(query, "maglevel"),
		["skill_fist"] = result.getNumber(query, "skill_fist"),
		["skill_club"] = result.getNumber(query, "skill_club"),
		["skill_sword"] = result.getNumber(query, "skill_sword"),
		["skill_axe"] = result.getNumber(query, "skill_axe"),
		["skill_dist"] = result.getNumber(query, "skill_dist"),
		["skill_shielding"] = result.getNumber(query, "skill_shielding"),
		["skill_fishing"] = result.getNumber(query, "skill_fishing"),
		["town_id"] = result.getNumber(query, "town_id"),
		["balance"] = result.getNumber(query, "balance"),
		["guild_id"] = result.getNumber(query, "guild_id"),
		["nick"] = result.getString(query, "nick"),
		["guild_name"] = result.getString(query, "guild_name"),
		["is_leader"] = result.getNumber(query, "is_leader"),
		["rank_name"] = result.getString(query, "rank_name"),
		["rank_level"] = result.getNumber(query, "rank_level"),
		["house_id"] = result.getNumber(query, "house_id"),
		["house_name"] = result.getString(query, "house_name"),
		["house_town"] = result.getNumber(query, "house_town")
	}

	result.free(query)
	return info
end

function checkDuplicateStorageKeys(varName)
	local keys = _G[varName]
	local seen = {}
	local duplicates = {}
	for k, v in pairs(keys) do
		if seen[v] then
			table.insert(duplicates, v)
		else
			seen[v] = true
		end
	end

	if next(duplicates) == nil then
		return false
	else
		return duplicates
	end
end
