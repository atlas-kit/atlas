Tools = Tools or {}

function onUseKitchenKnife(player, item, fromPosition, target, toPosition, isHotkey)
    if not table.contains({2566, 10511, 10515}, item.itemid) then
        return false
    end

    if table.contains(Tools.decoratedCakeFruits, target.itemid) and player:removeItem(6278, 1) then
        target:remove(1)
        player:addItem(6279, 1)
        player:getPosition():sendMagicEffect(CONST_ME_MAGIC_GREEN)
        return true
    end

    return false
end

Tools.onUseKitchenKnife = onUseKitchenKnife
