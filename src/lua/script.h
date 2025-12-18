#pragma once

#include "../enums.h"
#include "../position.h"

#if LUA_VERSION_NUM >= 502
#ifndef LUA_COMPAT_ALL
#ifndef LUA_COMPAT_MODULE
#define luaL_register(L, libname, l) (luaL_newlib(L, l), lua_pushvalue(L, -1), lua_setglobal(L, libname))
#endif
#undef lua_equal
#define lua_equal(L, i1, i2) lua_compare(L, (i1), (i2), LUA_OPEQ)
#endif
#endif

class AreaCombat;
class Combat;
class Container;
class Creature;
class Spell;
class Item;
class LuaScriptInterface;
class LuaVariant;
class Npc;
class Player;
class Thing;
struct Outfit;

using Combat_ptr = std::shared_ptr<Combat>;

inline constexpr int32_t EVENT_ID_USER = 1000;

struct LuaTimerEventDesc
{
	int32_t scriptId = -1;
	int32_t function = -1;
	std::vector<int32_t> parameters;
	uint32_t eventId = 0;

	LuaTimerEventDesc() = default;
	LuaTimerEventDesc(LuaTimerEventDesc&& other) = default;
};

class LuaScriptInterface
{
public:
	explicit LuaScriptInterface(std::string interfaceName);
	virtual ~LuaScriptInterface();

	// non-copyable
	LuaScriptInterface(const LuaScriptInterface&) = delete;
	LuaScriptInterface& operator=(const LuaScriptInterface&) = delete;

	virtual bool initState();
	bool reInitState();

	int32_t loadFile(const std::string& file, const std::shared_ptr<Npc>& npc = nullptr);

	const std::string& getFileById(int32_t scriptId);
	int32_t getEvent(std::string_view eventName);
	int32_t getEvent();
	int32_t getMetaEvent(const std::string& globalName, const std::string& eventName);

	const std::string& getInterfaceName() const { return interfaceName; }
	const std::string& getLastLuaError() const { return lastLuaError; }

	lua_State* getLuaState() const { return L; }

	bool pushFunction(int32_t functionId);

	bool callFunction(int params);
	void callVoidFunction(int params);

	void registerGlobalMethod(std::string_view functionName, lua_CFunction func);
	void registerGlobalVariable(std::string_view name, lua_Number value);
	void registerGlobalBoolean(std::string_view name, bool value);

	void registerTable(std::string_view tableName);
	void registerClass(std::string_view className, std::string_view baseClass, lua_CFunction newFunction = nullptr);
	void registerMetaMethod(std::string_view className, std::string_view methodName, lua_CFunction func);
	void registerMethod(std::string_view globalName, std::string_view methodName, lua_CFunction func);
	void registerVariable(std::string_view tableName, std::string_view name, lua_Number value);

	static const luaL_Reg luaBitReg[7];
	static const luaL_Reg luaConfigManagerTable[4];
	static const luaL_Reg luaDatabaseTable[9];
	static const luaL_Reg luaResultTable[6];

protected:
	virtual bool closeState();

	void registerFunctions();

	lua_State* L = nullptr;

	int32_t eventTableRef = -1;
	int32_t runningEventId = EVENT_ID_USER;

	// script file cache
	std::map<int32_t, std::string> cacheFiles;

private:
	// lua functions
	static int luaDoPlayerAddItem(lua_State* L);

	// get item info
	static int luaGetDepotId(lua_State* L);

	// get world info
	static int luaGetWorldUpTime(lua_State* L);

	// get subtype name
	static int luaGetSubTypeName(lua_State* L);

	// type validation
	static int luaIsDepot(lua_State* L);
	static int luaIsMoveable(lua_State* L);
	static int luaIsValidUID(lua_State* L);

	//
	static int luaCreateCombatArea(lua_State* L);

	static int luaDoAreaCombat(lua_State* L);
	static int luaDoTargetCombat(lua_State* L);

	static int luaDoChallengeCreature(lua_State* L);

	static int luaDebugPrint(lua_State* L);
	static int luaAddEvent(lua_State* L);
	static int luaStopEvent(lua_State* L);

