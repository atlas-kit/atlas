// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

#include "../const.h"
#include "../creature.h"

namespace tfs::events::creature {

void load();
void reload();
int32_t getOnHearScriptId();

bool onChangeOutfit(const std::shared_ptr<Creature>& creature, const Outfit_t& outfit);
ReturnValue onAreaCombat(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Tile>& tile, bool aggressive);
ReturnValue onTargetCombat(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& target);
void onHear(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& speaker,
            const std::string& words, SpeakClasses type);
void onChangeZone(const std::shared_ptr<Creature>& creature, ZoneType_t fromZone, ZoneType_t toZone);
void onUpdateStorage(const std::shared_ptr<Creature>& creature, uint32_t key, std::optional<int32_t> value,
                     std::optional<int32_t> oldValue, bool isSpawn);
void onChangeHealth(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& attacker,
                    CombatDamage& damage);
void onChangeMana(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& attacker,
                  CombatDamage& damage);
void onThink(const std::shared_ptr<Creature>& creature, std::chrono::milliseconds interval);
bool onPrepareDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& killer);
void onDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Item>& corpse,
             const std::shared_ptr<Creature>& killer, const std::shared_ptr<Creature>& mostDamageKiller,
             bool lastHitUnjustified, bool mostDamageUnjustified);
void onKill(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& target);

} // namespace tfs::events::creature
