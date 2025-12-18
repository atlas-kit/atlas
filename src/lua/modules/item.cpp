#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaItemCreate(lua_State* L)
{
	// Item(uid)
	uint32_t id = tfs::lua::getNumber<uint32_t>(L, 2);

	if (const auto& item = tfs::lua::getScriptEnv()->getItemByUID(id)) {
		tfs::lua::pushSharedPtr(L, item);
		tfs::lua::setItemMetatable(L, -1, item);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemIsItem(lua_State* L)
{
	// item:isItem()
	if (const auto& thing = tfs::lua::getThing(L, 1)) {
		tfs::lua::pushBoolean(L, thing->asItem() != nullptr);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemHasParent(lua_State* L)
{
	// item:hasParent()
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushBoolean(L, item->hasParent());
	return 1;
}

int luaItemGetParent(lua_State* L)
{
	// item:getParent()
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	const auto& parent = item->getParent();
	if (!parent) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushThing(L, parent);
	return 1;
}

int luaItemGetTopParent(lua_State* L)
{
	// item:getTopParent()
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	const auto& topParent = item->getTopParent();
	if (!topParent) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushThing(L, topParent);
	return 1;
}

int luaItemGetId(lua_State* L)
{
	// item:getId()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getID());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemClone(lua_State* L)
{
	// item:clone()
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	const auto& clone = item->clone();
	if (!clone) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushSharedPtr(L, clone);
	tfs::lua::setItemMetatable(L, -1, clone);
	return 1;
}

int luaItemSplit(lua_State* L)
{
	// item:split([count = 1])
	auto* itemPtr = tfs::lua::getRawSharedPtr<Item>(L, 1);
	if (!itemPtr) {
		lua_pushnil(L);
		return 1;
	}

	const auto& item = *itemPtr;
	if (!item || !item->isStackable()) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t count = std::min<uint16_t>(tfs::lua::getNumber<uint16_t>(L, 2, 1), item->getItemCount());
	uint16_t diff = item->getItemCount() - count;

	const auto& splitItem = item->clone();
	if (!splitItem) {
		lua_pushnil(L);
		return 1;
	}

	splitItem->setItemCount(count);

	const auto env = tfs::lua::getScriptEnv();
	uint32_t uid = env->addThing(item);

	if (item->isRemoved()) {
		env->removeItemByUID(uid);
	}

	const auto& newItem = g_game.transformItem(item, item->getID(), diff);
	if (newItem && newItem != item) {
		env->insertItem(uid, newItem);
	}

	*itemPtr = newItem;

	tfs::lua::pushSharedPtr(L, splitItem);
	tfs::lua::setItemMetatable(L, -1, splitItem);
	return 1;
}

int luaItemRemove(lua_State* L)
{
	// item:remove([count = -1])
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		int32_t count = tfs::lua::getNumber<int32_t>(L, 2, -1);
		tfs::lua::pushBoolean(L, g_game.internalRemoveItem(item, count) == RETURNVALUE_NOERROR);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetUniqueId(lua_State* L)
{
	// item:getUniqueId()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		uint32_t uniqueId = item->getUniqueId();
		if (uniqueId == 0) {
			uniqueId = tfs::lua::getScriptEnv()->addThing(item);
		}
		tfs::lua::pushNumber(L, uniqueId);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetActionId(lua_State* L)
{
	// item:getActionId()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getActionId());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetActionId(lua_State* L)
{
	// item:setActionId(actionId)
	uint16_t actionId = tfs::lua::getNumber<uint16_t>(L, 2);
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		item->setActionId(actionId);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetCount(lua_State* L)
{
	// item:getCount()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getItemCount());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetCharges(lua_State* L)
{
	// item:getCharges()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getCharges());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetFluidType(lua_State* L)
{
	// item:getFluidType()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getFluidType());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetWeight(lua_State* L)
{
	// item:getWeight()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getWeight());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetWorth(lua_State* L)
{
	// item:getWorth()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getWorth());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetSubType(lua_State* L)
{
	// item:getSubType()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(L, item->getSubType());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetName(lua_State* L)
{
	// item:getName()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushString(L, item->getName());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetPluralName(lua_State* L)
{
	// item:getPluralName()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushString(L, item->getPluralName());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetArticle(lua_State* L)
{
	// item:getArticle()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushString(L, item->getArticle());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetPosition(lua_State* L)
{
	// item:getPosition()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushPosition(L, item->getPosition());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetTile(lua_State* L)
{
	// item:getTile()
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	if (const auto& tile = item->getTile()) {
		tfs::lua::pushSharedPtr(L, tile);
		tfs::lua::setMetatable(L, -1, "Tile");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemHasAttribute(lua_State* L)
{
	// item:hasAttribute(key)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	itemAttrTypes attribute;
	if (tfs::lua::isNumber(L, 2)) {
		attribute = tfs::lua::getNumber<itemAttrTypes>(L, 2);
	} else if (lua_isstring(L, 2)) {
		attribute = stringToItemAttribute(tfs::lua::getString(L, 2));
	} else {
		attribute = ITEM_ATTRIBUTE_NONE;
	}

	tfs::lua::pushBoolean(L, item->hasAttribute(attribute));
	return 1;
}

int luaItemGetAttribute(lua_State* L)
{
	// item:getAttribute(key)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	itemAttrTypes attribute;
	if (tfs::lua::isNumber(L, 2)) {
		attribute = tfs::lua::getNumber<itemAttrTypes>(L, 2);
	} else if (lua_isstring(L, 2)) {
		attribute = stringToItemAttribute(tfs::lua::getString(L, 2));
	} else {
		attribute = ITEM_ATTRIBUTE_NONE;
	}

	if (ItemAttributes::isIntAttrType(attribute)) {
		tfs::lua::pushNumber(L, item->getIntAttr(attribute));
	} else if (ItemAttributes::isStrAttrType(attribute)) {
		tfs::lua::pushString(L, item->getStrAttr(attribute));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetAttribute(lua_State* L)
{
	// item:setAttribute(key, value)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	itemAttrTypes attribute = ITEM_ATTRIBUTE_NONE;
	if (tfs::lua::isNumber(L, 2)) {
		attribute = tfs::lua::getNumber<itemAttrTypes>(L, 2);
	} else if (lua_isstring(L, 2)) {
		attribute = stringToItemAttribute(tfs::lua::getString(L, 2));
	}

	if (ItemAttributes::isIntAttrType(attribute)) {
		if (attribute == ITEM_ATTRIBUTE_UNIQUEID) {
			tfs::lua::reportError(L, "Attempt to set protected key \"uid\"");
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		item->setIntAttr(attribute, tfs::lua::getNumber<int32_t>(L, 3));
		tfs::lua::pushBoolean(L, true);
	} else if (ItemAttributes::isStrAttrType(attribute)) {
		item->setStrAttr(attribute, tfs::lua::getString(L, 3));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemRemoveAttribute(lua_State* L)
{
	// item:removeAttribute(key)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	itemAttrTypes attribute = ITEM_ATTRIBUTE_NONE;
	if (tfs::lua::isNumber(L, 2)) {
		attribute = tfs::lua::getNumber<itemAttrTypes>(L, 2);
	} else if (lua_isstring(L, 2)) {
		attribute = stringToItemAttribute(tfs::lua::getString(L, 2));
	}

	bool ret = attribute != ITEM_ATTRIBUTE_UNIQUEID;
	if (ret) {
		item->removeAttribute(attribute);
	} else {
		tfs::lua::reportError(L, "Attempt to erase protected key \"uid\"");
	}
	tfs::lua::pushBoolean(L, ret);
	return 1;
}

int luaItemGetCustomAttribute(lua_State* L)
{
	// item:getCustomAttribute(key)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	const ItemAttributes::CustomAttribute* attr;
	if (tfs::lua::isNumber(L, 2)) {
		attr = item->getCustomAttribute(tfs::lua::getNumber<int64_t>(L, 2));
	} else if (lua_isstring(L, 2)) {
		attr = item->getCustomAttribute(tfs::lua::getString(L, 2));
	} else {
		lua_pushnil(L);
		return 1;
	}

	if (attr) {
		attr->pushToLua(L);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetCustomAttribute(lua_State* L)
{
	// item:setCustomAttribute(key, value)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	std::string key;
	if (tfs::lua::isNumber(L, 2)) {
		key = std::to_string(tfs::lua::getNumber<int64_t>(L, 2));
	} else if (lua_isstring(L, 2)) {
		key = tfs::lua::getString(L, 2);
	} else {
		lua_pushnil(L);
		return 1;
	}

	ItemAttributes::CustomAttribute val;
	if (tfs::lua::isNumber(L, 3)) {
		double tmp = tfs::lua::getNumber<double>(L, 3);
		if (std::floor(tmp) < tmp) {
			val = tmp;
		} else {
			val = static_cast<int64_t>(tmp);
		}
	} else if (lua_isstring(L, 3)) {
		val = tfs::lua::getString(L, 3);
	} else if (lua_isboolean(L, 3)) {
		val = tfs::lua::getBoolean(L, 3);
	} else {
		lua_pushnil(L);
		return 1;
	}

	item->setCustomAttribute(key, val);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaItemRemoveCustomAttribute(lua_State* L)
{
	// item:removeCustomAttribute(key)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	if (tfs::lua::isNumber(L, 2)) {
		tfs::lua::pushBoolean(L, item->removeCustomAttribute(tfs::lua::getNumber<int64_t>(L, 2)));
	} else if (lua_isstring(L, 2)) {
		tfs::lua::pushBoolean(L, item->removeCustomAttribute(tfs::lua::getString(L, 2)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemMoveTo(lua_State* L)
{
	// item:moveTo(position or thing[, flags])
	auto* itemPtr = tfs::lua::getRawSharedPtr<Item>(L, 1);
	if (!itemPtr) {
		lua_pushnil(L);
		return 1;
	}

	const auto& item = *itemPtr;
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	std::shared_ptr<Thing> toThing = nullptr;
	if (lua_isuserdata(L, 2)) {
		const auto const type = tfs::lua::getUserdataType(L, 2);
		switch (type) {
			case tfs::lua::LuaData_Container:
				toThing = tfs::lua::getSharedPtr<Container>(L, 2);
				break;
			case tfs::lua::LuaData_Player:
				toThing = tfs::lua::getSharedPtr<Player>(L, 2);
				break;
			case tfs::lua::LuaData_Tile:
				toThing = tfs::lua::getSharedPtr<Tile>(L, 2);
				break;
			default:
				break;
		}
	} else {
		toThing = g_game.map.getTile(tfs::lua::getPosition(L, 2));
	}

	if (!toThing) {
		lua_pushnil(L);
		return 1;
	}

	if (item->getParent() == toThing) {
		tfs::lua::pushBoolean(L, true);
		return 1;
	}

	uint32_t flags = tfs::lua::getNumber<uint32_t>(
	    L, 3, FLAG_NOLIMIT | FLAG_IGNOREBLOCKITEM | FLAG_IGNOREBLOCKCREATURE | FLAG_IGNORENOTMOVEABLE);

	if (!item->hasParent()) {
		tfs::lua::pushBoolean(L, g_game.internalAddItem(toThing, item, INDEX_WHEREEVER, flags) == RETURNVALUE_NOERROR);
	} else {
		std::shared_ptr<Item> moveItem = nullptr;
		ReturnValue ret = g_game.internalMoveItem(item->getParent(), toThing, INDEX_WHEREEVER, item,
		                                          item->getItemCount(), moveItem, flags);
		if (moveItem) {
			*itemPtr = moveItem;
		}
		tfs::lua::pushBoolean(L, ret == RETURNVALUE_NOERROR);
	}
	return 1;
}

int luaItemTransform(lua_State* L)
{
	// item:transform(itemId[, count/subType = -1])
	auto* itemPtr = tfs::lua::getRawSharedPtr<Item>(L, 1);
	if (!itemPtr) {
		lua_pushnil(L);
		return 1;
	}

	const auto& item = *itemPtr;
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (tfs::lua::isNumber(L, 2)) {
		itemId = tfs::lua::getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(tfs::lua::getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	int32_t subType = tfs::lua::getNumber<int32_t>(L, 3, -1);
	if (item->getID() == itemId && (subType == -1 || subType == item->getSubType())) {
		tfs::lua::pushBoolean(L, true);
		return 1;
	}

	const ItemType& it = Item::items[itemId];
	if (it.stackable) {
		subType = std::min<int32_t>(subType, ITEM_STACK_SIZE);
	}

	const auto env = tfs::lua::getScriptEnv();
	uint32_t uid = env->addThing(item);

	const auto& newItem = g_game.transformItem(item, itemId, subType);
	if (item->isRemoved()) {
		env->removeItemByUID(uid);
	}

	if (newItem && newItem != item) {
		env->insertItem(uid, newItem);
	}

	*itemPtr = newItem;
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaItemDecay(lua_State* L)
{
	// item:decay(decayId)
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		if (tfs::lua::isNumber(L, 2)) {
			item->setDecayTo(tfs::lua::getNumber<int32_t>(L, 2));
		}

		g_game.startDecay(item);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemGetSpecialDescription(lua_State* L)
{
	// item:getSpecialDescription()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushString(L, item->getSpecialDescription());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemHasProperty(lua_State* L)
{
	// item:hasProperty(property)
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		ITEMPROPERTY property = tfs::lua::getNumber<ITEMPROPERTY>(L, 2);
		tfs::lua::pushBoolean(L, item->hasProperty(property));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemIsLoadedFromMap(lua_State* L)
{
	// item:isLoadedFromMap()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushBoolean(L, item->isLoadedFromMap());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetStoreItem(lua_State* L)
{
	// item:setStoreItem(storeItem)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	item->setStoreItem(tfs::lua::getBoolean(L, 2, false));
	return 1;
}

int luaItemIsStoreItem(lua_State* L)
{
	// item:isStoreItem()
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushBoolean(L, item->isStoreItem());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetReflect(lua_State* L)
{
	// item:setReflect(combatType, reflect)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	Reflect reflect{
	    tfs::lua::getField<uint16_t>(L, 3, "percent"),
	    tfs::lua::getField<uint16_t>(L, 3, "chance"),
	};
	lua_pop(L, 2);

	item->setReflect(tfs::lua::getNumber<CombatType_t>(L, 2), reflect);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaItemGetReflect(lua_State* L)
{
	// item:getReflect(combatType[, total = true])
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		const Reflect& reflect =
		    item->getReflect(tfs::lua::getNumber<CombatType_t>(L, 2), tfs::lua::getBoolean(L, 3, true));

		lua_createtable(L, 0, 2);
		tfs::lua::setField(L, "percent", reflect.percent);
		tfs::lua::setField(L, "chance", reflect.chance);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaItemSetBoostPercent(lua_State* L)
{
	// item:setBoostPercent(combatType, percent)
	const auto& item = tfs::lua::getSharedPtr<Item>(L, 1);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	item->setBoostPercent(tfs::lua::getNumber<CombatType_t>(L, 2), tfs::lua::getNumber<uint16_t>(L, 3));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaItemGetBoostPercent(lua_State* L)
{
	// item:getBoostPercent(combatType[, total = true])
	if (const auto& item = tfs::lua::getSharedPtr<Item>(L, 1)) {
		tfs::lua::pushNumber(
		    L, item->getBoostPercent(tfs::lua::getNumber<CombatType_t>(L, 2), tfs::lua::getBoolean(L, 3, true)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerItem(LuaScriptInterface& i)
{
	registerEnum(i, CONST_PROP_BLOCKSOLID);
	registerEnum(i, CONST_PROP_HASHEIGHT);
	registerEnum(i, CONST_PROP_BLOCKPROJECTILE);
	registerEnum(i, CONST_PROP_BLOCKPATH);
	registerEnum(i, CONST_PROP_ISVERTICAL);
	registerEnum(i, CONST_PROP_ISHORIZONTAL);
	registerEnum(i, CONST_PROP_MOVEABLE);
	registerEnum(i, CONST_PROP_IMMOVABLEBLOCKSOLID);
	registerEnum(i, CONST_PROP_IMMOVABLEBLOCKPATH);
	registerEnum(i, CONST_PROP_IMMOVABLENOFIELDBLOCKPATH);
	registerEnum(i, CONST_PROP_NOFIELDBLOCKPATH);
	registerEnum(i, CONST_PROP_SUPPORTHANGABLE);

	registerEnum(i, DECAYING_FALSE);
	registerEnum(i, DECAYING_TRUE);
	registerEnum(i, DECAYING_PENDING);

	i.registerClass("Item", "", luaItemCreate);
	i.registerMetaMethod("Item", "__eq", tfs::lua::luaUserdataCompare);

	i.registerMethod("Item", "isItem", luaItemIsItem);

	i.registerMethod("Item", "hasParent", luaItemHasParent);
	i.registerMethod("Item", "getParent", luaItemGetParent);
	i.registerMethod("Item", "getTopParent", luaItemGetTopParent);

	i.registerMethod("Item", "getId", luaItemGetId);

	i.registerMethod("Item", "clone", luaItemClone);
	i.registerMethod("Item", "split", luaItemSplit);
	i.registerMethod("Item", "remove", luaItemRemove);

	i.registerMethod("Item", "getUniqueId", luaItemGetUniqueId);
	i.registerMethod("Item", "getActionId", luaItemGetActionId);
	i.registerMethod("Item", "setActionId", luaItemSetActionId);

	i.registerMethod("Item", "getCount", luaItemGetCount);
	i.registerMethod("Item", "getCharges", luaItemGetCharges);
	i.registerMethod("Item", "getFluidType", luaItemGetFluidType);
	i.registerMethod("Item", "getWeight", luaItemGetWeight);
	i.registerMethod("Item", "getWorth", luaItemGetWorth);

	i.registerMethod("Item", "getSubType", luaItemGetSubType);

	i.registerMethod("Item", "getName", luaItemGetName);
	i.registerMethod("Item", "getPluralName", luaItemGetPluralName);
	i.registerMethod("Item", "getArticle", luaItemGetArticle);

	i.registerMethod("Item", "getPosition", luaItemGetPosition);
	i.registerMethod("Item", "getTile", luaItemGetTile);

	i.registerMethod("Item", "hasAttribute", luaItemHasAttribute);
	i.registerMethod("Item", "getAttribute", luaItemGetAttribute);
	i.registerMethod("Item", "setAttribute", luaItemSetAttribute);
	i.registerMethod("Item", "removeAttribute", luaItemRemoveAttribute);
	i.registerMethod("Item", "getCustomAttribute", luaItemGetCustomAttribute);
	i.registerMethod("Item", "setCustomAttribute", luaItemSetCustomAttribute);
	i.registerMethod("Item", "removeCustomAttribute", luaItemRemoveCustomAttribute);

	i.registerMethod("Item", "moveTo", luaItemMoveTo);
	i.registerMethod("Item", "transform", luaItemTransform);
	i.registerMethod("Item", "decay", luaItemDecay);

	i.registerMethod("Item", "getSpecialDescription", luaItemGetSpecialDescription);

	i.registerMethod("Item", "hasProperty", luaItemHasProperty);
	i.registerMethod("Item", "isLoadedFromMap", luaItemIsLoadedFromMap);

	i.registerMethod("Item", "setStoreItem", luaItemSetStoreItem);
	i.registerMethod("Item", "isStoreItem", luaItemIsStoreItem);

	i.registerMethod("Item", "setReflect", luaItemSetReflect);
	i.registerMethod("Item", "getReflect", luaItemGetReflect);

	i.registerMethod("Item", "setBoostPercent", luaItemSetBoostPercent);
	i.registerMethod("Item", "getBoostPercent", luaItemGetBoostPercent);
}
