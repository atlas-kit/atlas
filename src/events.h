// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_EVENTS_H
#define FS_EVENTS_H

#include "otpch.h"

#include "const.h"
#include "creature.h"
#include "lua/script.h"
#include "networkmessage.h"

class ItemType;
class Party;
class Spell;
class Tile;

struct IEvent
{
	constexpr IEvent() = default;
	virtual ~IEvent() = default;

	IEvent(const IEvent&) = delete;
	IEvent& operator=(const IEvent&) = delete;
};

struct CreatureHealthChanged : public IEvent
{
	CreatureHealthChanged(std::shared_ptr<Creature> creature) : creature{std::move(creature)} {}

	const std::shared_ptr<Creature> creature;
};

struct CreatureHealed : public IEvent
{
	CreatureHealed(std::shared_ptr<Creature> victim, std::shared_ptr<Creature> healer, int32_t amount) :
	    victim{std::move(victim)}, healer{std::move(healer)}, amount{amount}
	{}

	const std::shared_ptr<Creature> victim;
	const std::shared_ptr<Creature> healer;
	const int32_t amount;
};

struct CreatureHealthDamaged : public IEvent
{
	CreatureHealthDamaged(std::shared_ptr<Creature> victim, std::shared_ptr<Creature> inflictor, int32_t amount) :
	    victim{std::move(victim)}, inflictor{std::move(inflictor)}, amount{amount}
	{}

	const std::shared_ptr<Creature> victim;
	const std::shared_ptr<Creature> inflictor;
	const int32_t amount;
};

struct PlayerManaChanged : public IEvent
{
	PlayerManaChanged(std::shared_ptr<Player> player) : player{std::move(player)} {}

	const std::shared_ptr<Player> player;
};

struct PlayerManaDrained : public IEvent
{
	PlayerManaDrained(std::shared_ptr<Player> victim, std::shared_ptr<Creature> inflictor, int32_t amount) :
	    victim{std::move(victim)}, inflictor{std::move(inflictor)}, amount{amount}
	{}

	const std::shared_ptr<Player> victim;
	const std::shared_ptr<Creature> inflictor;
	const int32_t amount;
};

enum class EventInfoId
{
	// Creature
	CREATURE_ONHEAR,

	// Monster
	MONSTER_ONSPAWN
};

namespace {

using Callback = std::move_only_function<void(const IEvent&)>;
std::unordered_map<std::type_index, std::vector<Callback>> callbacks;

} // namespace

namespace tfs::events {

template <typename T>
void subscribe(std::move_only_function<void(const T&)> callback)
{
	static_assert(std::is_base_of_v<IEvent, T>, "T must derive from IEvent");

	if (!callback) {
		throw std::invalid_argument("IEvent callback is empty");
	}

	const auto index = std::type_index(typeid(T));
	callbacks[index].push_back(
	    [callback = std::move(callback)](const IEvent& event) mutable { callback(static_cast<const T&>(event)); });
}

template <typename T, typename... Args>
void dispatch(Args&&... args)
{
	static_assert(std::is_base_of_v<IEvent, T>, "T must derive from IEvent");

	T event(std::forward<Args>(args)...);
	const auto index = std::type_index(typeid(T));
	const auto it = callbacks.find(index);
	if (it == callbacks.end()) {
		return;
	}

	for (auto& system : it->second) {
		system(event);
	}
}

void load();
void reload();
int32_t getScriptId(EventInfoId eventInfoId);

} // namespace tfs::events

namespace tfs::events::game {

void onStartup();
void onShutdown();
void onSave();

} // namespace tfs::events::game

namespace tfs::events::creature {

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
void onThink(const std::shared_ptr<Creature>& creature, uint32_t interval);
bool onPrepareDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& killer);
void onDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Item>& corpse,
             const std::shared_ptr<Creature>& killer, const std::shared_ptr<Creature>& mostDamageKiller,
             bool lastHitUnjustified, bool mostDamageUnjustified);
void onKill(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& target);

} // namespace tfs::events::creature

namespace tfs::events::party {

bool onJoin(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onLeave(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onDisband(const std::shared_ptr<Party>& party);
void onShareExperience(const std::shared_ptr<Party>& party, uint64_t& exp);
bool onInvite(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onRevokeInvitation(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onPassLeadership(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);

} // namespace tfs::events::party

namespace tfs::events::player {

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
bool onTurn(const std::shared_ptr<Player>& player, Direction direction);
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
void onNetworkMessage(const std::shared_ptr<Player>& player, uint8_t recvByte, NetworkMessage_ptr& msg);
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

namespace tfs::events::monster {

void onDropLoot(const std::shared_ptr<Monster>& monster, const std::shared_ptr<Container>& corpse);
bool onSpawn(const std::shared_ptr<Monster>& monster, const Position& position, bool startup, bool artificial);

} // namespace tfs::events::monster

#endif // FS_EVENTS_H
