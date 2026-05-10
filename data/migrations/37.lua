-- must match PlayerStorageKeys in storages.lua, change accordingly if modified
local CURRENT_MOUNT = 60000
local RANDOMIZE_MOUNT = 60001
local OUTFITS_BASE = 600000
local MOUNTS_BASE = 610000
local OLD_MOUNT_ID_TO_LOOKTYPE = {
    [1] = 368, [2] = 369, [3] = 370, [4] = 371, [5] = 372, [6] = 373, [7] = 374, [8] = 375, [9] = 376, [10] = 377,
    [11] = 378, [12] = 379, [13] = 387, [14] = 388, [15] = 389, [16] = 390, [17] = 392, [18] = 401, [19] = 402, [20] = 405,
    [21] = 406, [22] = 421, [23] = 426, [24] = 427, [25] = 437, [26] = 438, [27] = 447, [28] = 450, [29] = 502, [30] = 503,
    [31] = 506, [32] = 515, [33] = 521, [34] = 522, [35] = 526, [36] = 546, [37] = 547, [38] = 548, [39] = 559, [40] = 571,
    [41] = 572, [42] = 580, [43] = 606, [44] = 621, [45] = 622, [46] = 624, [47] = 626, [48] = 627, [49] = 628, [50] = 629,
    [51] = 630, [52] = 631, [53] = 644, [54] = 647, [55] = 648, [56] = 649, [57] = 650, [58] = 651, [59] = 669, [60] = 670,
    [61] = 671, [62] = 672, [63] = 673, [64] = 674, [65] = 688, [66] = 689, [67] = 690, [68] = 691, [69] = 692, [70] = 693,
    [71] = 682, [72] = 685, [73] = 686, [74] = 687, [75] = 726, [76] = 727, [77] = 728, [78] = 734, [79] = 735, [80] = 736,
    [81] = 738, [82] = 739, [83] = 740, [84] = 761, [85] = 762, [86] = 763, [87] = 848, [88] = 849, [89] = 850, [90] = 851,
    [91] = 868, [92] = 869, [93] = 870, [94] = 883, [95] = 886, [96] = 887, [97] = 888, [98] = 889, [99] = 890, [100] = 901,
    [101] = 902, [102] = 903, [103] = 905, [104] = 906, [105] = 907, [106] = 937, [107] = 950, [108] = 951, [109] = 952, [110] = 959,
    [111] = 960, [112] = 961, [113] = 1017, [114] = 1018, [115] = 1019, [116] = 1025, [117] = 1026, [118] = 1027, [119] = 1049, [120] = 1052,
    [121] = 1053, [122] = 1054, [123] = 1091, [124] = 1092, [125] = 1093, [126] = 1101, [127] = 1104, [128] = 1105, [129] = 1106, [130] = 1150,
    [131] = 1151, [132] = 1163, [133] = 1164, [134] = 1165, [135] = 1167, [136] = 1168, [137] = 1169, [138] = 1179, [139] = 1180, [140] = 1181,
    [141] = 1183, [142] = 1184, [143] = 1185, [144] = 1191, [145] = 1208, [146] = 1209, [147] = 1229, [148] = 1230, [149] = 1231, [150] = 1232,
    [151] = 1233, [152] = 1234, [153] = 1247, [154] = 1248, [155] = 1249, [156] = 1257, [157] = 1258, [158] = 1259, [159] = 1264, [160] = 1265,
    [161] = 1266, [162] = 1269, [163] = 1281, [164] = 1284, [165] = 1285, [166] = 1286, [167] = 1321, [168] = 1324, [169] = 1325, [170] = 1326,
    [171] = 1333, [172] = 1334, [173] = 1335, [174] = 1336, [175] = 1363, [176] = 1379, [177] = 1380, [178] = 1381, [179] = 1389, [180] = 1390,
    [181] = 1391, [182] = 1417, [183] = 1430, [184] = 1431, [185] = 1439, [186] = 1440, [187] = 1441, [188] = 1446, [189] = 1447, [190] = 1448,
    [191] = 1453, [192] = 1454, [193] = 1455, [194] = 1459, [195] = 1491, [196] = 1492, [197] = 1493, [198] = 1526, [199] = 1527, [200] = 1528,
    [201] = 1536, [202] = 1577, [203] = 1578, [204] = 1579, [205] = 1580, [206] = 1599, [207] = 1608, [208] = 1609, [209] = 1610,
}

