-- must match PlayerStorageKeys in storages.lua, change accordingly if modified
local CURRENT_MOUNT = 60000
local RANDOMIZE_MOUNT = 60001
local OUTFITS_BASE = 600000
local MOUNTS_BASE = 610000

function onUpdateDatabase()
    print("> Updating database to version 38 (revert outfits/mounts to storages)")

    local tx = DBTransaction()
    if not tx.begin() then
        return false
    end

    local query = DBInsert("INSERT INTO `player_storage` (`player_id`, `key`, `value`) VALUES ")
    local rows = 0

    do
        local resultId = db.storeQuery("SELECT `player_id`, `outfit_id`, `addons` FROM `player_outfits`")
        if resultId then
            repeat
                local playerId = result.getNumber(resultId, "player_id")
                local outfitId = result.getNumber(resultId, "outfit_id")
                local addons = result.getNumber(resultId, "addons")

                local storageKey = OUTFITS_BASE + outfitId
                query:addRow(string.format("%d, %d, %d", playerId, storageKey, addons))
                rows = rows + 1
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

                local storageKey = MOUNTS_BASE + mountId
                query:addRow(string.format("%d, %d, %d", playerId, storageKey, 1))
                rows = rows + 1
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
                    query:addRow(string.format("%d, %d, %d", playerId, CURRENT_MOUNT, currentMount))
                    rows = rows + 1
                end

                if randomizeMount > 0 then
                    query:addRow(string.format("%d, %d, %d", playerId, RANDOMIZE_MOUNT, randomizeMount))
                    rows = rows + 1
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
