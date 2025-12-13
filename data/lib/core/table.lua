table.contains = function(array, value)
	for _, targetColumn in pairs(array) do
		if targetColumn == value then
			return true
		end
	end
	return false
end

table.indexOf = function(array, value)
	for i, targetColumn in pairs(array) do
		if targetColumn == value then
			return i
		end
	end
	return nil
end

table.slice = function(array, from, to)
	return {unpack(array, from or 1, to or #array)}
end
