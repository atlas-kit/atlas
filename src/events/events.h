// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

class LuaScriptInterface;

enum class EventInfoId
{
	// Creature
	CREATURE_ONHEAR,

	// Monster
	MONSTER_ONSPAWN
};

namespace tfs::events {

LuaScriptInterface& getScriptInterface();

void load();
void reload();
int32_t getScriptId(EventInfoId eventInfoId);

} // namespace tfs::events
