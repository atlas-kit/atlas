function onUpdateDatabase()
	print("> Updating database to version 38 (login attempts)")

	db.query([[
		CREATE TABLE IF NOT EXISTS `login_attempts` (
			`ip` varbinary(16) NOT NULL,
			`attempts` int unsigned NOT NULL DEFAULT 0,
			`last_attempt` bigint NOT NULL,
			`blocked_until` bigint NOT NULL DEFAULT 0,
			PRIMARY KEY (`ip`),
			KEY `blocked_until` (`blocked_until`)
		) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
	]])
	return true
end
