function onUpdateDatabase()
	print("> Updating database to version 38 (bestiary tracker)")
	db.query([[
		CREATE TABLE IF NOT EXISTS `player_bestiary_tracker` (
		`player_id` int NOT NULL,
		`race_id` int NOT NULL,
		`created_at` bigint NOT NULL DEFAULT (UNIX_TIMESTAMP()),
		PRIMARY KEY (`player_id`, `race_id`),
		FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE,
		INDEX `idx_player_id` (`player_id`)
		) ENGINE=InnoDB DEFAULT CHARACTER SET=utf8;
	]])
	return true
end
