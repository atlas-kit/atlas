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

// result map

void registerClass(lua_State* L, std::string_view className, std::string_view baseClass,
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

void registerTable(lua_State* L, std::string_view tableName)
{
	// _G[tableName] = {}
	lua_newtable(L);
	lua_setglobal(L, tableName.data());
}

void registerMetaMethod(lua_State* L, std::string_view className, std::string_view methodName, lua_CFunction func)
{
	// className.metatable.methodName = func
	luaL_getmetatable(L, className.data());
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, methodName.data());

	// pop className.metatable
	lua_pop(L, 1);
}

void registerGlobalMethod(lua_State* L, std::string_view functionName, lua_CFunction func)
{
	// _G[functionName] = func
	lua_pushcfunction(L, func);
	lua_setglobal(L, functionName.data());
}

void registerVariable(lua_State* L, std::string_view tableName, std::string_view name, lua_Number value)
{
	// tableName.name = value
	lua_getglobal(L, tableName.data());
	tfs::lua::setField(L, name.data(), value);

	// pop tableName
	lua_pop(L, 1);
}

void registerGlobalVariable(lua_State* L, std::string_view name, lua_Number value)
{
	// _G[name] = value
	tfs::lua::pushNumber(L, value);
	lua_setglobal(L, name.data());
}

void registerGlobalBoolean(lua_State* L, std::string_view name, bool value)
{
	// _G[name] = value
	tfs::lua::pushBoolean(L, value);
	lua_setglobal(L, name.data());
}

