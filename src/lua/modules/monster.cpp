#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaMonsterCreate(lua_State* L)
{
	// Monster(id or userdata)
	std::shared_ptr<Monster> monster = nullptr;
	if (tfs::lua::isNumber(L, 2)) {
		monster = g_game.getMonsterByID(tfs::lua::getNumber<uint32_t>(L, 2));
	} else if (lua_isuserdata(L, 2)) {
		if (tfs::lua::getUserdataType(L, 2) != tfs::lua::LuaData_Monster) {
			lua_pushnil(L);
			return 1;
		}
		monster = tfs::lua::getSharedPtr<Monster>(L, 2);
	}

	if (monster) {
		tfs::lua::pushSharedPtr(L, monster);
		tfs::lua::setMetatable(L, -1, "Monster");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsMonster(lua_State* L)
{
	// monster:isMonster()
	if (const auto& creature = tfs::lua::getCreature(L, 1)) {
		tfs::lua::pushBoolean(L, creature->getMonster() != nullptr);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterGetId(lua_State* L)
{
	// monster:getId()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		// Set monster id if it's not set yet (only for onSpawn event)
		if (tfs::lua::getScriptEnv()->getScriptId() == tfs::events::getScriptId(EventInfoId::MONSTER_ONSPAWN)) {
			monster->setID();
		}

		tfs::lua::pushNumber(L, monster->getID());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterGetType(lua_State* L)
{
	// monster:getType()
	if (const auto& monster = tfs::lua::getSharedPtr<const Monster>(L, 1)) {
		tfs::lua::pushUserdata(L, monster->mType);
		tfs::lua::setMetatable(L, -1, "MonsterType");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterRename(lua_State* L)
{
	// monster:rename(name[, nameDescription])
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	monster->setName(tfs::lua::getString(L, 2));
	if (lua_gettop(L) >= 3) {
		monster->setNameDescription(tfs::lua::getString(L, 3));
	}

	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaMonsterGetSpawnPosition(lua_State* L)
{
	// monster:getSpawnPosition()
	if (const auto& monster = tfs::lua::getSharedPtr<const Monster>(L, 1)) {
		tfs::lua::pushPosition(L, monster->getMasterPos());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsInSpawnRange(lua_State* L)
{
	// monster:isInSpawnRange([position])
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushBoolean(
		    L, monster->isInSpawnRange(lua_gettop(L) >= 2 ? tfs::lua::getPosition(L, 2) : monster->getPosition()));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsIdle(lua_State* L)
{
	// monster:isIdle()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushBoolean(L, monster->getIdleStatus());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSetIdle(lua_State* L)
{
	// monster:setIdle(idle)
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	monster->setIdle(tfs::lua::getBoolean(L, 2));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaMonsterIsTarget(lua_State* L)
{
	// monster:isTarget(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		tfs::lua::pushBoolean(L, monster->isTarget(creature));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsOpponent(lua_State* L)
{
	// monster:isOpponent(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		tfs::lua::pushBoolean(L, monster->isOpponent(creature));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsFriend(lua_State* L)
{
	// monster:isFriend(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		tfs::lua::pushBoolean(L, monster->isFriend(creature));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterAddFriend(lua_State* L)
{
	// monster:addFriend(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		monster->addFriend(creature);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterRemoveFriend(lua_State* L)
{
	// monster:removeFriend(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		monster->removeFriend(creature);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterGetFriendList(lua_State* L)
{
	// monster:getFriendList()
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	const auto& friendList = monster->getFriendList();
	lua_createtable(L, friendList.size(), 0);

	int index = 0;
	for (const auto& creature : friendList | tfs::views::lock_weak_ptrs) {
		tfs::lua::pushSharedPtr(L, creature);
		tfs::lua::setCreatureMetatable(L, -1, creature);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterGetFriendCount(lua_State* L)
{
	// monster:getFriendCount()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushNumber(L, monster->getFriendList().size());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterAddTarget(lua_State* L)
{
	// monster:addTarget(creature[, pushFront = false])
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	const auto& creature = tfs::lua::getCreature(L, 2);
	if (!creature) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	bool pushFront = tfs::lua::getBoolean(L, 3, false);
	monster->addTarget(creature, pushFront);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaMonsterRemoveTarget(lua_State* L)
{
	// monster:removeTarget(creature)
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	const auto& creature = tfs::lua::getCreature(L, 2);
	if (!creature) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	monster->removeTarget(creature);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaMonsterGetTargetList(lua_State* L)
{
	// monster:getTargetList()
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	const auto& targetList = monster->getTargetList();
	lua_createtable(L, targetList.size(), 0);

	int index = 0;
	for (const auto& creature : targetList | tfs::views::lock_weak_ptrs) {
		tfs::lua::pushSharedPtr(L, creature);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterGetTargetCount(lua_State* L)
{
	// monster:getTargetCount()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushNumber(L, monster->getTargetList().size());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSelectTarget(lua_State* L)
{
	// monster:selectTarget(creature)
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		const auto& creature = tfs::lua::getCreature(L, 2);
		if (!creature) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		tfs::lua::pushBoolean(L, monster->selectTarget(creature));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSearchTarget(lua_State* L)
{
	// monster:searchTarget([searchType = TARGETSEARCH_DEFAULT])
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		TargetSearchType_t searchType = tfs::lua::getNumber<TargetSearchType_t>(L, 2, TARGETSEARCH_DEFAULT);
		tfs::lua::pushBoolean(L, monster->searchTarget(searchType));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterIsWalkingToSpawn(lua_State* L)
{
	// monster:isWalkingToSpawn()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushBoolean(L, monster->isWalkingToSpawn());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterWalkToSpawn(lua_State* L)
{
	// monster:walkToSpawn()
	if (const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1)) {
		tfs::lua::pushBoolean(L, monster->walkToSpawn());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterHasIcon(lua_State* L)
{
	// monster:hasSpecialIcon(iconId)
	if (const auto& monster = tfs::lua::getSharedPtr<const Monster>(L, 1)) {
		auto iconId = tfs::lua::getNumber<MonsterIcon_t>(L, 2);
		tfs::lua::pushBoolean(L, monster->getSpecialIcons().contains(iconId));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSetIcon(lua_State* L)
{
	// monster:setSpecialIcon(iconId, value)
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	auto iconId = tfs::lua::getNumber<MonsterIcon_t>(L, 2);
	if (iconId > MONSTER_ICON_LAST) {
		tfs::lua::reportError(L, "Invalid Monster Icon Id");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	monster->getSpecialIcons()[iconId] = tfs::lua::getNumber<uint16_t>(L, 3);
	monster->updateIcons();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaMonsterGetIcon(lua_State* L)
{
	// monster:getSpecialIcon(iconId)
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	auto iconId = tfs::lua::getNumber<MonsterIcon_t>(L, 2);
	const auto& icons = monster->getSpecialIcons();
	auto it = icons.find(iconId);
	if (it != icons.end()) {
		tfs::lua::pushNumber(L, it->second);
	} else {
		tfs::lua::pushNumber(L, 0);
	}
	return 1;
}

int luaMonsterRemoveIcon(lua_State* L)
{
	// monster:removeSpecialIcon(iconId)
	const auto& monster = tfs::lua::getSharedPtr<Monster>(L, 1);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	auto iconId = tfs::lua::getNumber<MonsterIcon_t>(L, 2);
	auto& icons = monster->getSpecialIcons();
	auto it = icons.find(iconId);
	if (it != icons.end()) {
		icons.erase(it);
		monster->updateIcons();
		tfs::lua::pushBoolean(L, true);
	} else {
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

} // namespace

void tfs::lua::registerMonster(LuaScriptInterface& i)
{
	i.registerClass("Monster", "Creature", luaMonsterCreate);
	i.registerMetaMethod("Monster", "__eq", tfs::lua::luaUserdataCompare);

	i.registerMethod("Monster", "isMonster", luaMonsterIsMonster);

	i.registerMethod("Monster", "getId", luaMonsterGetId);
	i.registerMethod("Monster", "getType", luaMonsterGetType);

	i.registerMethod("Monster", "rename", luaMonsterRename);

	i.registerMethod("Monster", "getSpawnPosition", luaMonsterGetSpawnPosition);
	i.registerMethod("Monster", "isInSpawnRange", luaMonsterIsInSpawnRange);

	i.registerMethod("Monster", "isIdle", luaMonsterIsIdle);
	i.registerMethod("Monster", "setIdle", luaMonsterSetIdle);

	i.registerMethod("Monster", "isTarget", luaMonsterIsTarget);
	i.registerMethod("Monster", "isOpponent", luaMonsterIsOpponent);
	i.registerMethod("Monster", "isFriend", luaMonsterIsFriend);

	i.registerMethod("Monster", "addFriend", luaMonsterAddFriend);
	i.registerMethod("Monster", "removeFriend", luaMonsterRemoveFriend);
	i.registerMethod("Monster", "getFriendList", luaMonsterGetFriendList);
	i.registerMethod("Monster", "getFriendCount", luaMonsterGetFriendCount);

	i.registerMethod("Monster", "addTarget", luaMonsterAddTarget);
	i.registerMethod("Monster", "removeTarget", luaMonsterRemoveTarget);
	i.registerMethod("Monster", "getTargetList", luaMonsterGetTargetList);
	i.registerMethod("Monster", "getTargetCount", luaMonsterGetTargetCount);

	i.registerMethod("Monster", "selectTarget", luaMonsterSelectTarget);
	i.registerMethod("Monster", "searchTarget", luaMonsterSearchTarget);

	i.registerMethod("Monster", "isWalkingToSpawn", luaMonsterIsWalkingToSpawn);
	i.registerMethod("Monster", "walkToSpawn", luaMonsterWalkToSpawn);

	i.registerMethod("Monster", "hasSpecialIcon", luaMonsterHasIcon);
	i.registerMethod("Monster", "setSpecialIcon", luaMonsterSetIcon);
	i.registerMethod("Monster", "getSpecialIcon", luaMonsterGetIcon);
	i.registerMethod("Monster", "removeSpecialIcon", luaMonsterRemoveIcon);
}
