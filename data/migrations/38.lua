function onUpdateDatabase()
    print("> Updating database to version 39 (sample players)")

    db.query("INSERT INTO `accounts` (`id`, `name`, `email`, `password`, `type`) VALUES (1, 'atlas', '@atlas', 'ece8f46bf734ca267a2b6e0f7c3b471a5c964428', 6)")

    db.query([[
        INSERT INTO `players`
        (`id`, `name`, `group_id`, `account_id`, `level`, `vocation`, `health`, `healthmax`, `experience`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `maglevel`, `mana`, `manamax`, `manaspent`, `town_id`, `conditions`, `cap`, `sex`, `skill_club`, `skill_club_tries`, `skill_sword`, `skill_sword_tries`, `skill_axe`, `skill_axe_tries`, `skill_dist`, `skill_dist_tries`)
        VALUES
            (1, 'None Sample', 1, 1, 2, 0, 155, 155, 100, 113, 115, 95, 39, 129, 2, 60, 60, 5936, 1, '', 410, 1, 12, 155, 12, 155, 12, 155, 12, 93),
            (2, 'Sorcerer Sample', 1, 1, 8, 1, 185, 185, 4200, 113, 115, 95, 39, 129, 0, 90, 90, 0, 1, '', 470, 1, 10, 0, 10, 0, 10, 0, 10, 0),
            (3, 'Druid Sample', 1, 1, 8, 2, 185, 185, 4200, 113, 115, 95, 39, 129, 0, 90, 90, 0, 1, '', 470, 1, 10, 0, 10, 0, 10, 0, 10, 0),
            (4, 'Paladin Sample', 1, 1, 8, 3, 185, 185, 4200, 113, 115, 95, 39, 129, 0, 90, 90, 0, 1, '', 470, 1, 10, 0, 10, 0, 10, 0, 10, 0),
            (5, 'Knight Sample', 1, 1, 8, 4, 185, 185, 4200, 113, 115, 95, 39, 129, 0, 90, 90, 0, 1, '', 470, 1, 10, 0, 10, 0, 10, 0, 10, 0),
            (6, 'Administrator', 6, 1, 2, 0, 155, 155, 100, 113, 115, 95, 39, 129, 0, 60, 60, 0, 1, '', 410, 1, 10, 0, 10, 0, 10, 0, 10, 0)
    ]])
    return true
end