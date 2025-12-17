function onUpdateDatabase()
	print("> Updating database to version 38 (big storage)")

	db.query([[
		ALTER TABLE `account_storage`
			MODIFY `key` BIGINT UNSIGNED NOT NULL,
			MODIFY `value` BIGINT NOT NULL;
	]])

	db.query([[
		ALTER TABLE `player_storage`
			MODIFY `key` BIGINT UNSIGNED NOT NULL DEFAULT 0,
			MODIFY `value` BIGINT NOT NULL DEFAULT 0;
	]])
	return true
end
