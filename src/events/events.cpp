// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "events.h"

#include "creature.h"
#include "monster.h"

namespace tfs::events {

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

} // namespace tfs::events
