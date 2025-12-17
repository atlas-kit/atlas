#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaCreateLoot(lua_State* L)
{
	// Loot() will create a new loot item
	tfs::lua::pushUserdata(L, new Loot);
	tfs::lua::setMetatable(L, -1, "Loot");
	return 1;
}

int luaDeleteLoot(lua_State* L)
{
	// loot:delete() loot:__gc()
	Loot** lootPtr = tfs::lua::getRawUserdata<Loot>(L, 1);
	if (lootPtr && *lootPtr) {
		delete *lootPtr;
		*lootPtr = nullptr;
	}
	return 0;
}

int luaLootSetId(lua_State* L)
{
	// loot:setId(id or name)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		if (tfs::lua::isNumber(L, 2)) {
			loot->lootBlock.id = tfs::lua::getNumber<uint16_t>(L, 2);
		} else {
			auto name = tfs::lua::getString(L, 2);
			const auto&& [it, end] = Item::items.nameToItems.equal_range(boost::algorithm::to_lower_copy(name));

			if (it == Item::items.nameToItems.cend()) {
				std::cout << "[Warning - Loot:setId] Unknown loot item \"" << name << "\".\n";
				tfs::lua::pushBoolean(L, false);
				return 1;
			}

			if (std::next(it) != end) {
				std::cout << "[Warning - Loot:setId] Non-unique loot item \"" << name << "\".\n";
				tfs::lua::pushBoolean(L, false);
				return 1;
			}

			loot->lootBlock.id = it->second;
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootSetSubType(lua_State* L)
{
	// loot:setSubType(type)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		loot->lootBlock.subType = tfs::lua::getNumber<uint16_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootSetChance(lua_State* L)
{
	// loot:setChance(chance)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		loot->lootBlock.chance = tfs::lua::getNumber<uint32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootSetMaxCount(lua_State* L)
{
	// loot:setMaxCount(max)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		loot->lootBlock.countmax = tfs::lua::getNumber<uint32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootSetActionId(lua_State* L)
{
	// loot:setActionId(actionid)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		loot->lootBlock.actionId = tfs::lua::getNumber<uint32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootSetDescription(lua_State* L)
{
	// loot:setDescription(desc)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		loot->lootBlock.text = tfs::lua::getString(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaLootAddChildLoot(lua_State* L)
{
	// loot:addChildLoot(loot)
	Loot* loot = tfs::lua::getUserdata<Loot>(L, 1);
	if (loot) {
		Loot* childLoot = tfs::lua::getUserdata<Loot>(L, 2);
		if (childLoot) {
			loot->lootBlock.childLoot.push_back(childLoot->lootBlock);
			tfs::lua::pushBoolean(L, true);
		} else {
			tfs::lua::pushBoolean(L, false);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerLoot(LuaScriptInterface& i)
{
	i.registerClass("Loot", "", luaCreateLoot);
	i.registerMetaMethod("Loot", "__gc", luaDeleteLoot);
	i.registerMethod("Loot", "delete", luaDeleteLoot);

	i.registerMethod("Loot", "setId", luaLootSetId);
	i.registerMethod("Loot", "setMaxCount", luaLootSetMaxCount);
	i.registerMethod("Loot", "setSubType", luaLootSetSubType);
	i.registerMethod("Loot", "setChance", luaLootSetChance);
	i.registerMethod("Loot", "setActionId", luaLootSetActionId);
	i.registerMethod("Loot", "setDescription", luaLootSetDescription);
	i.registerMethod("Loot", "addChildLoot", luaLootAddChildLoot);
}