	static int luaSaveServer(lua_State* L);
	static int luaCleanMap(lua_State* L);

	static int luaIsInWar(lua_State* L);

	static int luaGetWaypointPositionByName(lua_State* L);

	static int luaSendChannelMessage(lua_State* L);
	static int luaSendGuildChannelMessage(lua_State* L);

	static int luaIsScriptsInterface(lua_State* L);

	static int luaBitNot(lua_State* L);
	static int luaBitAnd(lua_State* L);
	static int luaBitOr(lua_State* L);
	static int luaBitXor(lua_State* L);
	static int luaBitLeftShift(lua_State* L);
	static int luaBitRightShift(lua_State* L);

	static int luaConfigManagerGetString(lua_State* L);
	static int luaConfigManagerGetNumber(lua_State* L);
	static int luaConfigManagerGetBoolean(lua_State* L);

	static int luaDatabaseExecute(lua_State* L);
	static int luaDatabaseAsyncExecute(lua_State* L);
	static int luaDatabaseStoreQuery(lua_State* L);
	static int luaDatabaseAsyncStoreQuery(lua_State* L);
	static int luaDatabaseEscapeString(lua_State* L);
	static int luaDatabaseEscapeBlob(lua_State* L);
	static int luaDatabaseLastInsertId(lua_State* L);
	static int luaDatabaseTableExists(lua_State* L);

	static int luaResultGetNumber(lua_State* L);
	static int luaResultGetString(lua_State* L);
	static int luaResultGetStream(lua_State* L);
	static int luaResultNext(lua_State* L);
	static int luaResultFree(lua_State* L);

	// _G
	static int luaIsType(lua_State* L);
	static int luaRawGetMetatable(lua_State* L);

	// os
	static int luaSystemTime(lua_State* L);

	// table
	static int luaTableCreate(lua_State* L);
	static int luaTablePack(lua_State* L);

	// DB Insert
	static int luaDBInsertCreate(lua_State* L);
	static int luaDBInsertAddRow(lua_State* L);
	static int luaDBInsertExecute(lua_State* L);
	static int luaDBInsertDelete(lua_State* L);

	// DB Transaction
	static int luaDBTransactionCreate(lua_State* L);
	static int luaDBTransactionDelete(lua_State* L);
	static int luaDBTransactionBegin(lua_State* L);
	static int luaDBTransactionCommit(lua_State* L);

	//
	std::string lastLuaError;
	std::string interfaceName;
	std::string loadingFile;
};

class LuaEnvironment : public LuaScriptInterface
{
public:
	LuaEnvironment();
	~LuaEnvironment();

	// non-copyable
	LuaEnvironment(const LuaEnvironment&) = delete;
	LuaEnvironment& operator=(const LuaEnvironment&) = delete;

	bool initState() override;
	bool reInitState();
	bool closeState() override;

	LuaScriptInterface* getTestInterface();

	Combat_ptr getCombatObject(uint32_t id) const;
	Combat_ptr createCombatObject(LuaScriptInterface* interface);
	void clearCombatObjects(LuaScriptInterface* interface);

	AreaCombat* getAreaObject(uint32_t id) const;
	uint32_t createAreaObject(LuaScriptInterface* interface);
	void clearAreaObjects(LuaScriptInterface* interface);

private:
	void executeTimerEvent(uint32_t eventIndex);

	std::unordered_map<uint32_t, LuaTimerEventDesc> timerEvents;
	std::unordered_map<uint32_t, Combat_ptr> combatMap;
	std::unordered_map<uint32_t, AreaCombat*> areaMap;

	std::unordered_map<LuaScriptInterface*, std::vector<uint32_t>> combatIdMap;
	std::unordered_map<LuaScriptInterface*, std::vector<uint32_t>> areaIdMap;

	LuaScriptInterface* testInterface = nullptr;

	uint32_t lastEventTimerId = 1;
	uint32_t lastCombatId = 0;
	uint32_t lastAreaId = 0;

	friend class LuaScriptInterface;
	friend class CombatSpell;
};
