// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "luascript.h"

#include "bed.h"
#include "chat.h"
#include "combat.h"
#include "configmanager.h"
#include "databasemanager.h"
#include "databasetasks.h"
#include "events.h"
#include "game.h"
#include "globalevent.h"
#include "housetile.h"
#include "iologindata.h"
#include "iomapserialize.h"
#include "iomarket.h"
#include "item.h"
#include "lua/api.h"
#include "lua/env.h"
#include "lua/error.h"
#include "lua/meta.h"
#include "lua/variant.h"
#include "matrixarea.h"
#include "movement.h"
#include "outfit.h"
#include "party.h"
#include "player.h"
#include "podium.h"
#include "protocolstatus.h"
#include "scheduler.h"
#include "script.h"
#include "spells.h"
#include "teleport.h"
#include "weapons.h"

#include <string>

extern Chat* g_chat;
extern Game g_game;
extern GlobalEvents* g_globalEvents;
extern Monsters g_monsters;
extern Vocations g_vocations;
extern Spells* g_spells;
extern Actions* g_actions;
extern TalkActions* g_talkActions;
extern CreatureEvents* g_creatureEvents;
extern MoveEvents* g_moveEvents;
extern GlobalEvents* g_globalEvents;
extern Scripts* g_scripts;
extern Weapons* g_weapons;

LuaEnvironment g_luaEnvironment;

namespace {

constexpr int32_t EVENT_ID_LOADING = 1;

bool getArea(lua_State* L, std::vector<uint32_t>& vec, uint32_t& rows)
{
	lua_pushnil(L);
	for (rows = 0; lua_next(L, -2) != 0; ++rows) {
		if (!lua_istable(L, -1)) {
			return false;
		}

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (!tfs::lua::isNumber(L, -1)) {
				return false;
			}
			vec.push_back(tfs::lua::getNumber<uint32_t>(L, -1));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	return (rows != 0);
}

} // namespace

LuaScriptInterface::LuaScriptInterface(std::string interfaceName) : interfaceName(std::move(interfaceName))
{
	if (!g_luaEnvironment.getLuaState()) {
		g_luaEnvironment.initState();
	}
}

LuaScriptInterface::~LuaScriptInterface() { closeState(); }

bool LuaScriptInterface::reInitState()
{
	g_luaEnvironment.clearCombatObjects(this);
	g_luaEnvironment.clearAreaObjects(this);

	closeState();
	return initState();
}

int32_t LuaScriptInterface::loadFile(const std::string& file, const std::shared_ptr<Npc>& npc /* = nullptr*/)
{
	// loads file as a chunk at stack top
	int ret = luaL_loadfile(L, file.data());
	if (ret != 0) {
		lastLuaError = tfs::lua::popString(L);
		return -1;
	}

	// check that it is loaded as a function
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return -1;
	}

	loadingFile = file;

	if (!tfs::lua::reserveScriptEnv()) {
		lua_pop(L, 1);
		return -1;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(EVENT_ID_LOADING, this);
	env->setNpc(npc);

	// execute it
	ret = tfs::lua::protectedCall(L, 0, 0);
	if (ret != 0) {
		tfs::lua::reportError(tfs::lua::popString(L));
		tfs::lua::resetScriptEnv();
		return -1;
	}

	tfs::lua::resetScriptEnv();
	return 0;
}

int32_t LuaScriptInterface::getEvent(std::string_view eventName)
{
	// get our events table
	lua_rawgeti(L, LUA_REGISTRYINDEX, eventTableRef);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return -1;
	}

	// get current event function pointer
	lua_getglobal(L, eventName.data());
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return -1;
	}

	// save in our events table
	lua_pushvalue(L, -1);
	lua_rawseti(L, -3, runningEventId);
	lua_pop(L, 2);

	// reset global value of this event
	lua_pushnil(L);
	lua_setglobal(L, eventName.data());

	cacheFiles[runningEventId] = std::format("{:s}:{:s}", loadingFile, eventName);
	return runningEventId++;
}

int32_t LuaScriptInterface::getEvent()
{
	// check if function is on the stack
	if (!lua_isfunction(L, -1)) {
		return -1;
	}

	// get our events table
	lua_rawgeti(L, LUA_REGISTRYINDEX, eventTableRef);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return -1;
	}

	// save in our events table
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, runningEventId);
	lua_pop(L, 2);

	cacheFiles[runningEventId] = loadingFile + ":callback";
	return runningEventId++;
}

int32_t LuaScriptInterface::getMetaEvent(const std::string& globalName, const std::string& eventName)
{
	// get our events table
	lua_rawgeti(L, LUA_REGISTRYINDEX, eventTableRef);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return -1;
	}

	// get current event function pointer
	lua_getglobal(L, globalName.data());
	lua_getfield(L, -1, eventName.data());
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 3);
		return -1;
	}

	// save in our events table
	lua_pushvalue(L, -1);
	lua_rawseti(L, -4, runningEventId);
	lua_pop(L, 1);

	// reset global value of this event
	lua_pushnil(L);
	lua_setfield(L, -2, eventName.data());
	lua_pop(L, 2);

	cacheFiles[runningEventId] = loadingFile + ":" + globalName + "@" + eventName;
	return runningEventId++;
}

const std::string& LuaScriptInterface::getFileById(int32_t scriptId)
{
	if (scriptId == EVENT_ID_LOADING) {
		return loadingFile;
	}

	auto it = cacheFiles.find(scriptId);
	if (it == cacheFiles.end()) {
		static const std::string& unk = "(Unknown scriptfile)";
		return unk;
	}
	return it->second;
}

bool LuaScriptInterface::pushFunction(int32_t functionId)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, eventTableRef);
	if (!lua_istable(L, -1)) {
		return false;
	}

	lua_rawgeti(L, -1, functionId);
	lua_replace(L, -2);
	return lua_isfunction(L, -1);
}

bool LuaScriptInterface::initState()
{
	if (L = g_luaEnvironment.getLuaState(); !L) {
		return false;
	}

	lua_newtable(L);
	eventTableRef = luaL_ref(L, LUA_REGISTRYINDEX);
	runningEventId = EVENT_ID_USER;
	return true;
}

