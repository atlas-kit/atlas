// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "groups.h"

#include "const.h"

namespace {

const std::unordered_map<std::string_view, PlayerFlags> ParsePlayerFlagMap = {
    {"cannotusecombat", PlayerFlag_CannotUseCombat},
    {"cannotattackplayer", PlayerFlag_CannotAttackPlayer},
    {"cannotattackmonster", PlayerFlag_CannotAttackMonster},
    {"cannotbeattacked", PlayerFlag_CannotBeAttacked},
    {"canconvinceall", PlayerFlag_CanConvinceAll},
    {"cansummonall", PlayerFlag_CanSummonAll},
    {"canillusionall", PlayerFlag_CanIllusionAll},
    {"cansenseinvisibility", PlayerFlag_CanSenseInvisibility},
    {"ignoredbymonsters", PlayerFlag_IgnoredByMonsters},
    {"notgaininfight", PlayerFlag_NotGainInFight},
    {"hasinfinitemana", PlayerFlag_HasInfiniteMana},
    {"hasinfinitesoul", PlayerFlag_HasInfiniteSoul},
    {"hasnoexhaustion", PlayerFlag_HasNoExhaustion},
    {"cannotusespells", PlayerFlag_CannotUseSpells},
    {"cannotpickupitem", PlayerFlag_CannotPickupItem},
    {"canalwayslogin", PlayerFlag_CanAlwaysLogin},
    {"canbroadcast", PlayerFlag_CanBroadcast},
    {"canedithouses", PlayerFlag_CanEditHouses},
    {"cannotbebanned", PlayerFlag_CannotBeBanned},
    {"cannotbepushed", PlayerFlag_CannotBePushed},
    {"hasinfinitecapacity", PlayerFlag_HasInfiniteCapacity},
    {"canpushallcreatures", PlayerFlag_CanPushAllCreatures},
    {"cantalkredprivate", PlayerFlag_CanTalkRedPrivate},
    {"cantalkredchannel", PlayerFlag_CanTalkRedChannel},
    {"talkorangehelpchannel", PlayerFlag_TalkOrangeHelpChannel},
    {"notgainexperience", PlayerFlag_NotGainExperience},
    {"notgainmana", PlayerFlag_NotGainMana},
    {"notgainhealth", PlayerFlag_NotGainHealth},
    {"notgainskill", PlayerFlag_NotGainSkill},
    {"setmaxspeed", PlayerFlag_SetMaxSpeed},
    {"specialvip", PlayerFlag_SpecialVIP},
    {"notgenerateloot", PlayerFlag_NotGenerateLoot},
    {"ignoreprotectionzone", PlayerFlag_IgnoreProtectionZone},
    {"ignorespellcheck", PlayerFlag_IgnoreSpellCheck},
    {"ignoreweaponcheck", PlayerFlag_IgnoreWeaponCheck},
    {"cannotbemuted", PlayerFlag_CannotBeMuted},
    {"isalwayspremium", PlayerFlag_IsAlwaysPremium},
    {"ignoreyellcheck", PlayerFlag_IgnoreYellCheck},
    {"ignoresendprivatecheck", PlayerFlag_IgnoreSendPrivateCheck}};

} // namespace

uint64_t Groups::getFlagFromName(std::string_view name)
{
	auto it = ParsePlayerFlagMap.find(name);
	if (it != ParsePlayerFlagMap.end()) {
		return it->second;
	}
	return 0;
}

Group& Groups::addGroup(Group group)
{
	for (Group& existing : groups) {
		if (existing.id == group.id) {
			// Update in place: keeps the element's address stable so Group*
			// pointers held by online players remain valid across reloads.
			existing = std::move(group);
			return existing;
		}
	}

	// std::deque keeps references to existing elements valid on push_back,
	// so adding a new group never invalidates online players' Group* pointers.
	return groups.emplace_back(std::move(group));
}

Group* Groups::getGroup(uint16_t id)
{
	for (Group& group : groups) {
		if (group.id == id) {
			return &group;
		}
	}
	return nullptr;
}