local function getMountLookType(mountId)
    return OLD_MOUNT_ID_TO_LOOKTYPE[mountId]
end

function onUpdateDatabase()
    print("> Updating database to version 38 (revert outfits/mounts to storages)")

    local tx = DBTransaction()
    if not tx.begin() then
        return false
    end

    local query = DBInsert("INSERT INTO `player_storage` (`player_id`, `key`, `value`) VALUES ")
    local rows = 0

    local function addStorageRow(playerId, storageKey, value)
        if not query:addRow(string.format("%d, %d, %d", playerId, storageKey, value)) then
            return false
        end

        rows = rows + 1
        return true
    end

    do
        local resultId = db.storeQuery("SELECT `player_id`, `outfit_id`, `addons` FROM `player_outfits`")
        if resultId then
            repeat
                local playerId = result.getNumber(resultId, "player_id")
                local outfitId = result.getNumber(resultId, "outfit_id")
                local addons = result.getNumber(resultId, "addons")

                local storageKey = OUTFITS_BASE + outfitId
                if not addStorageRow(playerId, storageKey, addons) then
                    result.free(resultId)
                    tx.rollback()
                    return false
                end
            until not result.next(resultId)
            result.free(resultId)
        end
    end

    do
        local resultId = db.storeQuery("SELECT `player_id`, `mount_id` FROM `player_mounts`")
        if resultId then
            repeat
                local playerId = result.getNumber(resultId, "player_id")
                local mountId = result.getNumber(resultId, "mount_id")
                local mountLookType = getMountLookType(mountId)
                if mountLookType then
                    local storageKey = MOUNTS_BASE + mountLookType
                    if not addStorageRow(playerId, storageKey, 1) then
                        result.free(resultId)
                        tx.rollback()
                        return false
                    end
                end
            until not result.next(resultId)
            result.free(resultId)
        end
    end

    -- Migrate currentmount and randomizemount from players table
    do
        local resultId = db.storeQuery(
            "SELECT `id`, `currentmount`, `randomizemount` FROM `players` WHERE `currentmount` > 0 OR `randomizemount` > 0")
        if resultId then
            repeat
                local playerId = result.getNumber(resultId, "id")
                local currentMount = result.getNumber(resultId, "currentmount")
                local randomizeMount = result.getNumber(resultId, "randomizemount")

                if currentMount > 0 then
                    local currentMountLookType = getMountLookType(currentMount)
                    if currentMountLookType then
                        if not addStorageRow(playerId, CURRENT_MOUNT, currentMountLookType) then
                            result.free(resultId)
                            tx.rollback()
                            return false
                        end
                    end
                end

                if randomizeMount > 0 then
                    if not addStorageRow(playerId, RANDOMIZE_MOUNT, randomizeMount) then
                        result.free(resultId)
                        tx.rollback()
                        return false
                    end
                end
            until not result.next(resultId)
            result.free(resultId)
        end
    end

    if rows > 0 and not query:execute() then
        tx.rollback()
        return false
    end

    if not db.query("DROP TABLE IF EXISTS `player_outfits`") then
        tx.rollback()
        return false
    end

    if not db.query("DROP TABLE IF EXISTS `player_mounts`") then
        tx.rollback()
        return false
    end

    if not db.query("ALTER TABLE `players` DROP COLUMN `currentmount`, DROP COLUMN `randomizemount`") then
        tx.rollback()
        return false
    end

    return tx.commit()
end
