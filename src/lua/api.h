#pragma once

#include "../outfit.h"
#include "error.h"
#include "variant.h"

class Creature;
class Cylinder;
class InstantSpell;
class Player;
class Thing;
class Spell;
struct LootBlock;
struct Mount;
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

bool isNumber(lua_State* L, int32_t arg);
int luaUserdataCompare(lua_State* L);

bool getBoolean(lua_State* L, int32_t arg);
bool getBoolean(lua_State* L, int32_t arg, bool defaultValue);

void setField(lua_State* L, const char* index, lua_Number value);
void setField(lua_State* L, const char* index, std::string_view value);

Outfit_t getOutfit(lua_State* L, int32_t arg);
Outfit getOutfitClass(lua_State* L, int32_t arg);
LuaVariant getVariant(lua_State* L, int32_t arg);
LuaDataType getUserdataType(lua_State* L, int32_t arg);
void pushLoot(lua_State* L, const std::vector<LootBlock>& lootList);
void pushTown(lua_State* L, const Town& town);

void pushThing(lua_State* L, const std::shared_ptr<Thing>& thing);
void pushVariant(lua_State* L, const LuaVariant& var);
void pushString(lua_State* L, std::string_view value);
void pushCallback(lua_State* L, int32_t callback);

std::string popString(lua_State* L);
int32_t popCallback(lua_State* L);

template <typename T>
std::enable_if_t<std::is_enum_v<T> || std::is_integral_v<T> || std::is_floating_point_v<T>, void> pushNumber(
    lua_State* L, T value)
{
	if constexpr (std::is_enum_v<T> || std::is_integral_v<T>) {
		lua_pushinteger(L, static_cast<lua_Integer>(value));
	} else if constexpr (std::is_floating_point_v<T>) {
		lua_pushnumber(L, static_cast<lua_Number>(value));
	}
}

// Userdata
template <class T>
void pushUserdata(lua_State* L, T* value)
{
	T** userdata = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
	*userdata = value;
}

template <class T>
std::shared_ptr<T>& getSharedPtr(lua_State* L, int32_t arg)
{
	return *static_cast<std::shared_ptr<T>*>(lua_touserdata(L, arg));
}

template <class T>
std::shared_ptr<T>* getRawSharedPtr(lua_State* L, int32_t arg)
{
	return static_cast<std::shared_ptr<T>*>(lua_touserdata(L, arg));
}
template <class T>
void pushSharedPtr(lua_State* L, std::shared_ptr<T> value)
{
	new (lua_newuserdata(L, sizeof(std::shared_ptr<T>))) std::shared_ptr<T>(std::move(value));
}

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

template <class T>
T** getRawUserdata(lua_State* L, int32_t arg)
{
	return static_cast<T**>(lua_touserdata(L, arg));
}

template <class T>
T* getUserdata(lua_State* L, int32_t arg)
{
	T** userdata = getRawUserdata<T>(L, arg);
	if (!userdata) {
		return nullptr;
	}
	return *userdata;
}

bool getBoolean(lua_State* L, int32_t arg);
bool getBoolean(lua_State* L, int32_t arg, bool defaultValue);

std::string getString(lua_State* L, int32_t arg);

Position getPosition(lua_State* L, int32_t arg);
Position getPosition(lua_State* L, int32_t arg, int32_t& stackpos);

std::shared_ptr<Thing> getThing(lua_State* L, int32_t arg);
std::shared_ptr<Creature> getCreature(lua_State* L, int32_t arg);
std::shared_ptr<Player> getPlayer(lua_State* L, int32_t arg);

template <typename T>
T getField(lua_State* L, int32_t arg, std::string_view key)
{
	lua_getfield(L, arg, key.data());
	return getNumber<T>(L, -1);
}

template <typename T, typename... Args>
T getField(lua_State* L, int32_t arg, std::string_view key, T&& defaultValue)
{
	lua_getfield(L, arg, key.data());
	return getNumber<T>(L, -1, std::forward<T>(defaultValue));
}

std::string getFieldString(lua_State* L, int32_t arg, std::string_view key);

void pushBoolean(lua_State* L, bool value);
void pushSpell(lua_State* L, const Spell& spell);
void pushPosition(lua_State* L, const Position& position, int32_t stackpos = 0);
void pushOutfit(lua_State* L, const Outfit_t& outfit);
void pushOutfit(lua_State* L, const Outfit* outfit);

} // namespace tfs::lua
