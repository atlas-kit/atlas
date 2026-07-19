#pragma once

#include "../enums.h"
#include "error.h"
#include "variant.h"

class Creature;
class Cylinder;
class InstantSpell;
class NetworkMessage;
class Party;
class Player;
class Spell;
class Thing;
class ItemType;

struct LootBlock;
struct Town;

namespace tfs::lua {

enum LuaDataType
{
	LuaData_Unknown,

	LuaData_Item,
	LuaData_Container,
	LuaData_Teleport,
	LuaData_Podium,
	LuaData_Player,
	LuaData_Monster,
	LuaData_Npc,
	LuaData_Tile,
};

// Number helpers
template <typename T>
std::enable_if_t<std::is_enum_v<T> || std::is_integral_v<T> || std::is_floating_point_v<T>, T> getNumber(
    lua_State* L, int32_t arg, T defaultValue = {})
{
	int isnum;
	if (auto num = lua_tointegerx(L, arg, &isnum); isnum != 0) {
		return static_cast<T>(num);
	}
	if (auto num = lua_tonumberx(L, arg, &isnum); isnum != 0) {
		return static_cast<T>(num);
	}
	return defaultValue;
}

bool isNumber(lua_State* L, int32_t arg);

// Boolean helpers
bool getBoolean(lua_State* L, int32_t arg);
bool getBoolean(lua_State* L, int32_t arg, bool defaultValue);
void pushBoolean(lua_State* L, bool value);

// String helpers
std::string getString(lua_State* L, int32_t arg);
std::string popString(lua_State* L);
void pushString(lua_State* L, std::string_view value);

// Table field helpers
void setField(lua_State* L, const char* index, lua_Number value);
void setField(lua_State* L, const char* index, std::string_view value);

template <typename T>
T getField(lua_State* L, int32_t arg, std::string_view key)
{
	lua_getfield(L, arg, key.data());
	return getNumber<T>(L, -1);
}

template <typename T>
T getField(lua_State* L, int32_t arg, std::string_view key, T&& defaultValue)
{
	lua_getfield(L, arg, key.data());
	return getNumber<T>(L, -1, std::forward<T>(defaultValue));
}

std::string getFieldString(lua_State* L, int32_t arg, std::string_view key);

// Push number helpers
template <typename T>
std::enable_if_t<std::is_enum_v<T> || std::is_integral_v<T> || std::is_floating_point_v<T>, void> pushNumber(
    lua_State* L, T value)
{
	if constexpr (std::is_enum_v<T> || std::is_integral_v<T>) {
		lua_pushinteger(L, static_cast<lua_Integer>(value));
	} else {
		lua_pushnumber(L, static_cast<lua_Number>(value));
	}
}

// Userdata helpers
int luaUserdataCompare(lua_State* L);
LuaDataType getUserdataType(lua_State* L, int32_t arg);

template <class T>
void pushUserdata(lua_State* L, T* value)
{
	T** userdata = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
	*userdata = value;
}

template <class T>
T** getRawUserdata(lua_State* L, int32_t arg)
{
	return static_cast<T**>(lua_touserdata(L, arg));
}

template <class T>
T* getUserdata(lua_State* L, int32_t arg)
{
	T** userdata = getRawUserdata<T>(L, arg);
	return userdata ? *userdata : nullptr;
}

// SharedPtr helpers
template <class T>
std::shared_ptr<T>& getSharedPtr(lua_State* L, int32_t arg)
{
	return *static_cast<std::shared_ptr<T>*>(lua_touserdata(L, arg));
}

template <class T>
int luaSharedPtrDelete(lua_State* L)
{
	auto& ptr = getSharedPtr<T>(L, 1);
	std::destroy_at(std::addressof(ptr));
	return 0;
}

template <class T>
std::shared_ptr<T>* getRawSharedPtr(lua_State* L, int32_t arg)
{
	return static_cast<std::shared_ptr<T>*>(lua_touserdata(L, arg));
}

// Registry key for the weak-table cache that deduplicates shared_ptr userdata
extern const char* SHARED_PTR_CACHE_KEY;

// Pushes the weak cache table onto the stack, creating it if absent
void requireSharedPtrCache(lua_State* L);

// Push a shared_ptr as a single userdata, reusing any existing one via weak cache
// Keyed by raw pointer address, avoids use_count inflation from duplicates
template <class T>
void pushSharedPtr(lua_State* L, const std::shared_ptr<T>& value)
{
	if (!value) {
		lua_pushnil(L);
		return;
	}

	// ensure the weak cache table is on the stack
	requireSharedPtrCache(L);

	// use the raw address as the cache key
	using RawT = std::remove_const_t<T>;
	auto* key = const_cast<RawT*>(value.get());

	// look up the existing userdata
	lua_pushlightuserdata(L, key);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) {
		lua_remove(L, -2);
		return;
	}
	lua_pop(L, 1);

	// create a new userdata and copy the shared_ptr into it
	auto* ud = static_cast<std::shared_ptr<T>*>(lua_newuserdata(L, sizeof(std::shared_ptr<T>)));
	std::construct_at(ud, value);

	// store in cache so future pushes reuse it
	lua_pushlightuserdata(L, key);
	lua_pushvalue(L, -2);
	lua_rawset(L, -4);

	lua_remove(L, -2);
}

// High-level converters (Lua -> C++)
Position getPosition(lua_State* L, int32_t arg);
Position getPosition(lua_State* L, int32_t arg, int32_t& stackpos);

Outfit_t getOutfit(lua_State* L, int32_t arg);

LuaVariant getVariant(lua_State* L, int32_t arg);

std::shared_ptr<Thing> getThing(lua_State* L, int32_t arg);
std::shared_ptr<Creature> getCreature(lua_State* L, int32_t arg);
std::shared_ptr<Player> getPlayer(lua_State* L, int32_t arg);

// High-level push helpers (C++ -> Lua)
void pushPosition(lua_State* L, const Position& position, int32_t stackpos = 0);
void pushOutfit(lua_State* L, const Outfit_t& outfit);

void pushVariant(lua_State* L, const LuaVariant& var);
void pushThing(lua_State* L, const std::shared_ptr<Thing>& thing);
void pushSpell(lua_State* L, const Spell& spell);
void pushTown(lua_State* L, const Town& town);
void pushLoot(lua_State* L, const std::vector<LootBlock>& lootList);
void pushParty(lua_State* L, const std::shared_ptr<Party>& party);
void pushItemType(lua_State* L, const ItemType* itemType);
void pushNetworkMessage(lua_State* L, NetworkMessage* msg);

// Callback helpers
void pushCallback(lua_State* L, int32_t callback);
int32_t popCallback(lua_State* L);

} // namespace tfs::lua
