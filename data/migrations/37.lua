function onUpdateDatabase()
	print("> Updating database to version 38 (blessings history).")
	db.query([[
		CREATE TABLE IF NOT EXISTS `blessings_history` (
			`id` int unsigned NOT NULL AUTO_INCREMENT,
			`player_id` int NOT NULL,
			`type` tinyint NOT NULL DEFAULT '0',
			`event` varchar(255) NOT NULL,
			`inserted` bigint unsigned NOT NULL,
			PRIMARY KEY (`id`),
			FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE
		) ENGINE=InnoDB DEFAULT CHARACTER SET=utf8;
	]])
	return true
end
