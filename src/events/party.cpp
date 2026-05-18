// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "party.h"

#include "../lua/env.h"
#include "../lua/error.h"
#include "../lua/script.h"
#include "../player.h"
#include "events.h"

namespace {

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

	auto& scriptInterface = tfs::events::getScriptInterface();
	if (scriptInterface.loadFile("data/scripts/events/party.lua") != 0) {
		std::cout << "[Warning - tfs::events::party::loadPartyScripts] Cannot load party events." << std::endl;
		std::cout << scriptInterface.getLastLuaError() << std::endl;
		return;
	}

	partyHandlers.onJoin = scriptInterface.getMetaEvent("Party", "onJoin");
	partyHandlers.onLeave = scriptInterface.getMetaEvent("Party", "onLeave");
	partyHandlers.onDisband = scriptInterface.getMetaEvent("Party", "onDisband");
	partyHandlers.onShareExperience = scriptInterface.getMetaEvent("Party", "onShareExperience");
	partyHandlers.onInvite = scriptInterface.getMetaEvent("Party", "onInvite");
	partyHandlers.onRevokeInvitation = scriptInterface.getMetaEvent("Party", "onRevokeInvitation");
	partyHandlers.onPassLeadership = scriptInterface.getMetaEvent("Party", "onPassLeadership");
}

} // namespace

namespace tfs::events::party {

void load()
{
	loadPartyScripts();
}

void reload()
{
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
	env->setScriptId(partyHandlers.onJoin, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onJoin);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(2);
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
	env->setScriptId(partyHandlers.onLeave, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onLeave);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(2);
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
	env->setScriptId(partyHandlers.onDisband, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onDisband);

	tfs::lua::pushParty(L, party);
	return tfs::events::getScriptInterface().callFunction(1);
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
	env->setScriptId(partyHandlers.onInvite, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onInvite);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(2);
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
	env->setScriptId(partyHandlers.onRevokeInvitation, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onRevokeInvitation);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(2);
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
	env->setScriptId(partyHandlers.onPassLeadership, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onPassLeadership);

	tfs::lua::pushParty(L, party);
	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(2);
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
	env->setScriptId(partyHandlers.onShareExperience, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(partyHandlers.onShareExperience);

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