std::string getStackTrace(lua_State* L, std::string_view error_desc)
{
	luaL_traceback(L, L, error_desc.data(), 1);
	return tfs::lua::popString(L);
}

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
	// registerClass(L, className, baseClass, newFunction)
	// registerTable(L, tableName)
	// registerMethod(L, className, functionName, function)
	// registerMetaMethod(L, className, functionName, function)
	// registerGlobalMethod(L, functionName, function)
	// registerVariable(L, tableName, name, value)
	// registerGlobalVariable(L, name, value)
	// registerEnum(L, value)
	// registerEnumIn(L, tableName, value)

	// Enums
	registerEnum(L, ACCOUNT_TYPE_NORMAL);
	registerEnum(L, ACCOUNT_TYPE_TUTOR);
	registerEnum(L, ACCOUNT_TYPE_SENIORTUTOR);
	registerEnum(L, ACCOUNT_TYPE_GAMEMASTER);
	registerEnum(L, ACCOUNT_TYPE_COMMUNITYMANAGER);
	registerEnum(L, ACCOUNT_TYPE_GOD);

	registerEnum(L, AMMO_NONE);
	registerEnum(L, AMMO_BOLT);
	registerEnum(L, AMMO_ARROW);
	registerEnum(L, AMMO_SPEAR);
	registerEnum(L, AMMO_THROWINGSTAR);
	registerEnum(L, AMMO_THROWINGKNIFE);
	registerEnum(L, AMMO_STONE);
	registerEnum(L, AMMO_SNOWBALL);

	registerEnum(L, CALLBACK_PARAM_LEVELMAGICVALUE);
	registerEnum(L, CALLBACK_PARAM_SKILLVALUE);
	registerEnum(L, CALLBACK_PARAM_TARGETTILE);
	registerEnum(L, CALLBACK_PARAM_TARGETCREATURE);

	registerEnum(L, COMBAT_FORMULA_UNDEFINED);
	registerEnum(L, COMBAT_FORMULA_LEVELMAGIC);
	registerEnum(L, COMBAT_FORMULA_SKILL);
	registerEnum(L, COMBAT_FORMULA_DAMAGE);

	registerEnum(L, COMBAT_NONE);
	registerEnum(L, COMBAT_PHYSICALDAMAGE);
	registerEnum(L, COMBAT_ENERGYDAMAGE);
	registerEnum(L, COMBAT_EARTHDAMAGE);
	registerEnum(L, COMBAT_FIREDAMAGE);
	registerEnum(L, COMBAT_UNDEFINEDDAMAGE);
	registerEnum(L, COMBAT_LIFEDRAIN);
	registerEnum(L, COMBAT_MANADRAIN);
	registerEnum(L, COMBAT_HEALING);
	registerEnum(L, COMBAT_DROWNDAMAGE);
	registerEnum(L, COMBAT_ICEDAMAGE);
	registerEnum(L, COMBAT_HOLYDAMAGE);
	registerEnum(L, COMBAT_DEATHDAMAGE);

	registerEnum(L, COMBAT_PARAM_TYPE);
	registerEnum(L, COMBAT_PARAM_EFFECT);
	registerEnum(L, COMBAT_PARAM_DISTANCEEFFECT);
	registerEnum(L, COMBAT_PARAM_BLOCKSHIELD);
	registerEnum(L, COMBAT_PARAM_BLOCKARMOR);
	registerEnum(L, COMBAT_PARAM_TARGETCASTERORTOPMOST);
	registerEnum(L, COMBAT_PARAM_CREATEITEM);
	registerEnum(L, COMBAT_PARAM_AGGRESSIVE);
	registerEnum(L, COMBAT_PARAM_DISPEL);
	registerEnum(L, COMBAT_PARAM_USECHARGES);

	registerEnum(L, CONDITION_NONE);
	registerEnum(L, CONDITION_POISON);
	registerEnum(L, CONDITION_FIRE);
	registerEnum(L, CONDITION_ENERGY);
	registerEnum(L, CONDITION_BLEEDING);
	registerEnum(L, CONDITION_HASTE);
	registerEnum(L, CONDITION_PARALYZE);
	registerEnum(L, CONDITION_OUTFIT);
	registerEnum(L, CONDITION_INVISIBLE);
	registerEnum(L, CONDITION_LIGHT);
	registerEnum(L, CONDITION_MANASHIELD);
	registerEnum(L, CONDITION_MANASHIELD_BREAKABLE);
	registerEnum(L, CONDITION_INFIGHT);
	registerEnum(L, CONDITION_DRUNK);
	registerEnum(L, CONDITION_EXHAUST_WEAPON);
	registerEnum(L, CONDITION_REGENERATION);
	registerEnum(L, CONDITION_SOUL);
	registerEnum(L, CONDITION_DROWN);
	registerEnum(L, CONDITION_MUTED);
	registerEnum(L, CONDITION_CHANNELMUTEDTICKS);
	registerEnum(L, CONDITION_YELLTICKS);
	registerEnum(L, CONDITION_ATTRIBUTES);
	registerEnum(L, CONDITION_FREEZING);
	registerEnum(L, CONDITION_DAZZLED);
	registerEnum(L, CONDITION_CURSED);
	registerEnum(L, CONDITION_EXHAUST_COMBAT);
	registerEnum(L, CONDITION_EXHAUST_HEAL);
	registerEnum(L, CONDITION_PACIFIED);
	registerEnum(L, CONDITION_SPELLCOOLDOWN);
	registerEnum(L, CONDITION_SPELLGROUPCOOLDOWN);
	registerEnum(L, CONDITION_ROOT);

	registerEnum(L, CONDITIONID_DEFAULT);
	registerEnum(L, CONDITIONID_COMBAT);
	registerEnum(L, CONDITIONID_HEAD);
	registerEnum(L, CONDITIONID_NECKLACE);
	registerEnum(L, CONDITIONID_BACKPACK);
	registerEnum(L, CONDITIONID_ARMOR);
	registerEnum(L, CONDITIONID_RIGHT);
	registerEnum(L, CONDITIONID_LEFT);
	registerEnum(L, CONDITIONID_LEGS);
	registerEnum(L, CONDITIONID_FEET);
	registerEnum(L, CONDITIONID_RING);
	registerEnum(L, CONDITIONID_AMMO);

	registerEnum(L, CONDITION_PARAM_OWNER);
	registerEnum(L, CONDITION_PARAM_TICKS);
	registerEnum(L, CONDITION_PARAM_DRUNKENNESS);
	registerEnum(L, CONDITION_PARAM_HEALTHGAIN);
	registerEnum(L, CONDITION_PARAM_HEALTHTICKS);
	registerEnum(L, CONDITION_PARAM_MANAGAIN);
	registerEnum(L, CONDITION_PARAM_MANATICKS);
	registerEnum(L, CONDITION_PARAM_DELAYED);
	registerEnum(L, CONDITION_PARAM_SPEED);
	registerEnum(L, CONDITION_PARAM_LIGHT_LEVEL);
	registerEnum(L, CONDITION_PARAM_LIGHT_COLOR);
	registerEnum(L, CONDITION_PARAM_SOULGAIN);
	registerEnum(L, CONDITION_PARAM_SOULTICKS);
	registerEnum(L, CONDITION_PARAM_MINVALUE);
	registerEnum(L, CONDITION_PARAM_MAXVALUE);
	registerEnum(L, CONDITION_PARAM_STARTVALUE);
	registerEnum(L, CONDITION_PARAM_TICKINTERVAL);
	registerEnum(L, CONDITION_PARAM_FORCEUPDATE);
	registerEnum(L, CONDITION_PARAM_SKILL_MELEE);
	registerEnum(L, CONDITION_PARAM_SKILL_FIST);
	registerEnum(L, CONDITION_PARAM_SKILL_CLUB);
	registerEnum(L, CONDITION_PARAM_SKILL_SWORD);
	registerEnum(L, CONDITION_PARAM_SKILL_AXE);
	registerEnum(L, CONDITION_PARAM_SKILL_DISTANCE);
	registerEnum(L, CONDITION_PARAM_SKILL_SHIELD);
	registerEnum(L, CONDITION_PARAM_SKILL_FISHING);
	registerEnum(L, CONDITION_PARAM_STAT_MAXHITPOINTS);
	registerEnum(L, CONDITION_PARAM_STAT_MAXMANAPOINTS);
	registerEnum(L, CONDITION_PARAM_STAT_MAGICPOINTS);
	registerEnum(L, CONDITION_PARAM_STAT_MAXHITPOINTSPERCENT);
	registerEnum(L, CONDITION_PARAM_STAT_MAXMANAPOINTSPERCENT);
	registerEnum(L, CONDITION_PARAM_STAT_MAGICPOINTSPERCENT);
	registerEnum(L, CONDITION_PARAM_PERIODICDAMAGE);
	registerEnum(L, CONDITION_PARAM_SKILL_MELEEPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_FISTPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_CLUBPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_SWORDPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_AXEPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_DISTANCEPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_SHIELDPERCENT);
	registerEnum(L, CONDITION_PARAM_SKILL_FISHINGPERCENT);
	registerEnum(L, CONDITION_PARAM_BUFF_SPELL);
	registerEnum(L, CONDITION_PARAM_SUBID);
	registerEnum(L, CONDITION_PARAM_FIELD);
	registerEnum(L, CONDITION_PARAM_DISABLE_DEFENSE);
	registerEnum(L, CONDITION_PARAM_MANASHIELD_BREAKABLE);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_CRITICALHITCHANCE);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_CRITICALHITAMOUNT);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_LIFELEECHCHANCE);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_LIFELEECHAMOUNT);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_MANALEECHCHANCE);
	registerEnum(L, CONDITION_PARAM_SPECIALSKILL_MANALEECHAMOUNT);
	registerEnum(L, CONDITION_PARAM_AGGRESSIVE);

	registerEnum(L, CONST_ME_NONE);
	registerEnum(L, CONST_ME_DRAWBLOOD);
	registerEnum(L, CONST_ME_LOSEENERGY);
	registerEnum(L, CONST_ME_POFF);
	registerEnum(L, CONST_ME_BLOCKHIT);
	registerEnum(L, CONST_ME_EXPLOSIONAREA);
	registerEnum(L, CONST_ME_EXPLOSIONHIT);
	registerEnum(L, CONST_ME_FIREAREA);
	registerEnum(L, CONST_ME_YELLOW_RINGS);
	registerEnum(L, CONST_ME_GREEN_RINGS);
	registerEnum(L, CONST_ME_HITAREA);
	registerEnum(L, CONST_ME_TELEPORT);
	registerEnum(L, CONST_ME_ENERGYHIT);
	registerEnum(L, CONST_ME_MAGIC_BLUE);
	registerEnum(L, CONST_ME_MAGIC_RED);
	registerEnum(L, CONST_ME_MAGIC_GREEN);
	registerEnum(L, CONST_ME_HITBYFIRE);
	registerEnum(L, CONST_ME_HITBYPOISON);
	registerEnum(L, CONST_ME_MORTAREA);
	registerEnum(L, CONST_ME_SOUND_GREEN);
	registerEnum(L, CONST_ME_SOUND_RED);
	registerEnum(L, CONST_ME_POISONAREA);
	registerEnum(L, CONST_ME_SOUND_YELLOW);
	registerEnum(L, CONST_ME_SOUND_PURPLE);
	registerEnum(L, CONST_ME_SOUND_BLUE);
	registerEnum(L, CONST_ME_SOUND_WHITE);
	registerEnum(L, CONST_ME_BUBBLES);
	registerEnum(L, CONST_ME_CRAPS);
	registerEnum(L, CONST_ME_GIFT_WRAPS);
	registerEnum(L, CONST_ME_FIREWORK_YELLOW);
	registerEnum(L, CONST_ME_FIREWORK_RED);
	registerEnum(L, CONST_ME_FIREWORK_BLUE);
	registerEnum(L, CONST_ME_STUN);
	registerEnum(L, CONST_ME_SLEEP);
	registerEnum(L, CONST_ME_WATERCREATURE);
	registerEnum(L, CONST_ME_GROUNDSHAKER);
	registerEnum(L, CONST_ME_HEARTS);
	registerEnum(L, CONST_ME_FIREATTACK);
	registerEnum(L, CONST_ME_ENERGYAREA);
	registerEnum(L, CONST_ME_SMALLCLOUDS);
	registerEnum(L, CONST_ME_HOLYDAMAGE);
	registerEnum(L, CONST_ME_BIGCLOUDS);
	registerEnum(L, CONST_ME_ICEAREA);
	registerEnum(L, CONST_ME_ICETORNADO);
	registerEnum(L, CONST_ME_ICEATTACK);
	registerEnum(L, CONST_ME_STONES);
	registerEnum(L, CONST_ME_SMALLPLANTS);
	registerEnum(L, CONST_ME_CARNIPHILA);
	registerEnum(L, CONST_ME_PURPLEENERGY);
	registerEnum(L, CONST_ME_YELLOWENERGY);
	registerEnum(L, CONST_ME_HOLYAREA);
	registerEnum(L, CONST_ME_BIGPLANTS);
	registerEnum(L, CONST_ME_CAKE);
	registerEnum(L, CONST_ME_GIANTICE);
	registerEnum(L, CONST_ME_WATERSPLASH);
	registerEnum(L, CONST_ME_PLANTATTACK);
	registerEnum(L, CONST_ME_TUTORIALARROW);
	registerEnum(L, CONST_ME_TUTORIALSQUARE);
	registerEnum(L, CONST_ME_MIRRORHORIZONTAL);
	registerEnum(L, CONST_ME_MIRRORVERTICAL);
	registerEnum(L, CONST_ME_SKULLHORIZONTAL);
	registerEnum(L, CONST_ME_SKULLVERTICAL);
	registerEnum(L, CONST_ME_ASSASSIN);
	registerEnum(L, CONST_ME_STEPSHORIZONTAL);
	registerEnum(L, CONST_ME_BLOODYSTEPS);
	registerEnum(L, CONST_ME_STEPSVERTICAL);
	registerEnum(L, CONST_ME_YALAHARIGHOST);
	registerEnum(L, CONST_ME_BATS);
	registerEnum(L, CONST_ME_SMOKE);
	registerEnum(L, CONST_ME_INSECTS);
	registerEnum(L, CONST_ME_DRAGONHEAD);
	registerEnum(L, CONST_ME_ORCSHAMAN);
	registerEnum(L, CONST_ME_ORCSHAMAN_FIRE);
	registerEnum(L, CONST_ME_THUNDER);
	registerEnum(L, CONST_ME_FERUMBRAS);
	registerEnum(L, CONST_ME_CONFETTI_HORIZONTAL);
	registerEnum(L, CONST_ME_CONFETTI_VERTICAL);
	registerEnum(L, CONST_ME_BLACKSMOKE);
	registerEnum(L, CONST_ME_REDSMOKE);
	registerEnum(L, CONST_ME_YELLOWSMOKE);
	registerEnum(L, CONST_ME_GREENSMOKE);
	registerEnum(L, CONST_ME_PURPLESMOKE);
	registerEnum(L, CONST_ME_EARLY_THUNDER);
	registerEnum(L, CONST_ME_RAGIAZ_BONECAPSULE);
	registerEnum(L, CONST_ME_CRITICAL_DAMAGE);
	registerEnum(L, CONST_ME_PLUNGING_FISH);
	registerEnum(L, CONST_ME_BLUECHAIN);
	registerEnum(L, CONST_ME_ORANGECHAIN);
	registerEnum(L, CONST_ME_GREENCHAIN);
	registerEnum(L, CONST_ME_PURPLECHAIN);
	registerEnum(L, CONST_ME_GREYCHAIN);
	registerEnum(L, CONST_ME_YELLOWCHAIN);
	registerEnum(L, CONST_ME_YELLOWSPARKLES);
	registerEnum(L, CONST_ME_FAEEXPLOSION);
	registerEnum(L, CONST_ME_FAECOMING);
	registerEnum(L, CONST_ME_FAEGOING);
	registerEnum(L, CONST_ME_BIGCLOUDSSINGLESPACE);
	registerEnum(L, CONST_ME_STONESSINGLESPACE);
	registerEnum(L, CONST_ME_BLUEGHOST);
	registerEnum(L, CONST_ME_POINTOFINTEREST);
	registerEnum(L, CONST_ME_MAPEFFECT);
	registerEnum(L, CONST_ME_PINKSPARK);
	registerEnum(L, CONST_ME_FIREWORK_GREEN);
	registerEnum(L, CONST_ME_FIREWORK_ORANGE);
	registerEnum(L, CONST_ME_FIREWORK_PURPLE);
	registerEnum(L, CONST_ME_FIREWORK_TURQUOISE);
	registerEnum(L, CONST_ME_THECUBE);
	registerEnum(L, CONST_ME_DRAWINK);
	registerEnum(L, CONST_ME_PRISMATICSPARKLES);
	registerEnum(L, CONST_ME_THAIAN);
	registerEnum(L, CONST_ME_THAIANGHOST);
	registerEnum(L, CONST_ME_GHOSTSMOKE);
	registerEnum(L, CONST_ME_FLOATINGBLOCK);
	registerEnum(L, CONST_ME_BLOCK);
	registerEnum(L, CONST_ME_ROOTING);
	registerEnum(L, CONST_ME_GHOSTLYSCRATCH);
	registerEnum(L, CONST_ME_GHOSTLYBITE);
	registerEnum(L, CONST_ME_BIGSCRATCHING);
	registerEnum(L, CONST_ME_SLASH);
	registerEnum(L, CONST_ME_BITE);
	registerEnum(L, CONST_ME_CHIVALRIOUSCHALLENGE);
	registerEnum(L, CONST_ME_DIVINEDAZZLE);
	registerEnum(L, CONST_ME_ELECTRICALSPARK);
	registerEnum(L, CONST_ME_PURPLETELEPORT);
	registerEnum(L, CONST_ME_REDTELEPORT);
	registerEnum(L, CONST_ME_ORANGETELEPORT);
	registerEnum(L, CONST_ME_GREYTELEPORT);
	registerEnum(L, CONST_ME_LIGHTBLUETELEPORT);
	registerEnum(L, CONST_ME_FATAL);
	registerEnum(L, CONST_ME_DODGE);
	registerEnum(L, CONST_ME_HOURGLASS);
	registerEnum(L, CONST_ME_FIREWORKSSTAR);
	registerEnum(L, CONST_ME_FIREWORKSCIRCLE);
	registerEnum(L, CONST_ME_FERUMBRAS_1);
	registerEnum(L, CONST_ME_GAZHARAGOTH);
	registerEnum(L, CONST_ME_MAD_MAGE);
	registerEnum(L, CONST_ME_HORESTIS);
	registerEnum(L, CONST_ME_DEVOVORGA);
	registerEnum(L, CONST_ME_FERUMBRAS_2);
	registerEnum(L, CONST_ME_FOAM);

	registerEnum(L, CONST_ANI_NONE);
	registerEnum(L, CONST_ANI_SPEAR);
	registerEnum(L, CONST_ANI_BOLT);
	registerEnum(L, CONST_ANI_ARROW);
	registerEnum(L, CONST_ANI_FIRE);
	registerEnum(L, CONST_ANI_ENERGY);
	registerEnum(L, CONST_ANI_POISONARROW);
	registerEnum(L, CONST_ANI_BURSTARROW);
	registerEnum(L, CONST_ANI_THROWINGSTAR);
	registerEnum(L, CONST_ANI_THROWINGKNIFE);
	registerEnum(L, CONST_ANI_SMALLSTONE);
	registerEnum(L, CONST_ANI_DEATH);
	registerEnum(L, CONST_ANI_LARGEROCK);
	registerEnum(L, CONST_ANI_SNOWBALL);
	registerEnum(L, CONST_ANI_POWERBOLT);
	registerEnum(L, CONST_ANI_POISON);
	registerEnum(L, CONST_ANI_INFERNALBOLT);
	registerEnum(L, CONST_ANI_HUNTINGSPEAR);
	registerEnum(L, CONST_ANI_ENCHANTEDSPEAR);
	registerEnum(L, CONST_ANI_REDSTAR);
	registerEnum(L, CONST_ANI_GREENSTAR);
	registerEnum(L, CONST_ANI_ROYALSPEAR);
	registerEnum(L, CONST_ANI_SNIPERARROW);
	registerEnum(L, CONST_ANI_ONYXARROW);
	registerEnum(L, CONST_ANI_PIERCINGBOLT);
	registerEnum(L, CONST_ANI_WHIRLWINDSWORD);
	registerEnum(L, CONST_ANI_WHIRLWINDAXE);
	registerEnum(L, CONST_ANI_WHIRLWINDCLUB);
	registerEnum(L, CONST_ANI_ETHEREALSPEAR);
	registerEnum(L, CONST_ANI_ICE);
	registerEnum(L, CONST_ANI_EARTH);
	registerEnum(L, CONST_ANI_HOLY);
	registerEnum(L, CONST_ANI_SUDDENDEATH);
	registerEnum(L, CONST_ANI_FLASHARROW);
	registerEnum(L, CONST_ANI_FLAMMINGARROW);
	registerEnum(L, CONST_ANI_SHIVERARROW);
	registerEnum(L, CONST_ANI_ENERGYBALL);
	registerEnum(L, CONST_ANI_SMALLICE);
	registerEnum(L, CONST_ANI_SMALLHOLY);
	registerEnum(L, CONST_ANI_SMALLEARTH);
	registerEnum(L, CONST_ANI_EARTHARROW);
	registerEnum(L, CONST_ANI_EXPLOSION);
	registerEnum(L, CONST_ANI_CAKE);
	registerEnum(L, CONST_ANI_TARSALARROW);
	registerEnum(L, CONST_ANI_VORTEXBOLT);
	registerEnum(L, CONST_ANI_PRISMATICBOLT);
	registerEnum(L, CONST_ANI_CRYSTALLINEARROW);
	registerEnum(L, CONST_ANI_DRILLBOLT);
	registerEnum(L, CONST_ANI_ENVENOMEDARROW);
	registerEnum(L, CONST_ANI_GLOOTHSPEAR);
	registerEnum(L, CONST_ANI_SIMPLEARROW);
	registerEnum(L, CONST_ANI_LEAFSTAR);
	registerEnum(L, CONST_ANI_DIAMONDARROW);
	registerEnum(L, CONST_ANI_SPECTRALBOLT);
	registerEnum(L, CONST_ANI_ROYALSTAR);
	registerEnum(L, CONST_ANI_WEAPONTYPE);

	registerEnum(L, CONST_PROP_BLOCKSOLID);
	registerEnum(L, CONST_PROP_HASHEIGHT);
	registerEnum(L, CONST_PROP_BLOCKPROJECTILE);
	registerEnum(L, CONST_PROP_BLOCKPATH);
	registerEnum(L, CONST_PROP_ISVERTICAL);
	registerEnum(L, CONST_PROP_ISHORIZONTAL);
	registerEnum(L, CONST_PROP_MOVEABLE);
	registerEnum(L, CONST_PROP_IMMOVABLEBLOCKSOLID);
	registerEnum(L, CONST_PROP_IMMOVABLEBLOCKPATH);
	registerEnum(L, CONST_PROP_IMMOVABLENOFIELDBLOCKPATH);
	registerEnum(L, CONST_PROP_NOFIELDBLOCKPATH);
	registerEnum(L, CONST_PROP_SUPPORTHANGABLE);

	registerEnum(L, CONST_SLOT_HEAD);
	registerEnum(L, CONST_SLOT_NECKLACE);
	registerEnum(L, CONST_SLOT_BACKPACK);
	registerEnum(L, CONST_SLOT_ARMOR);
	registerEnum(L, CONST_SLOT_RIGHT);
	registerEnum(L, CONST_SLOT_LEFT);
	registerEnum(L, CONST_SLOT_LEGS);
	registerEnum(L, CONST_SLOT_FEET);
	registerEnum(L, CONST_SLOT_RING);
	registerEnum(L, CONST_SLOT_AMMO);

	registerEnum(L, CREATURE_EVENT_NONE);
	registerEnum(L, CREATURE_EVENT_LOGIN);
	registerEnum(L, CREATURE_EVENT_LOGOUT);
	registerEnum(L, CREATURE_EVENT_RECONNECT);
	registerEnum(L, CREATURE_EVENT_THINK);
	registerEnum(L, CREATURE_EVENT_PREPAREDEATH);
	registerEnum(L, CREATURE_EVENT_DEATH);
	registerEnum(L, CREATURE_EVENT_KILL);
	registerEnum(L, CREATURE_EVENT_ADVANCE);
	registerEnum(L, CREATURE_EVENT_MODALWINDOW);
	registerEnum(L, CREATURE_EVENT_TEXTEDIT);
	registerEnum(L, CREATURE_EVENT_HEALTHCHANGE);
	registerEnum(L, CREATURE_EVENT_MANACHANGE);
	registerEnum(L, CREATURE_EVENT_EXTENDED_OPCODE);

	registerEnum(L, CREATURE_ID_MIN);
	registerEnum(L, CREATURE_ID_MAX);

	registerEnum(L, GAME_STATE_STARTUP);
	registerEnum(L, GAME_STATE_INIT);
	registerEnum(L, GAME_STATE_NORMAL);
	registerEnum(L, GAME_STATE_CLOSED);
	registerEnum(L, GAME_STATE_SHUTDOWN);
	registerEnum(L, GAME_STATE_CLOSING);
	registerEnum(L, GAME_STATE_MAINTAIN);

	registerEnum(L, ITEM_STACK_SIZE);

	registerEnum(L, MESSAGE_STATUS_DEFAULT);
	registerEnum(L, MESSAGE_STATUS_WARNING);
	registerEnum(L, MESSAGE_EVENT_ADVANCE);
	registerEnum(L, MESSAGE_STATUS_WARNING2);
	registerEnum(L, MESSAGE_STATUS_SMALL);
	registerEnum(L, MESSAGE_INFO_DESCR);
	registerEnum(L, MESSAGE_DAMAGE_DEALT);
	registerEnum(L, MESSAGE_DAMAGE_RECEIVED);
	registerEnum(L, MESSAGE_HEALED);
	registerEnum(L, MESSAGE_EXPERIENCE);
	registerEnum(L, MESSAGE_DAMAGE_OTHERS);
	registerEnum(L, MESSAGE_HEALED_OTHERS);
	registerEnum(L, MESSAGE_EXPERIENCE_OTHERS);
	registerEnum(L, MESSAGE_EVENT_DEFAULT);
	registerEnum(L, MESSAGE_LOOT);
	registerEnum(L, MESSAGE_TRADE);
	registerEnum(L, MESSAGE_GUILD);
	registerEnum(L, MESSAGE_PARTY_MANAGEMENT);
	registerEnum(L, MESSAGE_PARTY);
	registerEnum(L, MESSAGE_REPORT);
	registerEnum(L, MESSAGE_HOTKEY_PRESSED);
	registerEnum(L, MESSAGE_MARKET);
	registerEnum(L, MESSAGE_BEYOND_LAST);
	registerEnum(L, MESSAGE_TOURNAMENT_INFO);
	registerEnum(L, MESSAGE_ATTENTION);
	registerEnum(L, MESSAGE_BOOSTED_CREATURE);
	registerEnum(L, MESSAGE_OFFLINE_TRAINING);
	registerEnum(L, MESSAGE_TRANSACTION);

	registerEnum(L, CREATURETYPE_PLAYER);
	registerEnum(L, CREATURETYPE_MONSTER);
	registerEnum(L, CREATURETYPE_NPC);
	registerEnum(L, CREATURETYPE_SUMMON_OWN);
	registerEnum(L, CREATURETYPE_SUMMON_OTHERS);

	registerEnum(L, CLIENTOS_LINUX);
	registerEnum(L, CLIENTOS_WINDOWS);
	registerEnum(L, CLIENTOS_FLASH);
	registerEnum(L, CLIENTOS_OTCLIENT_LINUX);
	registerEnum(L, CLIENTOS_OTCLIENT_WINDOWS);
	registerEnum(L, CLIENTOS_OTCLIENT_MAC);

	registerEnum(L, FIGHTMODE_ATTACK);
	registerEnum(L, FIGHTMODE_BALANCED);
	registerEnum(L, FIGHTMODE_DEFENSE);

	registerEnum(L, ITEM_ATTRIBUTE_NONE);
	registerEnum(L, ITEM_ATTRIBUTE_ACTIONID);
	registerEnum(L, ITEM_ATTRIBUTE_UNIQUEID);
	registerEnum(L, ITEM_ATTRIBUTE_DESCRIPTION);
	registerEnum(L, ITEM_ATTRIBUTE_TEXT);
	registerEnum(L, ITEM_ATTRIBUTE_DATE);
	registerEnum(L, ITEM_ATTRIBUTE_WRITER);
	registerEnum(L, ITEM_ATTRIBUTE_NAME);
	registerEnum(L, ITEM_ATTRIBUTE_ARTICLE);
	registerEnum(L, ITEM_ATTRIBUTE_PLURALNAME);
	registerEnum(L, ITEM_ATTRIBUTE_WEIGHT);
	registerEnum(L, ITEM_ATTRIBUTE_ATTACK);
	registerEnum(L, ITEM_ATTRIBUTE_DEFENSE);
	registerEnum(L, ITEM_ATTRIBUTE_EXTRADEFENSE);
	registerEnum(L, ITEM_ATTRIBUTE_ARMOR);
	registerEnum(L, ITEM_ATTRIBUTE_HITCHANCE);
	registerEnum(L, ITEM_ATTRIBUTE_SHOOTRANGE);
	registerEnum(L, ITEM_ATTRIBUTE_OWNER);
	registerEnum(L, ITEM_ATTRIBUTE_DURATION);
	registerEnum(L, ITEM_ATTRIBUTE_DECAYSTATE);
	registerEnum(L, ITEM_ATTRIBUTE_CORPSEOWNER);
	registerEnum(L, ITEM_ATTRIBUTE_CHARGES);
	registerEnum(L, ITEM_ATTRIBUTE_FLUIDTYPE);
	registerEnum(L, ITEM_ATTRIBUTE_DOORID);
	registerEnum(L, ITEM_ATTRIBUTE_DECAYTO);
	registerEnum(L, ITEM_ATTRIBUTE_WRAPID);
	registerEnum(L, ITEM_ATTRIBUTE_STOREITEM);
	registerEnum(L, ITEM_ATTRIBUTE_ATTACK_SPEED);
	registerEnum(L, ITEM_ATTRIBUTE_OPENCONTAINER);
	registerEnum(L, ITEM_ATTRIBUTE_DURATION_MIN);
	registerEnum(L, ITEM_ATTRIBUTE_DURATION_MAX);

	registerEnum(L, ITEM_BROWSEFIELD);
	registerEnum(L, ITEM_BAG);
	registerEnum(L, ITEM_SHOPPING_BAG);
	registerEnum(L, ITEM_GOLD_COIN);
	registerEnum(L, ITEM_PLATINUM_COIN);
	registerEnum(L, ITEM_CRYSTAL_COIN);
	registerEnum(L, ITEM_AMULETOFLOSS);
	registerEnum(L, ITEM_PARCEL);
	registerEnum(L, ITEM_LABEL);
	registerEnum(L, ITEM_FIREFIELD_PVP_FULL);
	registerEnum(L, ITEM_FIREFIELD_PVP_MEDIUM);
	registerEnum(L, ITEM_FIREFIELD_PVP_SMALL);
	registerEnum(L, ITEM_FIREFIELD_PERSISTENT_FULL);
	registerEnum(L, ITEM_FIREFIELD_PERSISTENT_MEDIUM);
	registerEnum(L, ITEM_FIREFIELD_PERSISTENT_SMALL);
	registerEnum(L, ITEM_FIREFIELD_NOPVP);
	registerEnum(L, ITEM_FIREFIELD_NOPVP_MEDIUM);
	registerEnum(L, ITEM_POISONFIELD_PVP);
	registerEnum(L, ITEM_POISONFIELD_PERSISTENT);
	registerEnum(L, ITEM_POISONFIELD_NOPVP);
	registerEnum(L, ITEM_ENERGYFIELD_PVP);
	registerEnum(L, ITEM_ENERGYFIELD_PERSISTENT);
	registerEnum(L, ITEM_ENERGYFIELD_NOPVP);
	registerEnum(L, ITEM_MAGICWALL);
	registerEnum(L, ITEM_MAGICWALL_PERSISTENT);
	registerEnum(L, ITEM_MAGICWALL_SAFE);
	registerEnum(L, ITEM_WILDGROWTH);
	registerEnum(L, ITEM_WILDGROWTH_PERSISTENT);
	registerEnum(L, ITEM_WILDGROWTH_SAFE);
	registerEnum(L, ITEM_DECORATION_KIT);
	registerEnum(L, ITEM_MARKET);

	registerEnum(L, WIELDINFO_NONE);
	registerEnum(L, WIELDINFO_LEVEL);
	registerEnum(L, WIELDINFO_MAGLV);
	registerEnum(L, WIELDINFO_VOCREQ);
	registerEnum(L, WIELDINFO_PREMIUM);

	registerEnum(L, PlayerFlag_CannotUseCombat);
	registerEnum(L, PlayerFlag_CannotAttackPlayer);
	registerEnum(L, PlayerFlag_CannotAttackMonster);
	registerEnum(L, PlayerFlag_CannotBeAttacked);
	registerEnum(L, PlayerFlag_CanConvinceAll);
	registerEnum(L, PlayerFlag_CanSummonAll);
	registerEnum(L, PlayerFlag_CanIllusionAll);
	registerEnum(L, PlayerFlag_CanSenseInvisibility);
	registerEnum(L, PlayerFlag_IgnoredByMonsters);
	registerEnum(L, PlayerFlag_NotGainInFight);
	registerEnum(L, PlayerFlag_HasInfiniteMana);
	registerEnum(L, PlayerFlag_HasInfiniteSoul);
	registerEnum(L, PlayerFlag_HasNoExhaustion);
	registerEnum(L, PlayerFlag_CannotUseSpells);
	registerEnum(L, PlayerFlag_CannotPickupItem);
	registerEnum(L, PlayerFlag_CanAlwaysLogin);
	registerEnum(L, PlayerFlag_CanBroadcast);
	registerEnum(L, PlayerFlag_CanEditHouses);
	registerEnum(L, PlayerFlag_CannotBeBanned);
	registerEnum(L, PlayerFlag_CannotBePushed);
	registerEnum(L, PlayerFlag_HasInfiniteCapacity);
	registerEnum(L, PlayerFlag_CanPushAllCreatures);
	registerEnum(L, PlayerFlag_CanTalkRedPrivate);
	registerEnum(L, PlayerFlag_CanTalkRedChannel);
	registerEnum(L, PlayerFlag_TalkOrangeHelpChannel);
	registerEnum(L, PlayerFlag_NotGainExperience);
	registerEnum(L, PlayerFlag_NotGainMana);
	registerEnum(L, PlayerFlag_NotGainHealth);
	registerEnum(L, PlayerFlag_NotGainSkill);
	registerEnum(L, PlayerFlag_SetMaxSpeed);
	registerEnum(L, PlayerFlag_SpecialVIP);
	registerEnum(L, PlayerFlag_NotGenerateLoot);
	registerEnum(L, PlayerFlag_IgnoreProtectionZone);
	registerEnum(L, PlayerFlag_IgnoreSpellCheck);
	registerEnum(L, PlayerFlag_IgnoreWeaponCheck);
	registerEnum(L, PlayerFlag_CannotBeMuted);
	registerEnum(L, PlayerFlag_IsAlwaysPremium);
	registerEnum(L, PlayerFlag_IgnoreYellCheck);
	registerEnum(L, PlayerFlag_IgnoreSendPrivateCheck);

	registerEnum(L, PODIUM_SHOW_PLATFORM);
	registerEnum(L, PODIUM_SHOW_OUTFIT);
	registerEnum(L, PODIUM_SHOW_MOUNT);

	registerEnum(L, PLAYERSEX_FEMALE);
	registerEnum(L, PLAYERSEX_MALE);

	registerEnum(L, REPORT_REASON_NAMEINAPPROPRIATE);
	registerEnum(L, REPORT_REASON_NAMEPOORFORMATTED);
	registerEnum(L, REPORT_REASON_NAMEADVERTISING);
	registerEnum(L, REPORT_REASON_NAMEUNFITTING);
	registerEnum(L, REPORT_REASON_NAMERULEVIOLATION);
	registerEnum(L, REPORT_REASON_INSULTINGSTATEMENT);
	registerEnum(L, REPORT_REASON_SPAMMING);
	registerEnum(L, REPORT_REASON_ADVERTISINGSTATEMENT);
	registerEnum(L, REPORT_REASON_UNFITTINGSTATEMENT);
	registerEnum(L, REPORT_REASON_LANGUAGESTATEMENT);
	registerEnum(L, REPORT_REASON_DISCLOSURE);
	registerEnum(L, REPORT_REASON_RULEVIOLATION);
	registerEnum(L, REPORT_REASON_STATEMENT_BUGABUSE);
	registerEnum(L, REPORT_REASON_UNOFFICIALSOFTWARE);
	registerEnum(L, REPORT_REASON_PRETENDING);
	registerEnum(L, REPORT_REASON_HARASSINGOWNERS);
	registerEnum(L, REPORT_REASON_FALSEINFO);
	registerEnum(L, REPORT_REASON_ACCOUNTSHARING);
	registerEnum(L, REPORT_REASON_STEALINGDATA);
	registerEnum(L, REPORT_REASON_SERVICEATTACKING);
	registerEnum(L, REPORT_REASON_SERVICEAGREEMENT);

	registerEnum(L, REPORT_TYPE_NAME);
	registerEnum(L, REPORT_TYPE_STATEMENT);
	registerEnum(L, REPORT_TYPE_BOT);

	registerEnum(L, VOCATION_NONE);

	registerEnum(L, SKILL_FIST);
	registerEnum(L, SKILL_CLUB);
	registerEnum(L, SKILL_SWORD);
	registerEnum(L, SKILL_AXE);
	registerEnum(L, SKILL_DISTANCE);
	registerEnum(L, SKILL_SHIELD);
	registerEnum(L, SKILL_FISHING);
	registerEnum(L, SKILL_MAGLEVEL);
	registerEnum(L, SKILL_LEVEL);

	registerEnum(L, SPECIALSKILL_CRITICALHITCHANCE);
	registerEnum(L, SPECIALSKILL_CRITICALHITAMOUNT);
	registerEnum(L, SPECIALSKILL_LIFELEECHCHANCE);
	registerEnum(L, SPECIALSKILL_LIFELEECHAMOUNT);
	registerEnum(L, SPECIALSKILL_MANALEECHCHANCE);
	registerEnum(L, SPECIALSKILL_MANALEECHAMOUNT);

	registerEnum(L, STAT_MAXHITPOINTS);
	registerEnum(L, STAT_MAXMANAPOINTS);
	registerEnum(L, STAT_SOULPOINTS);
	registerEnum(L, STAT_MAGICPOINTS);

	registerEnum(L, SKULL_NONE);
	registerEnum(L, SKULL_YELLOW);
	registerEnum(L, SKULL_GREEN);
	registerEnum(L, SKULL_WHITE);
	registerEnum(L, SKULL_RED);
	registerEnum(L, SKULL_BLACK);
	registerEnum(L, SKULL_ORANGE);

	registerEnum(L, FLUID_NONE);
	registerEnum(L, FLUID_WATER);
	registerEnum(L, FLUID_BLOOD);
	registerEnum(L, FLUID_BEER);
	registerEnum(L, FLUID_SLIME);
	registerEnum(L, FLUID_LEMONADE);
	registerEnum(L, FLUID_MILK);
	registerEnum(L, FLUID_MANA);
	registerEnum(L, FLUID_LIFE);
	registerEnum(L, FLUID_OIL);
	registerEnum(L, FLUID_URINE);
	registerEnum(L, FLUID_COCONUTMILK);
	registerEnum(L, FLUID_WINE);
	registerEnum(L, FLUID_MUD);
	registerEnum(L, FLUID_FRUITJUICE);
	registerEnum(L, FLUID_LAVA);
	registerEnum(L, FLUID_RUM);
	registerEnum(L, FLUID_SWAMP);
	registerEnum(L, FLUID_TEA);
	registerEnum(L, FLUID_MEAD);

	registerEnum(L, TALKTYPE_SAY);
	registerEnum(L, TALKTYPE_WHISPER);
	registerEnum(L, TALKTYPE_YELL);
	registerEnum(L, TALKTYPE_PRIVATE_FROM);
	registerEnum(L, TALKTYPE_PRIVATE_TO);
	registerEnum(L, TALKTYPE_CHANNEL_Y);
	registerEnum(L, TALKTYPE_CHANNEL_O);
	registerEnum(L, TALKTYPE_SPELL);
	registerEnum(L, TALKTYPE_PRIVATE_NP);
	registerEnum(L, TALKTYPE_PRIVATE_NP_CONSOLE);
	registerEnum(L, TALKTYPE_PRIVATE_PN);
	registerEnum(L, TALKTYPE_BROADCAST);
	registerEnum(L, TALKTYPE_CHANNEL_R1);
	registerEnum(L, TALKTYPE_PRIVATE_RED_FROM);
	registerEnum(L, TALKTYPE_PRIVATE_RED_TO);
	registerEnum(L, TALKTYPE_MONSTER_SAY);
	registerEnum(L, TALKTYPE_MONSTER_YELL);
	registerEnum(L, TALKTYPE_POTION);

	registerEnum(L, TEXTCOLOR_BLUE);
	registerEnum(L, TEXTCOLOR_LIGHTGREEN);
	registerEnum(L, TEXTCOLOR_LIGHTBLUE);
	registerEnum(L, TEXTCOLOR_MAYABLUE);
	registerEnum(L, TEXTCOLOR_DARKRED);
	registerEnum(L, TEXTCOLOR_LIGHTGREY);
	registerEnum(L, TEXTCOLOR_SKYBLUE);
	registerEnum(L, TEXTCOLOR_PURPLE);
	registerEnum(L, TEXTCOLOR_ELECTRICPURPLE);
	registerEnum(L, TEXTCOLOR_RED);
	registerEnum(L, TEXTCOLOR_PASTELRED);
	registerEnum(L, TEXTCOLOR_ORANGE);
	registerEnum(L, TEXTCOLOR_YELLOW);
	registerEnum(L, TEXTCOLOR_WHITE_EXP);
	registerEnum(L, TEXTCOLOR_NONE);

	registerEnum(L, TILESTATE_NONE);
	registerEnum(L, TILESTATE_PROTECTIONZONE);
	registerEnum(L, TILESTATE_NOPVPZONE);
	registerEnum(L, TILESTATE_NOLOGOUT);
	registerEnum(L, TILESTATE_PVPZONE);
	registerEnum(L, TILESTATE_FLOORCHANGE);
	registerEnum(L, TILESTATE_FLOORCHANGE_DOWN);
	registerEnum(L, TILESTATE_FLOORCHANGE_NORTH);
	registerEnum(L, TILESTATE_FLOORCHANGE_SOUTH);
	registerEnum(L, TILESTATE_FLOORCHANGE_EAST);
	registerEnum(L, TILESTATE_FLOORCHANGE_WEST);
	registerEnum(L, TILESTATE_TELEPORT);
	registerEnum(L, TILESTATE_MAGICFIELD);
	registerEnum(L, TILESTATE_MAILBOX);
	registerEnum(L, TILESTATE_TRASHHOLDER);
	registerEnum(L, TILESTATE_BED);
	registerEnum(L, TILESTATE_DEPOT);
	registerEnum(L, TILESTATE_BLOCKSOLID);
	registerEnum(L, TILESTATE_BLOCKPATH);
	registerEnum(L, TILESTATE_IMMOVABLEBLOCKSOLID);
	registerEnum(L, TILESTATE_IMMOVABLEBLOCKPATH);
	registerEnum(L, TILESTATE_IMMOVABLENOFIELDBLOCKPATH);
	registerEnum(L, TILESTATE_NOFIELDBLOCKPATH);
	registerEnum(L, TILESTATE_FLOORCHANGE_SOUTH_ALT);
	registerEnum(L, TILESTATE_FLOORCHANGE_EAST_ALT);
	registerEnum(L, TILESTATE_SUPPORTS_HANGABLE);

	registerEnum(L, WEAPON_NONE);
	registerEnum(L, WEAPON_SWORD);
	registerEnum(L, WEAPON_CLUB);
	registerEnum(L, WEAPON_AXE);
	registerEnum(L, WEAPON_SHIELD);
	registerEnum(L, WEAPON_DISTANCE);
	registerEnum(L, WEAPON_WAND);
	registerEnum(L, WEAPON_AMMO);
	registerEnum(L, WEAPON_QUIVER);

	registerEnum(L, WORLD_TYPE_NO_PVP);
	registerEnum(L, WORLD_TYPE_PVP);
	registerEnum(L, WORLD_TYPE_PVP_ENFORCED);

	// Use with container:addItem, container:addItemEx and possibly other functions.
	registerEnum(L, FLAG_NOLIMIT);
	registerEnum(L, FLAG_IGNOREBLOCKITEM);
	registerEnum(L, FLAG_IGNOREBLOCKCREATURE);
	registerEnum(L, FLAG_CHILDISOWNER);
	registerEnum(L, FLAG_PATHFINDING);
	registerEnum(L, FLAG_IGNOREFIELDDAMAGE);
	registerEnum(L, FLAG_IGNORENOTMOVEABLE);
	registerEnum(L, FLAG_IGNOREAUTOSTACK);

	// Use with itemType:getSlotPosition
	registerEnum(L, SLOTP_WHEREEVER);
	registerEnum(L, SLOTP_HEAD);
	registerEnum(L, SLOTP_NECKLACE);
	registerEnum(L, SLOTP_BACKPACK);
	registerEnum(L, SLOTP_ARMOR);
	registerEnum(L, SLOTP_RIGHT);
	registerEnum(L, SLOTP_LEFT);
	registerEnum(L, SLOTP_LEGS);
	registerEnum(L, SLOTP_FEET);
	registerEnum(L, SLOTP_RING);
	registerEnum(L, SLOTP_AMMO);
	registerEnum(L, SLOTP_DEPOT);
	registerEnum(L, SLOTP_TWO_HAND);

	// Use with combat functions
	registerEnum(L, ORIGIN_NONE);
	registerEnum(L, ORIGIN_CONDITION);
	registerEnum(L, ORIGIN_SPELL);
	registerEnum(L, ORIGIN_MELEE);
	registerEnum(L, ORIGIN_RANGED);
	registerEnum(L, ORIGIN_WAND);

	// Use with house:getAccessList, house:setAccessList
	registerEnum(L, GUEST_LIST);
	registerEnum(L, SUBOWNER_LIST);

	// Use with npc:setSpeechBubble
	registerEnum(L, SPEECHBUBBLE_NONE);
	registerEnum(L, SPEECHBUBBLE_NORMAL);
	registerEnum(L, SPEECHBUBBLE_TRADE);
	registerEnum(L, SPEECHBUBBLE_QUEST);
	registerEnum(L, SPEECHBUBBLE_COMPASS);
	registerEnum(L, SPEECHBUBBLE_NORMAL2);
	registerEnum(L, SPEECHBUBBLE_NORMAL3);
	registerEnum(L, SPEECHBUBBLE_HIRELING);

	// Use with player:addMapMark
	registerEnum(L, MAPMARK_TICK);
	registerEnum(L, MAPMARK_QUESTION);
	registerEnum(L, MAPMARK_EXCLAMATION);
	registerEnum(L, MAPMARK_STAR);
	registerEnum(L, MAPMARK_CROSS);
	registerEnum(L, MAPMARK_TEMPLE);
	registerEnum(L, MAPMARK_KISS);
	registerEnum(L, MAPMARK_SHOVEL);
	registerEnum(L, MAPMARK_SWORD);
	registerEnum(L, MAPMARK_FLAG);
	registerEnum(L, MAPMARK_LOCK);
	registerEnum(L, MAPMARK_BAG);
	registerEnum(L, MAPMARK_SKULL);
	registerEnum(L, MAPMARK_DOLLAR);
	registerEnum(L, MAPMARK_REDNORTH);
	registerEnum(L, MAPMARK_REDSOUTH);
	registerEnum(L, MAPMARK_REDEAST);
	registerEnum(L, MAPMARK_REDWEST);
	registerEnum(L, MAPMARK_GREENNORTH);
	registerEnum(L, MAPMARK_GREENSOUTH);

	// Use with Game.getReturnMessage
	registerEnum(L, RETURNVALUE_NOERROR);
	registerEnum(L, RETURNVALUE_NOTPOSSIBLE);
	registerEnum(L, RETURNVALUE_NOTENOUGHROOM);
	registerEnum(L, RETURNVALUE_PLAYERISPZLOCKED);
	registerEnum(L, RETURNVALUE_PLAYERISNOTINVITED);
	registerEnum(L, RETURNVALUE_CANNOTTHROW);
	registerEnum(L, RETURNVALUE_THEREISNOWAY);
	registerEnum(L, RETURNVALUE_DESTINATIONOUTOFREACH);
	registerEnum(L, RETURNVALUE_CREATUREBLOCK);
	registerEnum(L, RETURNVALUE_NOTMOVEABLE);
	registerEnum(L, RETURNVALUE_DROPTWOHANDEDITEM);
	registerEnum(L, RETURNVALUE_BOTHHANDSNEEDTOBEFREE);
	registerEnum(L, RETURNVALUE_CANONLYUSEONEWEAPON);
	registerEnum(L, RETURNVALUE_NEEDEXCHANGE);
	registerEnum(L, RETURNVALUE_CANNOTBEDRESSED);
	registerEnum(L, RETURNVALUE_PUTTHISOBJECTINYOURHAND);
	registerEnum(L, RETURNVALUE_PUTTHISOBJECTINBOTHHANDS);
	registerEnum(L, RETURNVALUE_TOOFARAWAY);
	registerEnum(L, RETURNVALUE_FIRSTGODOWNSTAIRS);
	registerEnum(L, RETURNVALUE_FIRSTGOUPSTAIRS);
	registerEnum(L, RETURNVALUE_CONTAINERNOTENOUGHROOM);
	registerEnum(L, RETURNVALUE_NOTENOUGHCAPACITY);
	registerEnum(L, RETURNVALUE_CANNOTPICKUP);
	registerEnum(L, RETURNVALUE_THISISIMPOSSIBLE);
	registerEnum(L, RETURNVALUE_DEPOTISFULL);
	registerEnum(L, RETURNVALUE_CREATUREDOESNOTEXIST);
	registerEnum(L, RETURNVALUE_CANNOTUSETHISOBJECT);
	registerEnum(L, RETURNVALUE_PLAYERWITHTHISNAMEISNOTONLINE);
	registerEnum(L, RETURNVALUE_NOTREQUIREDLEVELTOUSERUNE);
	registerEnum(L, RETURNVALUE_YOUAREALREADYTRADING);
	registerEnum(L, RETURNVALUE_THISPLAYERISALREADYTRADING);
	registerEnum(L, RETURNVALUE_YOUMAYNOTLOGOUTDURINGAFIGHT);
	registerEnum(L, RETURNVALUE_DIRECTPLAYERSHOOT);
	registerEnum(L, RETURNVALUE_NOTENOUGHLEVEL);
	registerEnum(L, RETURNVALUE_NOTENOUGHMAGICLEVEL);
	registerEnum(L, RETURNVALUE_NOTENOUGHMANA);
	registerEnum(L, RETURNVALUE_NOTENOUGHSOUL);
	registerEnum(L, RETURNVALUE_YOUAREEXHAUSTED);
	registerEnum(L, RETURNVALUE_YOUCANNOTUSEOBJECTSTHATFAST);
	registerEnum(L, RETURNVALUE_PLAYERISNOTREACHABLE);
	registerEnum(L, RETURNVALUE_CANONLYUSETHISRUNEONCREATURES);
	registerEnum(L, RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE);
	registerEnum(L, RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
	registerEnum(L, RETURNVALUE_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE);
	registerEnum(L, RETURNVALUE_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE);
	registerEnum(L, RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE);
	registerEnum(L, RETURNVALUE_YOUCANONLYUSEITONCREATURES);
	registerEnum(L, RETURNVALUE_CREATUREISNOTREACHABLE);
	registerEnum(L, RETURNVALUE_TURNSECUREMODETOATTACKUNMARKEDPLAYERS);
	registerEnum(L, RETURNVALUE_YOUNEEDPREMIUMACCOUNT);
	registerEnum(L, RETURNVALUE_YOUNEEDTOLEARNTHISSPELL);
	registerEnum(L, RETURNVALUE_YOURVOCATIONCANNOTUSETHISSPELL);
	registerEnum(L, RETURNVALUE_YOUNEEDAWEAPONTOUSETHISSPELL);
	registerEnum(L, RETURNVALUE_PLAYERISPZLOCKEDLEAVEPVPZONE);
	registerEnum(L, RETURNVALUE_PLAYERISPZLOCKEDENTERPVPZONE);
	registerEnum(L, RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE);
	registerEnum(L, RETURNVALUE_YOUCANNOTLOGOUTHERE);
	registerEnum(L, RETURNVALUE_YOUNEEDAMAGICITEMTOCASTSPELL);
	registerEnum(L, RETURNVALUE_NAMEISTOOAMBIGUOUS);
	registerEnum(L, RETURNVALUE_CANONLYUSEONESHIELD);
	registerEnum(L, RETURNVALUE_NOPARTYMEMBERSINRANGE);
	registerEnum(L, RETURNVALUE_YOUARENOTTHEOWNER);
	registerEnum(L, RETURNVALUE_TRADEPLAYERFARAWAY);
	registerEnum(L, RETURNVALUE_YOUDONTOWNTHISHOUSE);
	registerEnum(L, RETURNVALUE_TRADEPLAYERALREADYOWNSAHOUSE);
	registerEnum(L, RETURNVALUE_TRADEPLAYERHIGHESTBIDDER);
	registerEnum(L, RETURNVALUE_YOUCANNOTTRADETHISHOUSE);
	registerEnum(L, RETURNVALUE_YOUDONTHAVEREQUIREDPROFESSION);
	registerEnum(L, RETURNVALUE_YOUCANNOTUSETHISBED);

	registerEnum(L, RELOAD_TYPE_ALL);
	registerEnum(L, RELOAD_TYPE_ACTIONS);
	registerEnum(L, RELOAD_TYPE_CHAT);
	registerEnum(L, RELOAD_TYPE_CONFIG);
	registerEnum(L, RELOAD_TYPE_CREATURESCRIPTS);
	registerEnum(L, RELOAD_TYPE_EVENTS);
	registerEnum(L, RELOAD_TYPE_GLOBAL);
	registerEnum(L, RELOAD_TYPE_GLOBALEVENTS);
	registerEnum(L, RELOAD_TYPE_ITEMS);
	registerEnum(L, RELOAD_TYPE_MONSTERS);
	registerEnum(L, RELOAD_TYPE_MOUNTS);
	registerEnum(L, RELOAD_TYPE_MOVEMENTS);
	registerEnum(L, RELOAD_TYPE_NPCS);
	registerEnum(L, RELOAD_TYPE_QUESTS);
	registerEnum(L, RELOAD_TYPE_SCRIPTS);
	registerEnum(L, RELOAD_TYPE_SPELLS);
	registerEnum(L, RELOAD_TYPE_TALKACTIONS);
	registerEnum(L, RELOAD_TYPE_WEAPONS);

	registerEnum(L, ZONE_PROTECTION);
	registerEnum(L, ZONE_NOPVP);
	registerEnum(L, ZONE_PVP);
	registerEnum(L, ZONE_NOLOGOUT);
	registerEnum(L, ZONE_NORMAL);

	registerEnum(L, MAX_LOOTCHANCE);

	registerEnum(L, SPELL_INSTANT);
	registerEnum(L, SPELL_RUNE);

	registerEnum(L, MONSTERS_EVENT_THINK);
	registerEnum(L, MONSTERS_EVENT_APPEAR);
	registerEnum(L, MONSTERS_EVENT_DISAPPEAR);
	registerEnum(L, MONSTERS_EVENT_MOVE);
	registerEnum(L, MONSTERS_EVENT_SAY);

	registerEnum(L, DECAYING_FALSE);
	registerEnum(L, DECAYING_TRUE);
	registerEnum(L, DECAYING_PENDING);

	registerEnum(L, RESOURCE_BANK_BALANCE);
	registerEnum(L, RESOURCE_GOLD_EQUIPPED);
	registerEnum(L, RESOURCE_PREY_WILDCARDS);
	registerEnum(L, RESOURCE_DAILYREWARD_STREAK);
	registerEnum(L, RESOURCE_DAILYREWARD_JOKERS);

	registerEnum(L, CREATURE_ICON_CROSS_WHITE);
	registerEnum(L, CREATURE_ICON_CROSS_WHITE_RED);
	registerEnum(L, CREATURE_ICON_ORB_RED);
	registerEnum(L, CREATURE_ICON_ORB_GREEN);
	registerEnum(L, CREATURE_ICON_ORB_RED_GREEN);
	registerEnum(L, CREATURE_ICON_GEM_GREEN);
	registerEnum(L, CREATURE_ICON_GEM_YELLOW);
	registerEnum(L, CREATURE_ICON_GEM_BLUE);
	registerEnum(L, CREATURE_ICON_GEM_PURPLE);
	registerEnum(L, CREATURE_ICON_GEM_RED);
	registerEnum(L, CREATURE_ICON_PIGEON);
	registerEnum(L, CREATURE_ICON_ENERGY);
	registerEnum(L, CREATURE_ICON_POISON);
	registerEnum(L, CREATURE_ICON_WATER);
	registerEnum(L, CREATURE_ICON_FIRE);
	registerEnum(L, CREATURE_ICON_ICE);
	registerEnum(L, CREATURE_ICON_ARROW_UP);
	registerEnum(L, CREATURE_ICON_ARROW_DOWN);
	registerEnum(L, CREATURE_ICON_WARNING);
	registerEnum(L, CREATURE_ICON_QUESTION);
	registerEnum(L, CREATURE_ICON_CROSS_RED);
	registerEnum(L, CREATURE_ICON_FIRST);
	registerEnum(L, CREATURE_ICON_LAST);

	registerEnum(L, MONSTER_ICON_VULNERABLE);
	registerEnum(L, MONSTER_ICON_WEAKENED);
	registerEnum(L, MONSTER_ICON_MELEE);
	registerEnum(L, MONSTER_ICON_INFLUENCED);
	registerEnum(L, MONSTER_ICON_FIENDISH);
	registerEnum(L, MONSTER_ICON_FIRST);
	registerEnum(L, MONSTER_ICON_LAST);

	// _G
	registerGlobalVariable(L, "INDEX_WHEREEVER", INDEX_WHEREEVER);
	registerGlobalBoolean(L, "VIRTUAL_PARENT", true);

	registerGlobalMethod(L, "isType", LuaScriptInterface::luaIsType);
	registerGlobalMethod(L, "rawgetmetatable", LuaScriptInterface::luaRawGetMetatable);


	// os
	registerMethod(L, "os", "mtime", LuaScriptInterface::luaSystemTime);

	// table
	registerMethod(L, "table", "create", LuaScriptInterface::luaTableCreate);
	registerMethod(L, "table", "pack", LuaScriptInterface::luaTablePack);
}

