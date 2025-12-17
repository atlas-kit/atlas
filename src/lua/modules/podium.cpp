#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

	int luaPodiumCreate(lua_State* L)
{
	// Podium(uid)
	uint32_t id = tfs::lua::getNumber<uint32_t>(L, 2);

	const auto& item = tfs::lua::getScriptEnv()->getItemByUID(id);
	if (item && item->getPodium()) {
		tfs::lua::pushSharedPtr(L, item);
		tfs::lua::setMetatable(L, -1, "Podium");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumGetOutfit(lua_State* L)
{
	// podium:getOutfit()
	if (const auto& podium = tfs::lua::getSharedPtr<const Podium>(L, 1)) {
		tfs::lua::pushOutfit(L, podium->getOutfit());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumSetOutfit(lua_State* L)
{
	// podium:setOutfit(outfit)
	if (const auto& podium = tfs::lua::getSharedPtr<Podium>(L, 1)) {
		podium->setOutfit(tfs::lua::getOutfit(L, 2));
		g_game.updatePodium(podium);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumHasFlag(lua_State* L)
{
	// podium:hasFlag(flag)
	if (const auto& podium = tfs::lua::getSharedPtr<Podium>(L, 1)) {
		PodiumFlags flag = tfs::lua::getNumber<PodiumFlags>(L, 2);
		tfs::lua::pushBoolean(L, podium->hasFlag(flag));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumSetFlag(lua_State* L)
{
	// podium:setFlag(flag, value)
	uint8_t value = tfs::lua::getBoolean(L, 3);
	PodiumFlags flag = tfs::lua::getNumber<PodiumFlags>(L, 2);
	const auto& podium = tfs::lua::getSharedPtr<Podium>(L, 1);

	if (podium) {
		podium->setFlagValue(flag, value);
		g_game.updatePodium(podium);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumGetDirection(lua_State* L)
{
	// podium:getDirection()
	if (const auto& podium = tfs::lua::getSharedPtr<const Podium>(L, 1)) {
		tfs::lua::pushNumber(L, podium->getDirection());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaPodiumSetDirection(lua_State* L)
{
	// podium:setDirection(direction)
	if (const auto& podium = tfs::lua::getSharedPtr<Podium>(L, 1)) {
		podium->setDirection(tfs::lua::getNumber<Direction>(L, 2));
		g_game.updatePodium(podium);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerPodium(LuaScriptInterface& i)
{
	i.registerClass("Podium", "Item", luaPodiumCreate);
	i.registerMetaMethod("Podium", "__eq", tfs::lua::luaUserdataCompare);

	i.registerMethod("Podium", "getOutfit", luaPodiumGetOutfit);
	i.registerMethod("Podium", "setOutfit", luaPodiumSetOutfit);
	i.registerMethod("Podium", "hasFlag", luaPodiumHasFlag);
	i.registerMethod("Podium", "setFlag", luaPodiumSetFlag);
	i.registerMethod("Podium", "getDirection", luaPodiumGetDirection);
	i.registerMethod("Podium", "setDirection", luaPodiumSetDirection);
}
