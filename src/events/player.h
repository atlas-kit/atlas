// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

#include "../creature.h"
#include "../networkmessage.h"

class ItemType;
class Spell;
class Tile;

namespace tfs::events::player {

void load();
void reload();

bool onBrowseField(const std::shared_ptr<Player>& player, const Position& position);
void onLook(const std::shared_ptr<Player>& player, const Position& position, const std::shared_ptr<Thing>& thing,
            uint8_t stackpos, int32_t lookDistance);
void onLookInBattleList(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& creature,
                        int32_t lookDistance);
void onLookInTrade(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& partner,
                   const std::shared_ptr<Item>& item, int32_t lookDistance);
void onLookInShop(const std::shared_ptr<Player>& player, const ItemType* itemType, uint8_t count);
void onLookInMarket(const std::shared_ptr<Player>& player, const ItemType* itemType);
ReturnValue onMoveItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, uint16_t count,
                       const Position& fromPosition, const Position& toPosition,
                       const std::shared_ptr<Thing>& fromThing, const std::shared_ptr<Thing>& toThing);
void onItemMoved(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, uint16_t count,
                 const Position& fromPosition, const Position& toPosition, const std::shared_ptr<Thing>& fromThing,
                 const std::shared_ptr<Thing>& toThing);
bool onMoveCreature(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& creature,
                    const Position& fromPosition, const Position& toPosition);
void onReportRuleViolation(const std::shared_ptr<Player>& player, const std::string& targetName, uint8_t reportType,
                           uint8_t reportReason, const std::string& comment, const std::string& translation);
void onRotateItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item);
bool onTradeRequest(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                    const std::shared_ptr<Item>& item);
bool onTradeAccept(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                   const std::shared_ptr<Item>& item, const std::shared_ptr<Item>& targetItem);
void onTradeCompleted(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                      const std::shared_ptr<Item>& item, const std::shared_ptr<Item>& targetItem, bool isSuccess);
void onPodiumRequest(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item);
void onPodiumEdit(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, const Outfit_t& outfit,
                  bool podiumVisible, Direction direction);
void onGainExperience(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& source, uint64_t& exp,
                      uint64_t rawExp, bool sendText);
void onLoseExperience(const std::shared_ptr<Player>& player, uint64_t& exp);
void onGainSkillTries(const std::shared_ptr<Player>& player, skills_t skill, uint64_t& tries);
void onWrapItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item);
void onInventoryUpdate(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, slots_t slot,
                       bool equip);
void onNetworkMessage(const std::shared_ptr<Player>& player, uint8_t recvByte, std::unique_ptr<NetworkMessage> msg);
bool onSpellCheck(const std::shared_ptr<Player>& player, const Spell* spell);
bool onLogin(const std::shared_ptr<Player>& player);
void onJoin(const std::shared_ptr<Player>& player);
bool onLogout(const std::shared_ptr<Player>& player);
void onReconnect(const std::shared_ptr<Player>& player);
void onAdvance(const std::shared_ptr<Player>& player, skills_t skill, uint32_t oldLevel, uint32_t newLevel);
void onModalWindow(const std::shared_ptr<Player>& player, uint32_t modalWindowId, uint8_t buttonId, uint8_t choiceId);
bool onTextEdit(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, std::string_view text,
                const uint32_t windowTextId);
void onExtendedOpcode(const std::shared_ptr<Player>& player, uint8_t opcode, std::string_view buffer);

} // namespace tfs::events::player
