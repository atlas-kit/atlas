-- Mount data source for the Outfits & Mounts module.
--
-- This file overrides the Game mount lookup helpers used throughout the module:
-- - Game.getMounts()
-- - Game.getMountByLookType(lookType)
-- - Game.getMountByName(name)
-- - Game.getMount(param)
--
-- Each entry is keyed by lookType and includes:
-- - name (string)
-- - speed (number)
-- - premium (bool)
local mounts = {
    [368] = {
        name = "Widow Queen",
        speed = 20,
        premium = true
    },
    [369] = {
        name = "Racing Bird",
        speed = 20,
        premium = true
    },
    [370] = {
        name = "War Bear",
        speed = 20,
        premium = true
    },
    [371] = {
        name = "Black Sheep",
        speed = 20,
        premium = true
    },
    [372] = {
        name = "Midnight Panther",
        speed = 20,
        premium = true
    },
    [373] = {
        name = "Draptor",
        speed = 20,
        premium = true
    },
    [374] = {
        name = "Titanica",
        speed = 20,
        premium = true
    },
    [375] = {
        name = "Tin Lizzard",
        speed = 20,
        premium = true
    },
    [376] = {
        name = "Blazebringer",
        speed = 20,
        premium = true
    },
    [377] = {
        name = "Rapid Boar",
        speed = 20,
        premium = true
    },
    [378] = {
        name = "Stampor",
        speed = 20,
        premium = true
    },
    [379] = {
        name = "Undead Cavebear",
        speed = 20,
        premium = true
    },
    [387] = {
        name = "Donkey",
        speed = 20,
        premium = true
    },
    [388] = {
        name = "Tiger Slug",
        speed = 20,
        premium = true
    },
    [389] = {
        name = "Uniwheel",
        speed = 20,
        premium = true
    },
    [390] = {
        name = "Crystal Wolf",
        speed = 20,
        premium = true
    },
    [392] = {
        name = "War Horse",
        speed = 20,
        premium = true
    },
    [401] = {
        name = "Kingly Deer",
        speed = 20,
        premium = true
    },
    [402] = {
        name = "Tamed Panda",
        speed = 20,
        premium = true
    },
    [405] = {
        name = "Dromedary",
        speed = 20,
        premium = true
    },
    [406] = {
        name = "Scorpion King",
        speed = 20,
        premium = true
    },
    [421] = {
        name = "Rented Horse",
        speed = 20,
        premium = false
    },
    [426] = {
        name = "Armoured War Horse",
        speed = 20,
        premium = false
    },
    [427] = {
        name = "Shadow Draptor",
        speed = 20,
        premium = false
    },
    [437] = {
        name = "Rented Horse",
        speed = 20,
        premium = false
    },
    [438] = {
        name = "Rented Horse",
        speed = 20,
        premium = false
    },
    [447] = {
        name = "Lady Bug",
        speed = 20,
        premium = true
    },
    [450] = {
        name = "Manta Ray",
        speed = 20,
        premium = true
    },
    [502] = {
        name = "Ironblight",
        speed = 20,
        premium = true
    },
    [503] = {
        name = "Magma Crawler",
        speed = 20,
        premium = true
    },
    [506] = {
        name = "Dragonling",
        speed = 20,
        premium = true
    },
    [515] = {
        name = "Gnarlhound",
        speed = 20,
        premium = true
    },
    [521] = {
        name = "Crimson Ray",
        speed = 20,
        premium = false
    },
    [522] = {
        name = "Steelbeak",
        speed = 20,
        premium = false
    },
    [526] = {
        name = "Water Buffalo",
        speed = 20,
        premium = true
    },
    [546] = {
        name = "Tombstinger",
        speed = 20,
        premium = false
    },
    [547] = {
        name = "Platesaurian",
        speed = 20,
        premium = false
    },
    [548] = {
        name = "Ursagrodon",
        speed = 20,
        premium = true
    },
    [559] = {
        name = "The Hellgrip",
        speed = 20,
        premium = true
    },
    [571] = {
        name = "Noble Lion",
        speed = 20,
        premium = true
    },
    [572] = {
        name = "Desert King",
        speed = 20,
        premium = false
    },
    [580] = {
        name = "Shock Head",
        speed = 20,
        premium = true
    },
    [606] = {
        name = "Walker",
        speed = 20,
        premium = true
    },
    [621] = {
        name = "Azudocus",
        speed = 20,
        premium = false
    },
    [622] = {
        name = "Carpacosaurus",
        speed = 20,
        premium = false
    },
    [624] = {
        name = "Death Crawler",
        speed = 20,
        premium = false
    },
    [626] = {
        name = "Flamesteed",
        speed = 20,
        premium = false
    },
    [627] = {
        name = "Jade Lion",
        speed = 20,
        premium = false
    },
    [628] = {
        name = "Jade Pincer",
        speed = 20,
        premium = false
    },
    [629] = {
        name = "Nethersteed",
        speed = 20,
        premium = false
    },
    [630] = {
        name = "Tempest",
        speed = 20,
        premium = false
    },
    [631] = {
        name = "Winter King",
        speed = 20,
        premium = false
    },
    [644] = {
        name = "Doombringer",
        speed = 20,
        premium = false
    },
    [647] = {
        name = "Woodland Prince",
        speed = 20,
        premium = false
    },
    [648] = {
        name = "Hailstorm Fury",
        speed = 20,
        premium = false
    },
    [649] = {
        name = "Siegebreaker",
        speed = 20,
        premium = false
    },
    [650] = {
        name = "Poisonbane",
        speed = 20,
        premium = false
    },
    [651] = {
        name = "Blackpelt",
        speed = 20,
        premium = false
    },
    [669] = {
        name = "Golden Dragonfly",
        speed = 20,
        premium = false
    },
    [670] = {
        name = "Steel Bee",
        speed = 20,
        premium = false
    },
    [671] = {
        name = "Copper Fly",
        speed = 20,
        premium = false
    },
    [672] = {
        name = "Tundra Rambler",
        speed = 20,
        premium = false
    },
    [673] = {
        name = "Highland Yak",
        speed = 20,
        premium = false
    },
    [674] = {
        name = "Glacier Vagabond",
        speed = 20,
        premium = false
    },
    [688] = {
        name = "Flying Divan",
        speed = 20,
        premium = false
    },
    [689] = {
        name = "Magic Carpet",
        speed = 20,
        premium = false
    },
    [690] = {
        name = "Floating Kashmir",
        speed = 20,
        premium = false
    },
    [691] = {
        name = "Ringtail Waccoon",
        speed = 20,
        premium = false
    },
    [692] = {
        name = "Night Waccoon",
        speed = 20,
        premium = false
    },
    [693] = {
        name = "Emerald Waccoon",
        speed = 20,
        premium = false
    },
    [682] = {
        name = "Glooth Glider",
        speed = 20,
        premium = true
    },
    [685] = {
        name = "Shadow Hart",
        speed = 20,
        premium = false
    },
    [686] = {
        name = "Black Stag",
        speed = 20,
        premium = false
    },
    [687] = {
        name = "Emperor Deer",
        speed = 20,
        premium = false
    },
    [726] = {
        name = "Flitterkatzen",
        speed = 20,
        premium = false
    },
    [727] = {
        name = "Venompaw",
        speed = 20,
        premium = false
    },
    [728] = {
        name = "Batcat",
        speed = 20,
        premium = false
    },
    [734] = {
        name = "Sea Devil",
        speed = 20,
        premium = false
    },
    [735] = {
        name = "Coralripper",
        speed = 20,
        premium = false
    },
    [736] = {
        name = "Plumfish",
        speed = 20,
        premium = false
    },
    [738] = {
        name = "Gorongra",
        speed = 20,
        premium = false
    },
    [739] = {
        name = "Noctungra",
        speed = 20,
        premium = false
    },
    [740] = {
        name = "Silverneck",
        speed = 20,
        premium = false
    },
    [761] = {
        name = "Slagsnare",
        speed = 20,
        premium = false
    },
    [762] = {
        name = "Nightstinger",
        speed = 20,
        premium = false
    },
    [763] = {
        name = "Razorcreep",
        speed = 20,
        premium = false
    },
    [848] = {
        name = "Rift Runner",
        speed = 20,
        premium = true
    },
    [849] = {
        name = "Nightdweller",
        speed = 20,
        premium = false
    },
    [850] = {
        name = "Frostflare",
        speed = 20,
        premium = false
    },
    [851] = {
        name = "Cinderhoof",
        speed = 20,
        premium = false
    },
    [868] = {
        name = "Mouldpincer",
        speed = 20,
        premium = false
    },
    [869] = {
        name = "Bloodcurl",
        speed = 20,
        premium = false
    },
    [870] = {
        name = "Leafscuttler",
        speed = 20,
        premium = false
    },
    [883] = {
        name = "Sparkion",
        speed = 20,
        premium = true
    },
    [886] = {
        name = "Swamp Snapper",
        speed = 20,
        premium = false
    },
    [887] = {
        name = "Mould Shell",
        speed = 20,
        premium = false
    },
    [888] = {
        name = "Reed Lurker",
        speed = 20,
        premium = false
    },
    [889] = {
        name = "Neon Sparkid",
        speed = 20,
        premium = true
    },
    [890] = {
        name = "Vortexion",
        speed = 20,
        premium = true
    },
    [901] = {
        name = "Ivory Fang",
        speed = 20,
        premium = false
    },
    [902] = {
        name = "Shadow Claw",
        speed = 20,
        premium = false
    },
    [903] = {
        name = "Snow Pelt",
        speed = 20,
        premium = false
    },
    [905] = {
        name = "Jackalope",
        speed = 20,
        premium = false
    },
    [906] = {
        name = "Dreadhare",
        speed = 20,
        premium = false
    },
    [907] = {
        name = "Wolpertinger",
        speed = 20,
        premium = false
    },
    [937] = {
        name = "Stone Rhino",
        speed = 20,
        premium = true
    },
    [950] = {
        name = "Gold Sphinx",
        speed = 20,
        premium = false
    },
    [951] = {
        name = "Emerald Sphinx",
        speed = 20,
        premium = false
    },
    [952] = {
        name = "Shadow Sphinx",
        speed = 20,
        premium = false
    },
    [959] = {
        name = "Jungle Saurian",
        speed = 20,
        premium = false
    },
    [960] = {
        name = "Ember Saurian",
        speed = 20,
        premium = false
    },
    [961] = {
        name = "Lagoon Saurian",
        speed = 20,
        premium = false
    },
    [1017] = {
        name = "Blazing Unicorn",
        speed = 20,
        premium = false
    },
    [1018] = {
        name = "Arctic Unicorn",
        speed = 20,
        premium = false
    },
    [1019] = {
        name = "Prismatic unicorn",
        speed = 20,
        premium = false
    },
    [1025] = {
        name = "Cranium Spider",
        speed = 20,
        premium = false
    },
    [1026] = {
        name = "Cave Tarantula",
        speed = 20,
        premium = false
    },
    [1027] = {
        name = "Gloom Widow",
        speed = 20,
        premium = false
    },
    [1049] = {
        name = "Mole",
        speed = 20,
        premium = true
    },
    [1052] = {
        name = "Marsh Toad",
        speed = 20,
        premium = false
    },
    [1053] = {
        name = "Sanguine Frog",
        speed = 20,
        premium = false
    },
    [1054] = {
        name = "Toxic Toad",
        speed = 20,
        premium = false
    },
    [1091] = {
        name = "Ebony Tiger",
        speed = 20,
        premium = false
    },
    [1092] = {
        name = "Feral Tiger",
        speed = 20,
        premium = false
    },
    [1093] = {
        name = "Jungle Tiger",
        speed = 20,
        premium = false
    },
    [1101] = {
        name = "Fleeting Knowledge",
        speed = 20,
        premium = true
    },
    [1104] = {
        name = "Tawny Owl",
        speed = 20,
        premium = false
    },
    [1105] = {
        name = "Snowy Owl",
        speed = 20,
        premium = false
    },
    [1106] = {
        name = "Boreal Owl",
        speed = 20,
        premium = false
    },
    [1150] = {
        name = "Lacewing Moth",
        speed = 20,
        premium = true
    },
    [1151] = {
        name = "Hibernal Moth",
        speed = 20,
        premium = true
    },
    [1163] = {
        name = "Cold Percht Sleigh",
        speed = 20,
        premium = true
    },
    [1164] = {
        name = "Bright Percht Sleigh",
        speed = 20,
        premium = true
    },
    [1165] = {
        name = "Dark Percht Sleigh",
        speed = 20,
        premium = true
    },
    [1167] = {
        name = "Festive Snowman",
        speed = 20,
        premium = false
    },
    [1168] = {
        name = "Muffled Snowman",
        speed = 20,
        premium = false
    },
    [1169] = {
        name = "Caped Snowman",
        speed = 20,
        premium = false
    },
    [1179] = {
        name = "Rabbit Rickshaw",
        speed = 20,
        premium = false
    },
    [1180] = {
        name = "Bunny Dray",
        speed = 20,
        premium = false
    },
    [1181] = {
        name = "Cony Cart",
        speed = 20,
        premium = false
    },
    [1183] = {
        name = "River Crocovile",
        speed = 20,
        premium = false
    },
    [1184] = {
        name = "Swamp Crocovile",
        speed = 20,
        premium = false
    },
    [1185] = {
        name = "Nightmarish Crocovile",
        speed = 20,
        premium = false
    },
    [1191] = {
        name = "Gryphon",
        speed = 20,
        premium = true
    },
    [1208] = {
        name = "Jousting Eagle",
        speed = 20,
        premium = false
    },
    [1209] = {
        name = "Cerberus Champion",
        speed = 20,
        premium = false
    },
    [1229] = {
        name = "Cold Percht Sleigh Variant",
        speed = 20,
        premium = true
    },
    [1230] = {
        name = "Bright Percht Sleigh Variant",
        speed = 20,
        premium = true
    },
    [1231] = {
        name = "Dark Percht Sleigh Variant",
        speed = 20,
        premium = true
    },
    [1232] = {
        name = "Cold Percht Sleigh Final",
        speed = 20,
        premium = true
    },
    [1233] = {
        name = "Bright Percht Sleigh Final",
        speed = 20,
        premium = true
    },
    [1234] = {
        name = "Dark Percht Sleigh Final",
        speed = 20,
        premium = true
    },
    [1247] = {
        name = "Battle Badger",
        speed = 20,
        premium = false
    },
    [1248] = {
        name = "Ether Badger",
        speed = 20,
        premium = false
    },
    [1249] = {
        name = "Zaoan Badger",
        speed = 20,
        premium = false
    },
    [1257] = {
        name = "Blue Rolling Barrel",
        speed = 20,
        premium = true
    },
    [1258] = {
        name = "Red Rolling Barrel",
        speed = 20,
        premium = true
    },
    [1259] = {
        name = "Green Rolling Barrel",
        speed = 20,
        premium = true
    },
    [1264] = {
        name = "Floating Sage",
        speed = 20,
        premium = false
    },
    [1265] = {
        name = "Floating Scholar",
        speed = 20,
        premium = false
    },
    [1266] = {
        name = "Floating Augur",
        speed = 20,
        premium = false
    },
    [1269] = {
        name = "Haze",
        speed = 20,
        premium = true
    },
    [1281] = {
        name = "Antelope",
        speed = 20,
        premium = true
    },
    [1284] = {
        name = "Snow Strider",
        speed = 20,
        premium = false
    },
    [1285] = {
        name = "Dusk Pryer",
        speed = 20,
        premium = false
    },
    [1286] = {
        name = "Dawn Strayer",
        speed = 20,
        premium = false
    },
    [1321] = {
        name = "Phantasmal Jade",
        speed = 20,
        premium = true
    },
    [1324] = {
        name = "Savanna Ostrich",
        speed = 20,
        premium = true
    },
    [1325] = {
        name = "Coral Rhea",
        speed = 20,
        premium = true
    },
    [1326] = {
        name = "Eventide Nandu",
        speed = 20,
        premium = true
    },
    [1333] = {
        name = "Voracious Hyaena",
        speed = 20,
        premium = false
    },
    [1334] = {
        name = "Cunning Hyaena",
        speed = 20,
        premium = false
    },
    [1335] = {
        name = "Scruffy Hyaena",
        speed = 20,
        premium = false
    },
    [1336] = {
        name = "White Lion",
        speed = 20,
        premium = true
    },
    [1363] = {
        name = "Krakoloss",
        speed = 20,
        premium = true
    },
    [1379] = {
        name = "Merry Mammoth",
        speed = 20,
        premium = false
    },
    [1380] = {
        name = "Holiday Mammoth",
        speed = 20,
        premium = false
    },
    [1381] = {
        name = "Festive Mammoth",
        speed = 20,
        premium = false
    },
    [1389] = {
        name = "Void Watcher",
        speed = 20,
        premium = false
    },
    [1390] = {
        name = "Rune Watcher",
        speed = 20,
        premium = false
    },
    [1391] = {
        name = "Rift Watcher",
        speed = 20,
        premium = false
    },
    [1417] = {
        name = "Phant",
        speed = 20,
        premium = true
    },
    [1430] = {
        name = "Shellodon",
        speed = 20,
        premium = true
    },
    [1431] = {
        name = "Singeing Steed",
        speed = 20,
        premium = true
    },
    [1439] = {
        name = "Hyacinth",
        speed = 20,
        premium = false
    },
    [1440] = {
        name = "Peony",
        speed = 20,
        premium = false
    },
    [1441] = {
        name = "Dandelion",
        speed = 20,
        premium = false
    },
    [1446] = {
        name = "Rustwurm",
        speed = 20,
        premium = false
    },
    [1447] = {
        name = "Bogwurm",
        speed = 20,
        premium = false
    },
    [1448] = {
        name = "Gloomwurm",
        speed = 20,
        premium = false
    },
    [1453] = {
        name = "Emerald Raven",
        speed = 20,
        premium = false
    },
    [1454] = {
        name = "Mystic Raven",
        speed = 20,
        premium = false
    },
    [1455] = {
        name = "Radiant Raven",
        speed = 20,
        premium = false
    },
    [1459] = {
        name = "Gloothomotive",
        speed = 20,
        premium = true
    },
    [1491] = {
        name = "Topaz Shrine",
        speed = 20,
        premium = false
    },
    [1492] = {
        name = "Jade Shrine",
        speed = 20,
        premium = false
    },
    [1493] = {
        name = "Obsidian Shrine",
        speed = 20,
        premium = false
    },
    [1526] = {
        name = "Poppy Ibex",
        speed = 20,
        premium = false
    },
    [1527] = {
        name = "Mint Ibex",
        speed = 20,
        premium = false
    },
    [1528] = {
        name = "Cinnamon Ibex",
        speed = 20,
        premium = false
    },
    [1536] = {
        name = "Giant Beaver",
        speed = 20,
        premium = true
    },
    [1577] = {
        name = "Ripptor",
        speed = 20,
        premium = true
    },
    [1578] = {
        name = "Parade Horse",
        speed = 20,
        premium = false
    },
    [1579] = {
        name = "Jousting Horse",
        speed = 20,
        premium = false
    },
    [1580] = {
        name = "Tourney Horse",
        speed = 20,
        premium = false
    },
    [1599] = {
        name = "Mutated Abomination",
        speed = 20,
        premium = true
    },
    [1608] = {
        name = "Tangerine Flecked Koi",
        speed = 20,
        premium = false
    },
    [1609] = {
        name = "Brass Speckled Koi",
        speed = 20,
        premium = false
    },
    [1610] = {
        name = "Ink Spotted Koi",
        speed = 20,
        premium = false
    }
}

function Game.getMountByLookType(lookType)
    local mount = mounts[lookType]
    if not mount then
        return nil
    end

    return {
        lookType = lookType,
        name = mount.name,
        speed = mount.speed,
        premium = mount.premium
    }
end

function Game.getMountByName(name)
    for lookType, mount in pairs(mounts) do
        if mount.name:lower() == name:lower() then
            return {
                lookType = lookType,
                name = mount.name,
                speed = mount.speed,
                premium = mount.premium
            }
        end
    end
    return nil
end

function Game.getMount(param)
    local lookType = tonumber(param)
    if lookType then
        return Game.getMountByLookType(lookType)
    end
    return Game.getMountByName(param)
end

do
    local cachedMounts = {}

    for lookType, mount in pairs(mounts) do
        table.insert(cachedMounts, {
            lookType = lookType,
            name = mount.name,
            speed = mount.speed,
            premium = mount.premium
        })
    end

    function Game.getMounts()
        return cachedMounts
    end
end
