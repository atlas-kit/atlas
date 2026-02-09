// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "party.h"

#include "../item.h"
#include "../lua/env.h"
#include "../lua/error.h"
#include "../player.h"

namespace {

LuaScriptInterface partyScriptInterface{"Party-Events Interface"};

struct PartyHandlers
{
	int32_t onJoin = -1;
	int32_t onLeave = -1;
	int32_t onDisband = -1;
	int32_t onShareExperience = -1;
	int32_t onInvite = -1;
	int32_t onRevokeInvitation = -1;
	int32_t onPassLeadership = -1;
} partyHandlers;

void loadPartyScripts()
{
	partyHandlers = {};

	if (partyScriptInterface.loadFile("data/scripts/events/party.lua") != 0) {
		std::cout << "[Warning - tfs::events::party::loadPartyScripts] Cannot load party events." << std::endl;
		std::cout << partyScriptInterface.getLastLuaError() << std::endl;
		return;
	}

	partyHandlers.onJoin = partyScriptInterface.getMetaEvent("Party", "onJoin");
	partyHandlers.onLeave = partyScriptInterface.getMetaEvent("Party", "onLeave");
	partyHandlers.onDisband = partyScriptInterface.getMetaEvent("Party", "onDisband");
	partyHandlers.onShareExperience = partyScriptInterface.getMetaEvent("Party", "onShareExperience");
	partyHandlers.onInvite = partyScriptInterface.getMetaEvent("Party", "onInvite");
	partyHandlers.onRevokeInvitation = partyScriptInterface.getMetaEvent("Party", "onRevokeInvitation");
	partyHandlers.onPassLeadership = partyScriptInterface.getMetaEvent("Party", "onPassLeadership");
}

} // namespace

namespace tfs::events::party {

void load()
{
	partyScriptInterface.initState();

	loadPartyScripts();
}

void reload()
{
	partyScriptInterface.reInitState();

	loadPartyScripts();
}

bool onJoin(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player)
{
	// Party:onJoin(player)
	if (partyHandlers.onJoin == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onJoin] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onJoin, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onJoin);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return partyScriptInterface.callFunction(2);
}

bool onLeave(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player)
{
	// Party:onLeave(player)
	if (partyHandlers.onLeave == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onLeave] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onLeave, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onLeave);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return partyScriptInterface.callFunction(2);
}

bool onDisband(const std::shared_ptr<Party>& party)
{
	// Party:onDisband()
	if (partyHandlers.onDisband == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onDisband] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onDisband, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onDisband);

	tfs::lua::pushParty(L, party);
	return partyScriptInterface.callFunction(1);
}

bool onInvite(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player)
{
	// Party:onInvite(player)
	if (partyHandlers.onInvite == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onInvite] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onInvite, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onInvite);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return partyScriptInterface.callFunction(2);
}

bool onRevokeInvitation(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player)
{
	// Party:onRevokeInvitation(player)
	if (partyHandlers.onRevokeInvitation == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onRevokeInvitation] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onRevokeInvitation, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onRevokeInvitation);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return partyScriptInterface.callFunction(2);
}

bool onPassLeadership(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player)
{
	// Party:onPassLeadership(player)
	if (partyHandlers.onPassLeadership == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onPassLeadership] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onPassLeadership, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onPassLeadership);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return partyScriptInterface.callFunction(2);
}

void onShareExperience(const std::shared_ptr<Party>& party, uint64_t& exp)
{
	// Party:onShareExperience(exp)
	if (partyHandlers.onShareExperience == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::party::onShareExperience] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(partyHandlers.onShareExperience, &partyScriptInterface);

	const auto L = partyScriptInterface.getLuaState();
	partyScriptInterface.pushFunction(partyHandlers.onShareExperience);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushNumber(L, exp);

	if (tfs::lua::protectedCall(L, 2, 1) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		exp = tfs::lua::getNumber<uint64_t>(L, -1, exp);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
}

} // namespace tfs::events::party
