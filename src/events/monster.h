// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

class Container;
class Monster;
struct Position;

namespace tfs::events::monster {

void load();
void reload();
int32_t getOnSpawnScriptId();

void onDropLoot(const std::shared_ptr<Monster>& monster, const std::shared_ptr<Container>& corpse);
bool onSpawn(const std::shared_ptr<Monster>& monster, const Position& position, bool startup, bool artificial);

} // namespace tfs::events::monster