bool LuaScriptInterface::closeState()
{
	if (!g_luaEnvironment.getLuaState() || !L) {
		return false;
	}

	cacheFiles.clear();
	if (eventTableRef != -1) {
		luaL_unref(L, LUA_REGISTRYINDEX, eventTableRef);
		eventTableRef = -1;
	}

	L = nullptr;
	return true;
}

bool LuaScriptInterface::callFunction(int params)
{
	bool result = false;
	int size = lua_gettop(L);
	if (tfs::lua::protectedCall(L, params, 1) != 0) {
		tfs::lua::reportError(tfs::lua::getString(L, -1));
	} else {
		result = tfs::lua::getBoolean(L, -1);
	}

	lua_pop(L, 1);
	if ((lua_gettop(L) + params + 1) != size) {
		tfs::lua::reportError("Stack size changed!");
	}

	tfs::lua::resetScriptEnv();
	return result;
}

void LuaScriptInterface::callVoidFunction(int params)
{
	int size = lua_gettop(L);
	if (tfs::lua::protectedCall(L, params, 0) != 0) {
		tfs::lua::reportError(tfs::lua::popString(L));
	}

	if ((lua_gettop(L) + params + 1) != size) {
		tfs::lua::reportError("Stack size changed!");
	}

	tfs::lua::resetScriptEnv();
}

void LuaScriptInterface::registerClass(std::string_view className, std::string_view baseClass,
                                       lua_CFunction newFunction = nullptr)
{
	// className = {}
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setglobal(L, className.data());
	int methods = lua_gettop(L);

	// methodsTable = {}
	lua_newtable(L);
	int methodsTable = lua_gettop(L);

	if (newFunction) {
		// className.__call = newFunction
		lua_pushcfunction(L, newFunction);
		lua_setfield(L, methodsTable, "__call");
	}

	uint32_t parents = 0;
	if (!baseClass.empty()) {
		lua_getglobal(L, baseClass.data());
		lua_rawgeti(L, -1, 'p');
		parents = tfs::lua::getNumber<uint32_t>(L, -1) + 1;
		lua_pop(L, 1);
		lua_setfield(L, methodsTable, "__index");
	}

	// setmetatable(className, methodsTable)
	lua_setmetatable(L, methods);

	// className.metatable = {}
	luaL_newmetatable(L, className.data());
	int metatable = lua_gettop(L);

	// className.metatable.__metatable = className
	lua_pushvalue(L, methods);
	lua_setfield(L, metatable, "__metatable");

	// className.metatable.__index = className
	lua_pushvalue(L, methods);
	lua_setfield(L, metatable, "__index");

	// className.metatable['h'] = hash
	tfs::lua::pushNumber(L, std::hash<std::string_view>()(className));
	lua_rawseti(L, metatable, 'h');

	// className.metatable['p'] = parents
	tfs::lua::pushNumber(L, parents);
	lua_rawseti(L, metatable, 'p');

	// className.metatable['t'] = type
	if (className == "Item") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Item);
	} else if (className == "Container") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Container);
	} else if (className == "Teleport") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Teleport);
	} else if (className == "Podium") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Podium);
	} else if (className == "Player") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Player);
	} else if (className == "Monster") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Monster);
	} else if (className == "Npc") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Npc);
	} else if (className == "Tile") {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Tile);
	} else {
		tfs::lua::pushNumber(L, tfs::lua::LuaData_Unknown);
	}
	lua_rawseti(L, metatable, 't');

	// pop className, className.metatable
	lua_pop(L, 2);
}

void LuaScriptInterface::registerTable(std::string_view tableName)
{
	// _G[tableName] = {}
	lua_newtable(L);
	lua_setglobal(L, tableName.data());
}

void LuaScriptInterface::registerMetaMethod(std::string_view className, std::string_view methodName, lua_CFunction func)
{
	// className.metatable.methodName = func
	luaL_getmetatable(L, className.data());
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, methodName.data());

	// pop className.metatable
	lua_pop(L, 1);
}

void LuaScriptInterface::registerMethod(std::string_view globalName, std::string_view methodName, lua_CFunction func)
{
	// globalName.methodName = func
	lua_getglobal(L, globalName.data());
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, methodName.data());

	// pop globalName
	lua_pop(L, 1);
}

void LuaScriptInterface::registerGlobalMethod(std::string_view functionName, lua_CFunction func)
{
	// _G[functionName] = func
	lua_pushcfunction(L, func);
	lua_setglobal(L, functionName.data());
}

void LuaScriptInterface::registerVariable(std::string_view tableName, std::string_view name, lua_Number value)
{
	// tableName.name = value
	lua_getglobal(L, tableName.data());
	tfs::lua::setField(L, name.data(), value);

	// pop tableName
	lua_pop(L, 1);
}

void LuaScriptInterface::registerGlobalVariable(std::string_view name, lua_Number value)
{
	// _G[name] = value
	tfs::lua::pushNumber(L, value);
	lua_setglobal(L, name.data());
}

void LuaScriptInterface::registerGlobalBoolean(std::string_view name, bool value)
{
	// _G[name] = value
	tfs::lua::pushBoolean(L, value);
	lua_setglobal(L, name.data());
}

#undef registerEnum
#undef registerEnumIn