#undef registerEnum
#undef registerEnumIn

// Push

void tfs::lua::registerMethod(lua_State* L, std::string_view globalName, std::string_view methodName,
                              lua_CFunction func)
{
	// globalName.methodName = func
	lua_getglobal(L, globalName.data());
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, methodName.data());

	// pop globalName
	lua_pop(L, 1);
}

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

// _G
int LuaScriptInterface::luaIsType(lua_State* L)
{
	// isType(derived, base)
	lua_getmetatable(L, -2);
	lua_getmetatable(L, -2);

	lua_rawgeti(L, -2, 'p');
	uint_fast8_t parentsB = tfs::lua::getNumber<uint_fast8_t>(L, 1);

	lua_rawgeti(L, -3, 'h');
	size_t hashB = tfs::lua::getNumber<size_t>(L, 1);

	lua_rawgeti(L, -3, 'p');
	uint_fast8_t parentsA = tfs::lua::getNumber<uint_fast8_t>(L, 1);
	for (uint_fast8_t i = parentsA; i < parentsB; ++i) {
		lua_getfield(L, -3, "__index");
		lua_replace(L, -4);
	}

	lua_rawgeti(L, -4, 'h');
	size_t hashA = tfs::lua::getNumber<size_t>(L, 1);

	tfs::lua::pushBoolean(L, hashA == hashB);
	return 1;
}

int LuaScriptInterface::luaRawGetMetatable(lua_State* L)
{
	// rawgetmetatable(metatableName)
	luaL_getmetatable(L, tfs::lua::getString(L, 1).data());
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
