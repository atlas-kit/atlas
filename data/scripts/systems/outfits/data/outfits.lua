-- Outfit data source for the Outfits & Mounts module.
--
-- This file overrides the Game outfit lookup helpers used throughout the module:
-- - Game.getOutfits(sex)
-- - Game.getOutfitByLookType(lookType)
-- - Game.getOutfitByName(name, sex)
-- - Game.getOutfit(param, sex)
--
-- Each entry is keyed by lookType and includes:
-- - name (string)
-- - sex (PLAYERSEX_FEMALE / PLAYERSEX_MALE)
-- - premium (bool)
-- - unlocked (bool): if false, player must own it to wear
-- - enabled (bool): if false, excluded from lists
local outfits = {
    -- Female outfits
    [136] = {
        name = "Citizen",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [137] = {
        name = "Hunter",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [138] = {
        name = "Mage",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [139] = {
        name = "Knight",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [140] = {
        name = "Noblewoman",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [141] = {
        name = "Summoner",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [142] = {
        name = "Warrior",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [147] = {
        name = "Barbarian",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [148] = {
        name = "Druid",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [149] = {
        name = "Wizard",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [150] = {
        name = "Oriental",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [155] = {
        name = "Pirate",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [156] = {
        name = "Assassin",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [157] = {
        name = "Beggar",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [158] = {
        name = "Shaman",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [252] = {
        name = "Norsewoman",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [269] = {
        name = "Nightmare",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [270] = {
        name = "Jester",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [279] = {
        name = "Brotherhood",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [288] = {
        name = "Demon Hunter",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [324] = {
        name = "Yalaharian",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [329] = {
        name = "Newly Wed",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [336] = {
        name = "Warmaster",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [366] = {
        name = "Wayfarer",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [431] = {
        name = "Afflicted",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [433] = {
        name = "Elementalist",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [464] = {
        name = "Deepling",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [466] = {
        name = "Insectoid",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [471] = {
        name = "Entrepreneur",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [513] = {
        name = "Crystal Warlord",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [514] = {
        name = "Soil Guardian",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [542] = {
        name = "Demon Outfit",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [575] = {
        name = "Cave Explorer",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [578] = {
        name = "Dream Warden",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [618] = {
        name = "Glooth Engineer",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [620] = {
        name = "Jersey",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [632] = {
        name = "Champion",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [635] = {
        name = "Conjurer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [636] = {
        name = "Beastmaster",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [664] = {
        name = "Chaos Acolyte",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [666] = {
        name = "Death Herald",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [683] = {
        name = "Ranger",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [694] = {
        name = "Ceremonial Garb",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [696] = {
        name = "Puppeteer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [698] = {
        name = "Spirit Caller",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [724] = {
        name = "Evoker",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [732] = {
        name = "Seaweaver",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [745] = {
        name = "Recruiter",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [749] = {
        name = "Sea Dog",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [759] = {
        name = "Royal Pumpkin",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [845] = {
        name = "Rift Warrior",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [852] = {
        name = "Winter Warden",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [874] = {
        name = "Philosopher",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [885] = {
        name = "Arena Champion",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [900] = {
        name = "Lupine Warden",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [909] = {
        name = "Grove Keeper",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [929] = {
        name = "Festive Outfit",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [956] = {
        name = "Pharaoh",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [958] = {
        name = "Trophy Hunter",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [963] = {
        name = "Retro Warrior",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [965] = {
        name = "Retro Summoner",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [967] = {
        name = "Retro Noblewoman",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [969] = {
        name = "Retro Mage",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [971] = {
        name = "Retro Knight",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [973] = {
        name = "Retro Hunter",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [975] = {
        name = "Retro Citizen",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1020] = {
        name = "Herbalist",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1024] = {
        name = "Sun Priest",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1043] = {
        name = "Makeshift Warrior",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1050] = {
        name = "Siege Master",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1057] = {
        name = "Mercenary",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1070] = {
        name = "Battle Mage",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1095] = {
        name = "Discoverer",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1103] = {
        name = "Sinister Archer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1128] = {
        name = "Pumpkin Mummy",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1147] = {
        name = "Dream Warrior",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1162] = {
        name = "Percht Raider",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1174] = {
        name = "Owl Keeper",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1187] = {
        name = "Guidon Bearer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1203] = {
        name = "Void Master",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1205] = {
        name = "Veteran Paladin",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1207] = {
        name = "Lion of War",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1211] = {
        name = "Golden Outfit",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1244] = {
        name = "Hand of the Inquisition",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1246] = {
        name = "Breezy Garb",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1252] = {
        name = "Orcsoberfest Garb",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1271] = {
        name = "Poltergeist",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1280] = {
        name = "Herder",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1283] = {
        name = "Falconer",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1289] = {
        name = "Dragon Slayer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1293] = {
        name = "Trailblazer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1323] = {
        name = "Revenant",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1332] = {
        name = "Jouster",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1339] = {
        name = "Moth Cape",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1372] = {
        name = "Rascoohan",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1383] = {
        name = "Merry Garb",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1385] = {
        name = "Rune Master",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1387] = {
        name = "Citizen of Issavi",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1416] = {
        name = "Forest Warden",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1437] = {
        name = "Royal Bounacean Advisor",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1445] = {
        name = "Dragon Knight",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1450] = {
        name = "Arbalester",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1456] = {
        name = "Royal Costume",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1461] = {
        name = "Formal Dress",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1490] = {
        name = "Ghost Blade",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1501] = {
        name = "Nordic Chieftain",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1569] = {
        name = "Fire-Fighter",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1576] = {
        name = "Fencer",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1582] = {
        name = "Shadowlotus Disciple",
        sex = PLAYERSEX_FEMALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1598] = {
        name = "Ancient Aucar",
        sex = PLAYERSEX_FEMALE,
        premium = true,
        unlocked = false,
        enabled = true
    },

    -- Male outfits
    [128] = {
        name = "Citizen",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [129] = {
        name = "Hunter",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [130] = {
        name = "Mage",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [131] = {
        name = "Knight",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = true,
        enabled = true
    },
    [132] = {
        name = "Nobleman",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [133] = {
        name = "Summoner",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [134] = {
        name = "Warrior",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [143] = {
        name = "Barbarian",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [144] = {
        name = "Druid",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [145] = {
        name = "Wizard",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [146] = {
        name = "Oriental",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = true,
        enabled = true
    },
    [151] = {
        name = "Pirate",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [152] = {
        name = "Assassin",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [153] = {
        name = "Beggar",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [154] = {
        name = "Shaman",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [251] = {
        name = "Norseman",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [268] = {
        name = "Nightmare",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [273] = {
        name = "Jester",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [278] = {
        name = "Brotherhood",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [289] = {
        name = "Demon Hunter",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [325] = {
        name = "Yalaharian",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [328] = {
        name = "Newly Wed",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [335] = {
        name = "Warmaster",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [367] = {
        name = "Wayfarer",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [430] = {
        name = "Afflicted",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [432] = {
        name = "Elementalist",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [463] = {
        name = "Deepling",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [465] = {
        name = "Insectoid",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [472] = {
        name = "Entrepreneur",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [512] = {
        name = "Crystal Warlord",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [516] = {
        name = "Soil Guardian",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [541] = {
        name = "Demon Outfit",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [574] = {
        name = "Cave Explorer",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [577] = {
        name = "Dream Warden",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [610] = {
        name = "Glooth Engineer",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [619] = {
        name = "Jersey",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [633] = {
        name = "Champion",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [634] = {
        name = "Conjurer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [637] = {
        name = "Beastmaster",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [665] = {
        name = "Chaos Acolyte",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [667] = {
        name = "Death Herald",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [684] = {
        name = "Ranger",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [695] = {
        name = "Ceremonial Garb",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [697] = {
        name = "Puppeteer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [699] = {
        name = "Spirit Caller",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [725] = {
        name = "Evoker",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [733] = {
        name = "Seaweaver",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [746] = {
        name = "Recruiter",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [750] = {
        name = "Sea Dog",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [760] = {
        name = "Royal Pumpkin",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [846] = {
        name = "Rift Warrior",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [853] = {
        name = "Winter Warden",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [873] = {
        name = "Philosopher",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [884] = {
        name = "Arena Champion",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [899] = {
        name = "Lupine Warden",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [908] = {
        name = "Grove Keeper",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [931] = {
        name = "Festive Outfit",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [955] = {
        name = "Pharaoh",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [957] = {
        name = "Trophy Hunter",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [962] = {
        name = "Retro Warrior",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [964] = {
        name = "Retro Summoner",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [966] = {
        name = "Retro Nobleman",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [968] = {
        name = "Retro Mage",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [970] = {
        name = "Retro Knight",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [972] = {
        name = "Retro Hunter",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [974] = {
        name = "Retro Citizen",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1021] = {
        name = "Herbalist",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1023] = {
        name = "Sun Priest",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1042] = {
        name = "Makeshift Warrior",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1051] = {
        name = "Siege Master",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1056] = {
        name = "Mercenary",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1069] = {
        name = "Battle Mage",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1094] = {
        name = "Discoverer",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1102] = {
        name = "Sinister Archer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1127] = {
        name = "Pumpkin Mummy",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1146] = {
        name = "Dream Warrior",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1161] = {
        name = "Percht Raider",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1173] = {
        name = "Owl Keeper",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1186] = {
        name = "Guidon Bearer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1202] = {
        name = "Void Master",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1204] = {
        name = "Veteran Paladin",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1206] = {
        name = "Lion of War",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1210] = {
        name = "Golden Outfit",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1243] = {
        name = "Hand of the Inquisition",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1245] = {
        name = "Breezy Garb",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1251] = {
        name = "Orcsoberfest Garb",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1270] = {
        name = "Poltergeist",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1279] = {
        name = "Herder",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1282] = {
        name = "Falconer",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1288] = {
        name = "Dragon Slayer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1292] = {
        name = "Trailblazer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1322] = {
        name = "Revenant",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1331] = {
        name = "Jouster",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1338] = {
        name = "Moth Cape",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1371] = {
        name = "Rascoohan",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1382] = {
        name = "Merry Garb",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1384] = {
        name = "Rune Master",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1386] = {
        name = "Citizen of Issavi",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1415] = {
        name = "Forest Warden",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1436] = {
        name = "Royal Bounacean Advisor",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1444] = {
        name = "Dragon Knight",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1449] = {
        name = "Arbalester",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1457] = {
        name = "Royal Costume",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1460] = {
        name = "Formal Dress",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1489] = {
        name = "Ghost Blade",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1500] = {
        name = "Nordic Chieftain",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1568] = {
        name = "Fire-Fighter",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    },
    [1575] = {
        name = "Fencer",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1581] = {
        name = "Shadowlotus Disciple",
        sex = PLAYERSEX_MALE,
        premium = false,
        unlocked = false,
        enabled = true
    },
    [1597] = {
        name = "Ancient Aucar",
        sex = PLAYERSEX_MALE,
        premium = true,
        unlocked = false,
        enabled = true
    }
}

function Game.getOutfitByLookType(lookType)
    local outfit = outfits[lookType]
    if not outfit then
        return nil
    end

    return {
        lookType = lookType,
        name = outfit.name,
        sex = outfit.sex,
        premium = outfit.premium,
        unlocked = outfit.unlocked
    }
end

function Game.getOutfitByName(name, sex)
    if type(name) ~= "string" then
        return nil
    end

    for lookType, outfit in pairs(outfits) do
        if outfit.name:lower() == name:lower() and outfit.sex == sex then
            return {
                lookType = lookType,
                name = outfit.name,
                sex = outfit.sex,
                premium = outfit.premium,
                unlocked = outfit.unlocked
            }
        end
    end
    return nil
end

function Game.getOutfit(param, sex)
    local lookType = tonumber(param)
    if lookType then
        local outfit = Game.getOutfitByLookType(lookType)
        if outfit and (sex == nil or outfit.sex == sex) then
            return outfit
        end
        return nil
    end
    return Game.getOutfitByName(param, sex)
end

do
    local cachedOutfits = {
        [PLAYERSEX_FEMALE] = {},
        [PLAYERSEX_MALE] = {}
    }

    for lookType, outfit in pairs(outfits) do
        if outfit.enabled then
            table.insert(cachedOutfits[outfit.sex], {
                lookType = lookType,
                name = outfit.name,
                sex = outfit.sex,
                premium = outfit.premium,
                unlocked = outfit.unlocked
            })
        end
    end

    function Game.getOutfits(sex)
        return cachedOutfits[sex]
    end
end
