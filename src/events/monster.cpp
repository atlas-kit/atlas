// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "../monster.h"

#include "../item.h"
#include "../lua/env.h"
#include "monster.h"

namespace {

LuaScriptInterface monsterScriptInterface{"Monster-Events Interface"};

struct MonsterHandlers
{
	int32_t onDropLoot = -1;
	int32_t onSpawn = -1;
} monsterHandlers;

void loadMonsterScripts()
{
	monsterHandlers = {};

	if (monsterScriptInterface.loadFile("data/scripts/events/monster.lua") != 0) {
		std::cout << "[Warning - tfs::events::monster::loadMonsterScripts] Cannot load monster events." << std::endl;
		std::cout << monsterScriptInterface.getLastLuaError() << std::endl;
		return;
	}

	monsterHandlers.onDropLoot = monsterScriptInterface.getMetaEvent("Monster", "onDropLoot");
	monsterHandlers.onSpawn = monsterScriptInterface.getMetaEvent("Monster", "onSpawn");
}

} // namespace

namespace tfs::events::monster {

void load()
{
	monsterScriptInterface.initState();

	loadMonsterScripts();
}

void reload()
{
	monsterScriptInterface.reInitState();

	loadMonsterScripts();
}

int32_t getOnSpawnScriptId() { return monsterHandlers.onSpawn; }

bool onSpawn(const std::shared_ptr<Monster>& monster, const Position& position, bool startup, bool artificial)
{
	// Monster:onSpawn(position, startup, artificial)
	if (monsterHandlers.onSpawn == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::monster::onSpawn] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(monsterHandlers.onSpawn, &monsterScriptInterface);

	const auto L = monsterScriptInterface.getLuaState();
	monsterScriptInterface.pushFunction(monsterHandlers.onSpawn);

	tfs::lua::pushThing(L, monster);
	tfs::lua::pushPosition(L, position);
	tfs::lua::pushBoolean(L, startup);
	tfs::lua::pushBoolean(L, artificial);
	return monsterScriptInterface.callFunction(4);
}

void onDropLoot(const std::shared_ptr<Monster>& monster, const std::shared_ptr<Container>& corpse)
{
	// Monster:onDropLoot(corpse)
	if (monsterHandlers.onDropLoot == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::monster::onDropLoot] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(monsterHandlers.onDropLoot, &monsterScriptInterface);

	const auto L = monsterScriptInterface.getLuaState();
	monsterScriptInterface.pushFunction(monsterHandlers.onDropLoot);

	tfs::lua::pushThing(L, monster);

	if (corpse) {
		tfs::lua::pushThing(L, corpse);
	} else {
		lua_pushnil(L);
	}

	monsterScriptInterface.callVoidFunction(2);
}

} // namespace tfs::events::monster