int LuaScriptInterface::luaDoPlayerAddItem(lua_State* L)
{
	// doPlayerAddItem(cid, itemid, <optional: default: 1> count/subtype, <optional: default: 1> canDropOnMap)
	// doPlayerAddItem(cid, itemid, <optional: default: 1> count, <optional: default: 1> canDropOnMap, <optional:
	// default: 1>subtype)
	const auto& player = tfs::lua::getPlayer(L, 1);
	if (!player) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_PLAYER_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	uint16_t itemId = tfs::lua::getNumber<uint16_t>(L, 2);
	int32_t count = tfs::lua::getNumber<int32_t>(L, 3, 1);
	bool canDropOnMap = tfs::lua::getBoolean(L, 4, true);
	uint16_t subType = tfs::lua::getNumber<uint16_t>(L, 5, 1);

	const ItemType& it = Item::items[itemId];
	int32_t itemCount;

	auto parameters = lua_gettop(L);
	if (parameters > 4) {
		// subtype already supplied, count then is the amount
		itemCount = std::max<int32_t>(1, count);
	} else if (it.hasSubType()) {
		if (it.stackable) {
			itemCount = static_cast<int32_t>(std::ceil(static_cast<float>(count) / ITEM_STACK_SIZE));
		} else {
			itemCount = 1;
		}
		subType = count;
	} else {
		itemCount = std::max<int32_t>(1, count);
	}

	while (itemCount > 0) {
		uint16_t stackCount = subType;
		if (it.stackable && stackCount > ITEM_STACK_SIZE) {
			stackCount = ITEM_STACK_SIZE;
		}

		const auto& newItem = Item::CreateItem(itemId, stackCount);
		if (!newItem) {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_ITEM_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		if (it.stackable) {
			subType -= stackCount;
		}

		ReturnValue ret = g_game.internalPlayerAddItem(player, newItem, canDropOnMap);
		if (ret != RETURNVALUE_NOERROR) {
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		if (--itemCount == 0) {
			if (newItem->hasParent()) {
				uint32_t uid = tfs::lua::getScriptEnv()->addThing(newItem);
				tfs::lua::pushNumber(L, uid);
				return 1;
			} else {
				// stackable item stacked with existing object, newItem will be released
				tfs::lua::pushBoolean(L, false);
				return 1;
			}
		}
	}

	tfs::lua::pushBoolean(L, false);
	return 1;
}

int LuaScriptInterface::luaDebugPrint(lua_State* L)
{
	// debugPrint(text)
	tfs::lua::reportError(L, tfs::lua::getString(L, -1));
	return 0;
}

int LuaScriptInterface::luaGetWorldUpTime(lua_State* L)
{
	// getWorldUpTime()
	uint64_t uptime = (OTSYS_TIME() - ProtocolStatus::start) / 1000;
	tfs::lua::pushNumber(L, uptime);
	return 1;
}

int LuaScriptInterface::luaGetSubTypeName(lua_State* L)
{
	// getSubTypeName(subType)
	int32_t subType = tfs::lua::getNumber<int32_t>(L, 1);
	if (subType > 0) {
		tfs::lua::pushString(L, Item::items[subType].name);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaScriptInterface::luaCreateCombatArea(lua_State* L)
{
	// createCombatArea({area}, <optional> {extArea})
	const auto env = tfs::lua::getScriptEnv();
	if (env->getScriptId() != EVENT_ID_LOADING) {
		tfs::lua::reportError(L, "This function can only be used while loading the script.");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	uint32_t areaId = g_luaEnvironment.createAreaObject(env->getScriptInterface());
	AreaCombat* area = g_luaEnvironment.getAreaObject(areaId);

	int parameters = lua_gettop(L);
	if (parameters >= 2) {
		uint32_t rowsExtArea;
		std::vector<uint32_t> vecExtArea;
		if (!lua_istable(L, 2) || !getArea(L, vecExtArea, rowsExtArea)) {
			tfs::lua::reportError(L, "Invalid extended area table.");
			tfs::lua::pushBoolean(L, false);
			return 1;
		}
		area->setupExtArea(vecExtArea, rowsExtArea);
	}

	uint32_t rowsArea = 0;
	std::vector<uint32_t> vecArea;
	if (!lua_istable(L, 1) || !getArea(L, vecArea, rowsArea)) {
		tfs::lua::reportError(L, "Invalid area table.");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	area->setupArea(vecArea, rowsArea);
	tfs::lua::pushNumber(L, areaId);
	return 1;
}

int LuaScriptInterface::luaDoAreaCombat(lua_State* L)
{
	// doAreaCombat(cid, type, pos, area, min, max, effect[, origin = ORIGIN_SPELL[, blockArmor = false[, blockShield =
	// false[, ignoreResistances = false]]]])
	const auto& creature = tfs::lua::getCreature(L, 1);
	if (!creature && (!tfs::lua::isNumber(L, 1) || tfs::lua::getNumber<uint32_t>(L, 1) != 0)) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	uint32_t areaId = tfs::lua::getNumber<uint32_t>(L, 4);
	const AreaCombat* area = g_luaEnvironment.getAreaObject(areaId);
	if (area || areaId == 0) {
		CombatType_t combatType = tfs::lua::getNumber<CombatType_t>(L, 2);

		CombatParams params;
		params.combatType = combatType;
		params.impactEffect = tfs::lua::getNumber<uint8_t>(L, 7);

		params.blockedByArmor = tfs::lua::getBoolean(L, 9, false);
		params.blockedByShield = tfs::lua::getBoolean(L, 10, false);
		params.ignoreResistances = tfs::lua::getBoolean(L, 11, false);

		CombatDamage damage;
		damage.origin = tfs::lua::getNumber<CombatOrigin>(L, 8, ORIGIN_SPELL);
		damage.primary.type = combatType;
		damage.primary.value = normal_random(tfs::lua::getNumber<int32_t>(L, 5), tfs::lua::getNumber<int32_t>(L, 6));

		Combat::doAreaCombat(creature, tfs::lua::getPosition(L, 3), area, damage, params);
		tfs::lua::pushBoolean(L, true);
	} else {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_AREA_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

int LuaScriptInterface::luaDoTargetCombat(lua_State* L)
{
	// doTargetCombat(cid, target, type, min, max, effect[, origin = ORIGIN_SPELL[, blockArmor = false[, blockShield =
	// false[, ignoreResistances = false]]]])
	const auto& creature = tfs::lua::getCreature(L, 1);
	if (!creature && (!tfs::lua::isNumber(L, 1) || tfs::lua::getNumber<uint32_t>(L, 1) != 0)) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const auto& target = tfs::lua::getCreature(L, 2);
	if (!target) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	CombatType_t combatType = tfs::lua::getNumber<CombatType_t>(L, 3);

	CombatParams params{
	    .combatType = combatType,
	    .impactEffect = tfs::lua::getNumber<uint8_t>(L, 6),
	    .blockedByArmor = tfs::lua::getBoolean(L, 8, false),
	    .blockedByShield = tfs::lua::getBoolean(L, 9, false),
	    .ignoreResistances = tfs::lua::getBoolean(L, 10, false),
	};

	CombatDamage damage{
	    .primary =
	        {
	            .type = combatType,
	            .value = normal_random(tfs::lua::getNumber<int32_t>(L, 4), tfs::lua::getNumber<int32_t>(L, 5)),
	        },
	    .origin = tfs::lua::getNumber<CombatOrigin>(L, 7, ORIGIN_SPELL),
	};

	Combat::doTargetCombat(creature, target, damage, params);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaDoChallengeCreature(lua_State* L)
{
	// doChallengeCreature(cid, target[, force = false])
	const auto& creature = tfs::lua::getCreature(L, 1);
	if (!creature) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const auto& target = tfs::lua::getCreature(L, 2);
	if (!target) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CREATURE_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	target->challengeCreature(creature, tfs::lua::getBoolean(L, 3, false));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaIsValidUID(lua_State* L)
{
	// isValidUID(uid)
	tfs::lua::pushBoolean(L, tfs::lua::getScriptEnv()->getThingByUID(tfs::lua::getNumber<uint32_t>(L, -1)) != nullptr);
	return 1;
}

int LuaScriptInterface::luaIsDepot(lua_State* L)
{
	// isDepot(uid)
	const auto& container = tfs::lua::getScriptEnv()->getContainerByUID(tfs::lua::getNumber<uint32_t>(L, -1));
	tfs::lua::pushBoolean(L, container && container->getDepotLocker());
	return 1;
}

int LuaScriptInterface::luaIsMoveable(lua_State* L)
{
	// isMoveable(uid)
	// isMovable(uid)
	const auto& thing = tfs::lua::getScriptEnv()->getThingByUID(tfs::lua::getNumber<uint32_t>(L, -1));
	if (const auto& item = thing->asItem()) {
		tfs::lua::pushBoolean(L, item->isPushable());
	} else if (const auto& creature = thing->asCreature()) {
		tfs::lua::pushBoolean(L, creature->isPushable());
	} else if (const auto& tile = thing->asTile()) {
		tfs::lua::pushBoolean(L, false);
	} else {
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

int LuaScriptInterface::luaGetDepotId(lua_State* L)
{
	// getDepotId(uid)
	uint32_t uid = tfs::lua::getNumber<uint32_t>(L, -1);

	const auto& container = tfs::lua::getScriptEnv()->getContainerByUID(uid);
	if (!container) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_CONTAINER_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const auto& depotLocker = container->getDepotLocker();
	if (!depotLocker) {
		tfs::lua::reportError(L, "Depot not found");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	tfs::lua::pushNumber(L, depotLocker->getDepotId());
	return 1;
}

int LuaScriptInterface::luaAddEvent(lua_State* L)
{
	// addEvent(callback, delay, ...)
	int parameters = lua_gettop(L);
	if (parameters < 2) {
		tfs::lua::reportError(L, std::format("Not enough parameters: {:d}.", parameters));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	if (!lua_isfunction(L, 1)) {
		tfs::lua::reportError(L, "callback parameter should be a function.");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	if (!tfs::lua::isNumber(L, 2)) {
		tfs::lua::reportError(L, "delay parameter should be a number.");
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	if (ConfigManager::getBoolean(ConfigManager::WARN_UNSAFE_SCRIPTS) ||
	    ConfigManager::getBoolean(ConfigManager::CONVERT_UNSAFE_SCRIPTS)) {
		std::vector<std::pair<int32_t, tfs::lua::LuaDataType>> indexes;
		for (int i = 3; i <= parameters; ++i) {
			if (lua_getmetatable(L, i) == 0) {
				continue;
			}
			lua_rawgeti(L, -1, 't');

			const auto type = tfs::lua::getNumber<tfs::lua::LuaDataType>(L, -1);
			if (type != tfs::lua::LuaData_Unknown && type != tfs::lua::LuaData_Tile) {
				indexes.push_back({i, type});
			}
			lua_pop(L, 2);
		}

		if (!indexes.empty()) {
			if (ConfigManager::getBoolean(ConfigManager::WARN_UNSAFE_SCRIPTS)) {
				bool plural = indexes.size() > 1;

				std::string warningString = "Argument";
				if (plural) {
					warningString += 's';
				}

				for (const auto& entry : indexes) {
					if (entry == indexes.front()) {
						warningString += ' ';
					} else if (entry == indexes.back()) {
						warningString += " and ";
					} else {
						warningString += ", ";
					}
					warningString += '#';
					warningString += std::to_string(entry.first);
				}

				if (plural) {
					warningString += " are unsafe";
				} else {
					warningString += " is unsafe";
				}

				tfs::lua::reportError(L, warningString);
			}

			if (ConfigManager::getBoolean(ConfigManager::CONVERT_UNSAFE_SCRIPTS)) {
				for (auto&& [index, type] : indexes | std::views::as_const) {
					switch (type) {
						case tfs::lua::LuaData_Item:
						case tfs::lua::LuaData_Container:
						case tfs::lua::LuaData_Teleport:
						case tfs::lua::LuaData_Podium: {
							lua_getglobal(L, "Item");
							lua_getfield(L, -1, "getUniqueId");
							break;
						}
						case tfs::lua::LuaData_Player:
						case tfs::lua::LuaData_Monster:
						case tfs::lua::LuaData_Npc: {
							lua_getglobal(L, "Creature");
							lua_getfield(L, -1, "getId");
							break;
						}
						default:
							break;
					}
					lua_replace(L, -2);
					lua_pushvalue(L, index);
					lua_call(L, 1, 1);
					lua_replace(L, index);
				}
			}
		}
	}

	LuaTimerEventDesc eventDesc;
	eventDesc.parameters.reserve(parameters -
	                             2); // safe to use -2 since we garanteed that there is at least two parameters
	for (int i = 0; i < parameters - 2; ++i) {
		eventDesc.parameters.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
	}

	uint32_t delay = std::max<uint32_t>(100, tfs::lua::getNumber<uint32_t>(L, 2));
	lua_pop(L, 1);

	eventDesc.function = luaL_ref(L, LUA_REGISTRYINDEX);
	eventDesc.scriptId = tfs::lua::getScriptEnv()->getScriptId();

	auto& lastTimerEventId = g_luaEnvironment.lastEventTimerId;
	eventDesc.eventId = g_scheduler.addEvent(
	    createSchedulerTask(delay, [=]() { g_luaEnvironment.executeTimerEvent(lastTimerEventId); }));

	g_luaEnvironment.timerEvents.emplace(lastTimerEventId, std::move(eventDesc));
	tfs::lua::pushNumber(L, lastTimerEventId++);
	return 1;
}

int LuaScriptInterface::luaStopEvent(lua_State* L)
{
	// stopEvent(eventid)
	uint32_t eventId = tfs::lua::getNumber<uint32_t>(L, 1);

	auto& timerEvents = g_luaEnvironment.timerEvents;
	auto it = timerEvents.find(eventId);
	if (it == timerEvents.end()) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	LuaTimerEventDesc timerEventDesc = std::move(it->second);
	timerEvents.erase(it);

	g_scheduler.stopEvent(timerEventDesc.eventId);
	luaL_unref(L, LUA_REGISTRYINDEX, timerEventDesc.function);

	for (auto parameter : timerEventDesc.parameters) {
		luaL_unref(L, LUA_REGISTRYINDEX, parameter);
	}

	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaSaveServer(lua_State* L)
{
	g_globalEvents->save();
	g_game.saveGameState();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaCleanMap(lua_State* L)
{
	tfs::lua::pushNumber(L, g_game.map.clean());
	return 1;
}

int LuaScriptInterface::luaIsInWar(lua_State* L)
{
	// isInWar(cid, target)
	const auto& player = tfs::lua::getPlayer(L, 1);
	if (!player) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_PLAYER_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const auto& targetPlayer = tfs::lua::getPlayer(L, 2);
	if (!targetPlayer) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_PLAYER_NOT_FOUND));
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	tfs::lua::pushBoolean(L, player->isInWar(targetPlayer));
	return 1;
}

int LuaScriptInterface::luaGetWaypointPositionByName(lua_State* L)
{
	// getWaypointPositionByName(name)
	auto& waypoints = g_game.map.waypoints;

	auto it = waypoints.find(tfs::lua::getString(L, -1));
	if (it != waypoints.end()) {
		tfs::lua::pushPosition(L, it->second);
	} else {
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

int LuaScriptInterface::luaSendChannelMessage(lua_State* L)
{
	// sendChannelMessage(channelId, type, message)
	uint32_t channelId = tfs::lua::getNumber<uint32_t>(L, 1);
	ChatChannel* channel = g_chat->getChannelById(channelId);
	if (!channel) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	SpeakClasses type = tfs::lua::getNumber<SpeakClasses>(L, 2);
	std::string message = tfs::lua::getString(L, 3);
	channel->sendToAll(message, type);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaSendGuildChannelMessage(lua_State* L)
{
	// sendGuildChannelMessage(guildId, type, message)
	uint32_t guildId = tfs::lua::getNumber<uint32_t>(L, 1);
	ChatChannel* channel = g_chat->getGuildChannelById(guildId);
	if (!channel) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	SpeakClasses type = tfs::lua::getNumber<SpeakClasses>(L, 2);
	std::string message = tfs::lua::getString(L, 3);
	channel->sendToAll(message, type);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int LuaScriptInterface::luaIsScriptsInterface(lua_State* L)
{
	// isScriptsInterface()
	if (tfs::lua::getScriptEnv()->getScriptInterface() == &g_scripts->getScriptInterface()) {
		tfs::lua::pushBoolean(L, true);
	} else {
		tfs::lua::reportError(L, "Event: can only be called inside (data/scripts/)");
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

const luaL_Reg LuaScriptInterface::luaBitReg[] = {
    //{"tobit", LuaScriptInterface::luaBitToBit},
    {"bnot", LuaScriptInterface::luaBitNot},
    {"band", LuaScriptInterface::luaBitAnd},
    {"bor", LuaScriptInterface::luaBitOr},
    {"bxor", LuaScriptInterface::luaBitXor},
    {"lshift", LuaScriptInterface::luaBitLeftShift},
    {"rshift", LuaScriptInterface::luaBitRightShift},
    //{"arshift", LuaScriptInterface::luaBitArithmeticalRightShift},
    //{"rol", LuaScriptInterface::luaBitRotateLeft},
    //{"ror", LuaScriptInterface::luaBitRotateRight},
    //{"bswap", LuaScriptInterface::luaBitSwapEndian},
    //{"tohex", LuaScriptInterface::luaBitToHex},
    {nullptr, nullptr}};

int LuaScriptInterface::luaBitNot(lua_State* L)
{
	tfs::lua::pushNumber(L, ~tfs::lua::getNumber<uint32_t>(L, -1));
	return 1;
}

#define MULTIOP(name, op) \
	int LuaScriptInterface::luaBit##name(lua_State* L) \
	{ \
		int n = lua_gettop(L); \
		uint32_t w = tfs::lua::getNumber<uint32_t>(L, -1); \
		for (int i = 1; i < n; ++i) w op tfs::lua::getNumber<uint32_t>(L, i); \
		tfs::lua::pushNumber(L, w); \
		return 1; \
	}

MULTIOP(And, &=)
MULTIOP(Or, |=)
MULTIOP(Xor, ^=)

#define SHIFTOP(name, op) \
	int LuaScriptInterface::luaBit##name(lua_State* L) \
	{ \
		uint32_t n1 = tfs::lua::getNumber<uint32_t>(L, 1), n2 = tfs::lua::getNumber<uint32_t>(L, 2); \
		tfs::lua::pushNumber(L, (n1 op n2)); \
		return 1; \
	}

SHIFTOP(LeftShift, <<)
SHIFTOP(RightShift, >>)

const luaL_Reg LuaScriptInterface::luaConfigManagerTable[] = {
    {"getString", LuaScriptInterface::luaConfigManagerGetString},
    {"getNumber", LuaScriptInterface::luaConfigManagerGetNumber},
    {"getBoolean", LuaScriptInterface::luaConfigManagerGetBoolean},
    {nullptr, nullptr}};

int LuaScriptInterface::luaConfigManagerGetString(lua_State* L)
{
	tfs::lua::pushString(L, ConfigManager::getString(tfs::lua::getNumber<ConfigManager::string_config_t>(L, -1)));
	return 1;
}

int LuaScriptInterface::luaConfigManagerGetNumber(lua_State* L)
{
	tfs::lua::pushNumber(L, ConfigManager::getNumber(tfs::lua::getNumber<ConfigManager::integer_config_t>(L, -1)));
	return 1;
}

int LuaScriptInterface::luaConfigManagerGetBoolean(lua_State* L)
{
	tfs::lua::pushBoolean(L, ConfigManager::getBoolean(tfs::lua::getNumber<ConfigManager::boolean_config_t>(L, -1)));
	return 1;
}

const luaL_Reg LuaScriptInterface::luaDatabaseTable[] = {
    {"query", LuaScriptInterface::luaDatabaseExecute},
    {"asyncQuery", LuaScriptInterface::luaDatabaseAsyncExecute},
    {"storeQuery", LuaScriptInterface::luaDatabaseStoreQuery},
    {"asyncStoreQuery", LuaScriptInterface::luaDatabaseAsyncStoreQuery},
    {"escapeString", LuaScriptInterface::luaDatabaseEscapeString},
    {"escapeBlob", LuaScriptInterface::luaDatabaseEscapeBlob},
    {"lastInsertId", LuaScriptInterface::luaDatabaseLastInsertId},
    {"tableExists", LuaScriptInterface::luaDatabaseTableExists},
    {nullptr, nullptr}};

int LuaScriptInterface::luaDatabaseExecute(lua_State* L)
{
	// db.query(query)
	tfs::lua::pushBoolean(L, Database::getInstance().executeQuery(tfs::lua::getString(L, -1)));
	return 1;
}

int LuaScriptInterface::luaDatabaseAsyncExecute(lua_State* L)
{
	// db.asyncQuery(query, callback)
	std::function<void(const std::shared_ptr<DBResult>&, bool)> callback;
	if (lua_gettop(L) > 1) {
		int32_t ref = luaL_ref(L, LUA_REGISTRYINDEX);
		auto scriptId = tfs::lua::getScriptEnv()->getScriptId();
		callback = [ref, scriptId](const std::shared_ptr<DBResult>&, bool success) {
			lua_State* L = g_luaEnvironment.getLuaState();
			if (!L) {
				return;
			}

			if (!tfs::lua::reserveScriptEnv()) {
				luaL_unref(L, LUA_REGISTRYINDEX, ref);
				return;
			}

			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			tfs::lua::pushBoolean(L, success);
			auto env = tfs::lua::getScriptEnv();
			env->setScriptId(scriptId, &g_luaEnvironment);
			g_luaEnvironment.callFunction(1);

			luaL_unref(L, LUA_REGISTRYINDEX, ref);
		};
	}
	g_databaseTasks.addTask(tfs::lua::getString(L, -1), callback);
	return 0;
}

int LuaScriptInterface::luaDatabaseStoreQuery(lua_State* L)
{
	// db.storeQuery(query)
	if (const auto& result = Database::getInstance().storeQuery(tfs::lua::getString(L, -1))) {
		tfs::lua::pushNumber(L, tfs::lua::addResult(result));
	} else {
		tfs::lua::pushBoolean(L, false);
	}
	return 1;
}

int LuaScriptInterface::luaDatabaseAsyncStoreQuery(lua_State* L)
{
	// db.asyncStoreQuery(query, callback)
	std::function<void(const std::shared_ptr<DBResult>&, bool)> callback;
	if (lua_gettop(L) > 1) {
		int32_t ref = luaL_ref(L, LUA_REGISTRYINDEX);
		auto scriptId = tfs::lua::getScriptEnv()->getScriptId();
		callback = [ref, scriptId](const std::shared_ptr<DBResult>& result, bool) {
			lua_State* L = g_luaEnvironment.getLuaState();
			if (!L) {
				return;
			}

			if (!tfs::lua::reserveScriptEnv()) {
				luaL_unref(L, LUA_REGISTRYINDEX, ref);
				return;
			}

			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			if (result) {
				tfs::lua::pushNumber(L, tfs::lua::addResult(result));
			} else {
				tfs::lua::pushBoolean(L, false);
			}
			auto env = tfs::lua::getScriptEnv();
			env->setScriptId(scriptId, &g_luaEnvironment);
			g_luaEnvironment.callFunction(1);

			luaL_unref(L, LUA_REGISTRYINDEX, ref);
		};
	}
	g_databaseTasks.addTask(tfs::lua::getString(L, -1), callback, true);
	return 0;
}

int LuaScriptInterface::luaDatabaseEscapeString(lua_State* L)
{
	// db.escapeString(s)
	tfs::lua::pushString(L, Database::getInstance().escapeString(tfs::lua::getString(L, -1)));
	return 1;
}

int LuaScriptInterface::luaDatabaseEscapeBlob(lua_State* L)
{
	// db.escapeBlob(s, length)
	uint32_t length = tfs::lua::getNumber<uint32_t>(L, 2);
	tfs::lua::pushString(L, Database::getInstance().escapeBlob(tfs::lua::getString(L, 1).data(), length));
	return 1;
}

int LuaScriptInterface::luaDatabaseLastInsertId(lua_State* L)
{
	// db.lastInsertId()
	tfs::lua::pushNumber(L, Database::getInstance().getLastInsertId());
	return 1;
}

int LuaScriptInterface::luaDatabaseTableExists(lua_State* L)
{
	// db.tableExists(tableName)
	tfs::lua::pushBoolean(L, DatabaseManager::tableExists(tfs::lua::getString(L, -1)));
	return 1;
}

const luaL_Reg LuaScriptInterface::luaResultTable[] = {
    {"getNumber", LuaScriptInterface::luaResultGetNumber}, {"getString", LuaScriptInterface::luaResultGetString},
    {"getStream", LuaScriptInterface::luaResultGetStream}, {"next", LuaScriptInterface::luaResultNext},
    {"free", LuaScriptInterface::luaResultFree},           {nullptr, nullptr}};

int LuaScriptInterface::luaResultGetNumber(lua_State* L)
{
	const auto& result = tfs::lua::getResultByID(tfs::lua::getNumber<uint32_t>(L, 1));
	if (!result) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const std::string& s = tfs::lua::getString(L, 2);
	tfs::lua::pushNumber(L, result->getNumber<int64_t>(s));
	return 1;
}

int LuaScriptInterface::luaResultGetString(lua_State* L)
{
	const auto& result = tfs::lua::getResultByID(tfs::lua::getNumber<uint32_t>(L, 1));
	if (!result) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	const std::string& s = tfs::lua::getString(L, 2);
	tfs::lua::pushString(L, result->getString(s));
	return 1;
}

int LuaScriptInterface::luaResultGetStream(lua_State* L)
{
	const auto& result = tfs::lua::getResultByID(tfs::lua::getNumber<uint32_t>(L, 1));
	if (!result) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	auto stream = result->getString(tfs::lua::getString(L, 2));
	lua_pushlstring(L, stream.data(), stream.size());
	tfs::lua::pushNumber(L, stream.size());
	return 2;
}

int LuaScriptInterface::luaResultNext(lua_State* L)
{
	const auto& result = tfs::lua::getResultByID(tfs::lua::getNumber<uint32_t>(L, -1));
	if (!result) {
		tfs::lua::pushBoolean(L, false);
		return 1;
	}

	tfs::lua::pushBoolean(L, result->next());
	return 1;
}

int LuaScriptInterface::luaResultFree(lua_State* L)
{
	tfs::lua::pushBoolean(L, tfs::lua::removeResult(tfs::lua::getNumber<uint32_t>(L, -1)));
	return 1;
}

// os
int LuaScriptInterface::luaSystemTime(lua_State* L)
{
	// os.mtime()
	tfs::lua::pushNumber(L, OTSYS_TIME());
	return 1;
}

// table
int LuaScriptInterface::luaTableCreate(lua_State* L)
{
	// table.create(arrayLength, keyLength)
	lua_createtable(L, tfs::lua::getNumber<int32_t>(L, 1), tfs::lua::getNumber<int32_t>(L, 2));
	return 1;
}

int LuaScriptInterface::luaTablePack(lua_State* L)
{
	// table.pack(...)
	int n = lua_gettop(L);         /* number of elements to pack */
	lua_createtable(L, n, 1);      /* create result table */
	lua_insert(L, 1);              /* put it at index 1 */
	for (int i = n; i >= 1; i--) { /* assign elements */
		lua_rawseti(L, 1, i);
	}
	if (luaL_callmeta(L, -1, "__index") != 0) {
		lua_replace(L, -2);
	}
	tfs::lua::pushNumber(L, n);
	lua_setfield(L, 1, "n"); /* t.n = number of elements */
	return 1;                /* return table */
}

#define registerEnum(L, value) \
	{ \
		std::string enumName = #value; \
		registerGlobalVariable(L, enumName.substr(enumName.find_last_of(':') + 1), value); \
	}

void LuaScriptInterface::registerFunctions()
{
	using namespace tfs::lua;

	// doPlayerAddItem(uid, itemid, <optional: default: 1> count/subtype)
	// doPlayerAddItem(cid, itemid, <optional: default: 1> count, <optional: default: 1> canDropOnMap, <optional:
	// default: 1>subtype) Returns uid of the created item
	lua_register(L, "doPlayerAddItem", LuaScriptInterface::luaDoPlayerAddItem);

	// isValidUID(uid)
	lua_register(L, "isValidUID", LuaScriptInterface::luaIsValidUID);

	// isDepot(uid)
	lua_register(L, "isDepot", LuaScriptInterface::luaIsDepot);

	// isMovable(uid)
	lua_register(L, "isMovable", LuaScriptInterface::luaIsMoveable);

	// doAddContainerItem(uid, itemid, <optional> count/subtype)
	// lua_register(L, "doAddContainerItem", LuaScriptInterface::luaDoAddContainerItem);

	// getDepotId(uid)
	lua_register(L, "getDepotId", LuaScriptInterface::luaGetDepotId);

	// getWorldUpTime()
	lua_register(L, "getWorldUpTime", LuaScriptInterface::luaGetWorldUpTime);

	// getSubTypeName(subType)
	lua_register(L, "getSubTypeName", LuaScriptInterface::luaGetSubTypeName);

	// createCombatArea({area}, <optional> {extArea})
	lua_register(L, "createCombatArea", LuaScriptInterface::luaCreateCombatArea);

	// doAreaCombat(cid, type, pos, area, min, max, effect[, origin = ORIGIN_SPELL[, blockArmor = false[, blockShield =
	// false[, ignoreResistances = false]]]])
	lua_register(L, "doAreaCombat", LuaScriptInterface::luaDoAreaCombat);

	// doTargetCombat(cid, target, type, min, max, effect[, origin = ORIGIN_SPELL[, blockArmor = false[, blockShield =
	// false[, ignoreResistances = false]]]])
	lua_register(L, "doTargetCombat", LuaScriptInterface::luaDoTargetCombat);

	// doChallengeCreature(cid, target[, force = false])
	lua_register(L, "doChallengeCreature", LuaScriptInterface::luaDoChallengeCreature);

	// addEvent(callback, delay, ...)
	lua_register(L, "addEvent", LuaScriptInterface::luaAddEvent);

	// stopEvent(eventid)
	lua_register(L, "stopEvent", LuaScriptInterface::luaStopEvent);

	// saveServer()
	lua_register(L, "saveServer", LuaScriptInterface::luaSaveServer);

	// cleanMap()
	lua_register(L, "cleanMap", LuaScriptInterface::luaCleanMap);

	// debugPrint(text)
	lua_register(L, "debugPrint", LuaScriptInterface::luaDebugPrint);

	// isInWar(cid, target)
	lua_register(L, "isInWar", LuaScriptInterface::luaIsInWar);

	// getWaypointPosition(name)
	lua_register(L, "getWaypointPositionByName", LuaScriptInterface::luaGetWaypointPositionByName);

	// sendChannelMessage(channelId, type, message)
	lua_register(L, "sendChannelMessage", LuaScriptInterface::luaSendChannelMessage);

	// sendGuildChannelMessage(guildId, type, message)
	lua_register(L, "sendGuildChannelMessage", LuaScriptInterface::luaSendGuildChannelMessage);

	// isScriptsInterface()
	lua_register(L, "isScriptsInterface", LuaScriptInterface::luaIsScriptsInterface);

	// bit operations for Lua, based on bitlib project release 24
	// bit.bnot, bit.band, bit.bor, bit.bxor, bit.lshift, bit.rshift
	luaL_register(L, "bit", LuaScriptInterface::luaBitReg);
	lua_pop(L, 1);

	// configManager table
	luaL_register(L, "configManager", LuaScriptInterface::luaConfigManagerTable);
	lua_pop(L, 1);

	// db table
	luaL_register(L, "db", LuaScriptInterface::luaDatabaseTable);
	lua_pop(L, 1);

	// result table
	luaL_register(L, "result", LuaScriptInterface::luaResultTable);
	lua_pop(L, 1);

	/* New functions */
	// registerClass(className, baseClass, newFunction)
	// registerTable(tableName)
	// registerMethod(className, functionName, function)
	// registerMetaMethod(className, functionName, function)
	// registerGlobalMethod(functionName, function)
	// registerVariable(tableName, name, value)
	// registerGlobalVariable(name, value)
	// registerEnum(value)
	// registerEnumIn(tableName, value)

	// os
	registerMethod(L, "os", "mtime", LuaScriptInterface::luaSystemTime);

	// table
	registerMethod(L, "table", "create", LuaScriptInterface::luaTableCreate);
	registerMethod(L, "table", "pack", LuaScriptInterface::luaTablePack);
}

LuaEnvironment::LuaEnvironment() : LuaScriptInterface("Main Interface") {}

LuaEnvironment::~LuaEnvironment()
{
	delete testInterface;
	closeState();
}

bool LuaEnvironment::initState()
{
	L = luaL_newstate();
	if (!L) {
		return false;
	}

	luaL_openlibs(L);
	registerFunctions();

	runningEventId = EVENT_ID_USER;
	return true;
}

bool LuaEnvironment::reInitState()
{
	// TODO: get children, reload children
	closeState();
	return initState();
}

bool LuaEnvironment::closeState()
{
	if (!L) {
		return false;
	}

	for (auto&& interface : combatIdMap | std::views::keys | std::views::as_const) {
		clearCombatObjects(interface);
	}

	for (auto&& interface : areaIdMap | std::views::keys | std::views::as_const) {
		clearAreaObjects(interface);
	}

	for (auto&& timerEvent : timerEvents | std::views::values) {
		LuaTimerEventDesc timerEventDesc = std::move(timerEvent);
		for (int32_t parameter : timerEventDesc.parameters) {
			luaL_unref(L, LUA_REGISTRYINDEX, parameter);
		}
		luaL_unref(L, LUA_REGISTRYINDEX, timerEventDesc.function);
	}

	combatIdMap.clear();
	areaIdMap.clear();
	timerEvents.clear();
	cacheFiles.clear();

	lua_close(L);
	L = nullptr;
	return true;
}

LuaScriptInterface* LuaEnvironment::getTestInterface()
{
	if (!testInterface) {
		testInterface = new LuaScriptInterface("Test Interface");
		testInterface->initState();
	}
	return testInterface;
}

Combat_ptr LuaEnvironment::getCombatObject(uint32_t id) const
{
	auto it = combatMap.find(id);
	if (it == combatMap.end()) {
		return nullptr;
	}
	return it->second;
}

Combat_ptr LuaEnvironment::createCombatObject(LuaScriptInterface* interface)
{
	Combat_ptr combat = std::make_shared<Combat>();
	combatMap[++lastCombatId] = combat;
	combatIdMap[interface].push_back(lastCombatId);
	return combat;
}

void LuaEnvironment::clearCombatObjects(LuaScriptInterface* interface)
{
	auto it = combatIdMap.find(interface);
	if (it == combatIdMap.end()) {
		return;
	}

	for (uint32_t id : it->second) {
		auto itt = combatMap.find(id);
		if (itt != combatMap.end()) {
			combatMap.erase(itt);
		}
	}
	it->second.clear();
}

AreaCombat* LuaEnvironment::getAreaObject(uint32_t id) const
{
	auto it = areaMap.find(id);
	if (it == areaMap.end()) {
		return nullptr;
	}
	return it->second;
}

uint32_t LuaEnvironment::createAreaObject(LuaScriptInterface* interface)
{
	areaMap[++lastAreaId] = new AreaCombat;
	areaIdMap[interface].push_back(lastAreaId);
	return lastAreaId;
}

void LuaEnvironment::clearAreaObjects(LuaScriptInterface* interface)
{
	auto it = areaIdMap.find(interface);
	if (it == areaIdMap.end()) {
		return;
	}

	for (uint32_t id : it->second) {
		auto itt = areaMap.find(id);
		if (itt != areaMap.end()) {
			delete itt->second;
			areaMap.erase(itt);
		}
	}
	it->second.clear();
}

void LuaEnvironment::executeTimerEvent(uint32_t eventIndex)
{
	auto it = timerEvents.find(eventIndex);
	if (it == timerEvents.end()) {
		return;
	}

	LuaTimerEventDesc timerEventDesc = std::move(it->second);
	timerEvents.erase(it);

	// push function
	lua_rawgeti(L, LUA_REGISTRYINDEX, timerEventDesc.function);

	// push parameters
	for (auto parameter : std::views::reverse(timerEventDesc.parameters)) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, parameter);
	}

	// call the function
	if (tfs::lua::reserveScriptEnv()) {
		const auto env = tfs::lua::getScriptEnv();
		env->setTimerEvent();
		env->setScriptId(timerEventDesc.scriptId, this);
		callFunction(timerEventDesc.parameters.size());
	} else {
		std::cout << "[Error - LuaScriptInterface::executeTimerEvent] Call stack overflow\n";
	}

	// free resources
	luaL_unref(L, LUA_REGISTRYINDEX, timerEventDesc.function);
	for (auto parameter : timerEventDesc.parameters) {
		luaL_unref(L, LUA_REGISTRYINDEX, parameter);
	}
}
