// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "events.h"

#include "../lua/env.h"
#include "../lua/script.h"
#include "creature.h"
#include "game.h"
#include "monster.h"
#include "party.h"
#include "player.h"

namespace {

LuaScriptInterface scriptInterface{"Event Interface"};

} // namespace

namespace tfs::events {

LuaScriptInterface& getScriptInterface() { return scriptInterface; }

int32_t getScriptId(EventInfoId eventInfoId)
{
	switch (eventInfoId) {
		case EventInfoId::CREATURE_ONHEAR:
			return tfs::events::creature::getOnHearScriptId();
		case EventInfoId::MONSTER_ONSPAWN:
			return tfs::events::monster::getOnSpawnScriptId();
		default:
			return -1;
	}
}

void load()
{
	scriptInterface.initState();

	tfs::events::game::load();
	tfs::events::creature::load();
	tfs::events::party::load();
	tfs::events::player::load();
	tfs::events::monster::load();
}

void reload()
{
	scriptInterface.reInitState();

	tfs::events::game::reload();
	tfs::events::creature::reload();
	tfs::events::party::reload();
	tfs::events::player::reload();
	tfs::events::monster::reload();
}

} // namespace tfs::events
