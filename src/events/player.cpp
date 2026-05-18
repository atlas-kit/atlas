// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "../player.h"

#include "../item.h"
#include "../lua/env.h"
#include "../lua/error.h"
#include "../lua/script.h"
#include "events.h"
#include "player.h"

namespace {

struct PlayerHandlers
{
	int32_t onBrowseField = -1;
	int32_t onLook = -1;
	int32_t onLookInBattleList = -1;
	int32_t onLookInTrade = -1;
	int32_t onLookInShop = -1;
	int32_t onLookInMarket = -1;
	int32_t onMoveItem = -1;
	int32_t onItemMoved = -1;
	int32_t onMoveCreature = -1;
	int32_t onReportRuleViolation = -1;
	int32_t onRotateItem = -1;
	int32_t onTradeRequest = -1;
	int32_t onTradeAccept = -1;
	int32_t onTradeCompleted = -1;
	int32_t onPodiumRequest = -1;
	int32_t onPodiumEdit = -1;
	int32_t onGainExperience = -1;
	int32_t onLoseExperience = -1;
	int32_t onGainSkillTries = -1;
	int32_t onWrapItem = -1;
	int32_t onInventoryUpdate = -1;
	int32_t onNetworkMessage = -1;
	int32_t onSpellCheck = -1;
	int32_t onLogin = -1;
	int32_t onJoin = -1;
	int32_t onLogout = -1;
	int32_t onReconnect = -1;
	int32_t onAdvance = -1;
	int32_t onModalWindow = -1;
	int32_t onTextEdit = -1;
	int32_t onExtendedOpcode = -1;
} playerHandlers;

void loadPlayerScripts()
{
	playerHandlers = {};

	auto& scriptInterface = tfs::events::getScriptInterface();
	if (scriptInterface.loadFile("data/scripts/events/player.lua") != 0) {
		std::cout << "[Warning - tfs::events::player::loadPlayerScripts] Cannot load player events." << std::endl;
		std::cout << scriptInterface.getLastLuaError() << std::endl;
		return;
	}

	playerHandlers.onBrowseField = scriptInterface.getMetaEvent("Player", "onBrowseField");
	playerHandlers.onLook = scriptInterface.getMetaEvent("Player", "onLook");
	playerHandlers.onLookInBattleList = scriptInterface.getMetaEvent("Player", "onLookInBattleList");
	playerHandlers.onLookInTrade = scriptInterface.getMetaEvent("Player", "onLookInTrade");
	playerHandlers.onLookInShop = scriptInterface.getMetaEvent("Player", "onLookInShop");
	playerHandlers.onLookInMarket = scriptInterface.getMetaEvent("Player", "onLookInMarket");
	playerHandlers.onMoveItem = scriptInterface.getMetaEvent("Player", "onMoveItem");
	playerHandlers.onItemMoved = scriptInterface.getMetaEvent("Player", "onItemMoved");
	playerHandlers.onMoveCreature = scriptInterface.getMetaEvent("Player", "onMoveCreature");
	playerHandlers.onReportRuleViolation = scriptInterface.getMetaEvent("Player", "onReportRuleViolation");
	playerHandlers.onRotateItem = scriptInterface.getMetaEvent("Player", "onRotateItem");
	playerHandlers.onTradeRequest = scriptInterface.getMetaEvent("Player", "onTradeRequest");
	playerHandlers.onTradeAccept = scriptInterface.getMetaEvent("Player", "onTradeAccept");
	playerHandlers.onTradeCompleted = scriptInterface.getMetaEvent("Player", "onTradeCompleted");
	playerHandlers.onPodiumRequest = scriptInterface.getMetaEvent("Player", "onPodiumRequest");
	playerHandlers.onPodiumEdit = scriptInterface.getMetaEvent("Player", "onPodiumEdit");
	playerHandlers.onGainExperience = scriptInterface.getMetaEvent("Player", "onGainExperience");
	playerHandlers.onLoseExperience = scriptInterface.getMetaEvent("Player", "onLoseExperience");
	playerHandlers.onGainSkillTries = scriptInterface.getMetaEvent("Player", "onGainSkillTries");
	playerHandlers.onWrapItem = scriptInterface.getMetaEvent("Player", "onWrapItem");
	playerHandlers.onInventoryUpdate = scriptInterface.getMetaEvent("Player", "onInventoryUpdate");
	playerHandlers.onNetworkMessage = scriptInterface.getMetaEvent("Player", "onNetworkMessage");
	playerHandlers.onSpellCheck = scriptInterface.getMetaEvent("Player", "onSpellCheck");
	playerHandlers.onLogin = scriptInterface.getMetaEvent("Player", "onLogin");
	playerHandlers.onJoin = scriptInterface.getMetaEvent("Player", "onJoin");
	playerHandlers.onLogout = scriptInterface.getMetaEvent("Player", "onLogout");
	playerHandlers.onReconnect = scriptInterface.getMetaEvent("Player", "onReconnect");
	playerHandlers.onAdvance = scriptInterface.getMetaEvent("Player", "onAdvance");
	playerHandlers.onModalWindow = scriptInterface.getMetaEvent("Player", "onModalWindow");
	playerHandlers.onTextEdit = scriptInterface.getMetaEvent("Player", "onTextEdit");
	playerHandlers.onExtendedOpcode = scriptInterface.getMetaEvent("Player", "onExtendedOpcode");
}

} // namespace

namespace tfs::events::player {

void load()
{
	loadPlayerScripts();
}

void reload()
{
	loadPlayerScripts();
}

bool onBrowseField(const std::shared_ptr<Player>& player, const Position& position)
{
	// Player:onBrowseField(position)
	if (playerHandlers.onBrowseField == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onBrowseField] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onBrowseField, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onBrowseField);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushPosition(L, position);
	return tfs::events::getScriptInterface().callFunction(2);
}

void onLook(const std::shared_ptr<Player>& player, const Position& position, const std::shared_ptr<Thing>& thing,
            uint8_t stackpos, int32_t lookDistance)
{
	// Player:onLook(thing, position, distance)
	if (playerHandlers.onLook == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLook] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLook, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLook);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, thing);
	tfs::lua::pushPosition(L, position, stackpos);
	tfs::lua::pushNumber(L, lookDistance);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

void onLookInBattleList(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& creature,
                        int32_t lookDistance)
{
	// Player:onLookInBattleList(creature, distance)
	if (playerHandlers.onLookInBattleList == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLookInBattleList] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLookInBattleList, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLookInBattleList);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, creature);
	tfs::lua::pushNumber(L, lookDistance);
	tfs::events::getScriptInterface().callVoidFunction(3);
}

void onLookInTrade(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& partner,
                   const std::shared_ptr<Item>& item, int32_t lookDistance)
{
	// Player:onLookInTrade(partner, item, distance)
	if (playerHandlers.onLookInTrade == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLookInTrade] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLookInTrade, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLookInTrade);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, partner);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushNumber(L, lookDistance);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

void onLookInShop(const std::shared_ptr<Player>& player, const ItemType* itemType, uint8_t count)
{
	// Player:onLookInShop(itemType, count)
	if (playerHandlers.onLookInShop == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLookInShop] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLookInShop, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLookInShop);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushItemType(L, itemType);
	tfs::lua::pushNumber(L, count);
	tfs::events::getScriptInterface().callVoidFunction(3);
}

void onLookInMarket(const std::shared_ptr<Player>& player, const ItemType* itemType)
{
	// Player:onLookInMarket(itemType)
	if (playerHandlers.onLookInMarket == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLookInMarket] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLookInMarket, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLookInMarket);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushItemType(L, itemType);
	tfs::events::getScriptInterface().callVoidFunction(2);
}

ReturnValue onMoveItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, uint16_t count,
                       const Position& fromPosition, const Position& toPosition,
                       const std::shared_ptr<Thing>& fromThing, const std::shared_ptr<Thing>& toThing)
{
	// Player:onMoveItem(item, count, fromPosition, toPosition)
	if (playerHandlers.onMoveItem == -1) {
		return RETURNVALUE_NOERROR;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onMoveItem] Call stack overflow" << std::endl;
		return RETURNVALUE_NOTPOSSIBLE;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onMoveItem, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onMoveItem);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushNumber(L, count);
	tfs::lua::pushPosition(L, fromPosition);
	tfs::lua::pushPosition(L, toPosition);
	tfs::lua::pushThing(L, fromThing);
	tfs::lua::pushThing(L, toThing);

	ReturnValue returnValue;
	if (tfs::lua::protectedCall(L, 7, 1) != 0) {
		returnValue = RETURNVALUE_NOTPOSSIBLE;
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		returnValue = tfs::lua::getNumber<ReturnValue>(L, -1, RETURNVALUE_NOERROR);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
	return returnValue;
}

void onItemMoved(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, uint16_t count,
                 const Position& fromPosition, const Position& toPosition, const std::shared_ptr<Thing>& fromThing,
                 const std::shared_ptr<Thing>& toThing)
{
	// Player:onItemMoved(item, count, fromPosition, toPosition)
	if (playerHandlers.onItemMoved == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onItemMoved] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onItemMoved, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onItemMoved);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushNumber(L, count);
	tfs::lua::pushPosition(L, fromPosition);
	tfs::lua::pushPosition(L, toPosition);
	tfs::lua::pushThing(L, fromThing);
	tfs::lua::pushThing(L, toThing);
	tfs::events::getScriptInterface().callVoidFunction(7);
}

bool onMoveCreature(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& creature,
                    const Position& fromPosition, const Position& toPosition)
{
	// Player:onMoveCreature(creature, fromPosition, toPosition)
	if (playerHandlers.onMoveCreature == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onMoveCreature] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onMoveCreature, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onMoveCreature);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, creature);
	tfs::lua::pushPosition(L, fromPosition);
	tfs::lua::pushPosition(L, toPosition);
	return tfs::events::getScriptInterface().callFunction(4);
}

void onReportRuleViolation(const std::shared_ptr<Player>& player, const std::string& targetName, uint8_t reportType,
                           uint8_t reportReason, const std::string& comment, const std::string& translation)
{
	// Player:onReportRuleViolation(targetName, reportType, reportReason, comment, translation)
	if (playerHandlers.onReportRuleViolation == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onReportRuleViolation] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onReportRuleViolation, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onReportRuleViolation);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushString(L, targetName);
	tfs::lua::pushNumber(L, reportType);
	tfs::lua::pushNumber(L, reportReason);
	tfs::lua::pushString(L, comment);
	tfs::lua::pushString(L, translation);
	tfs::events::getScriptInterface().callVoidFunction(6);
}

void onRotateItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item)
{
	// Player:onRotateItem(item)
	if (playerHandlers.onRotateItem == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onRotateItem] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onRotateItem, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onRotateItem);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::events::getScriptInterface().callVoidFunction(2);
}

bool onTradeRequest(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                    const std::shared_ptr<Item>& item)
{
	// Player:onTradeRequest(target, item)
	if (playerHandlers.onTradeRequest == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onTradeRequest] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onTradeRequest, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onTradeRequest);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, target);
	tfs::lua::pushThing(L, item);
	return tfs::events::getScriptInterface().callFunction(3);
}

bool onTradeAccept(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                   const std::shared_ptr<Item>& item, const std::shared_ptr<Item>& targetItem)
{
	// Player:onTradeAccept(target, item, targetItem)
	if (playerHandlers.onTradeAccept == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onTradeAccept] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onTradeAccept, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onTradeAccept);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, target);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushThing(L, targetItem);
	return tfs::events::getScriptInterface().callFunction(4);
}

void onTradeCompleted(const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& target,
                      const std::shared_ptr<Item>& item, const std::shared_ptr<Item>& targetItem, bool isSuccess)
{
	// Player:onTradeCompleted(target, item, targetItem, isSuccess)
	if (playerHandlers.onTradeCompleted == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onTradeCompleted] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onTradeCompleted, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onTradeCompleted);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, target);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushThing(L, targetItem);
	tfs::lua::pushBoolean(L, isSuccess);
	tfs::events::getScriptInterface().callVoidFunction(5);
}

void onPodiumRequest(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item)
{
	// Player:onPodiumRequest(item)
	if (playerHandlers.onPodiumRequest == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onPodiumRequest] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onPodiumRequest, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onPodiumRequest);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::events::getScriptInterface().callVoidFunction(2);
}

void onPodiumEdit(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, const Outfit_t& outfit,
                  bool podiumVisible, Direction direction)
{
	// Player:onPodiumEdit(item, outfit, direction, isVisible)
	if (playerHandlers.onPodiumEdit == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onPodiumEdit] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onPodiumEdit, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onPodiumEdit);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushOutfit(L, outfit);
	tfs::lua::pushNumber(L, direction);
	tfs::lua::pushBoolean(L, podiumVisible);
	tfs::events::getScriptInterface().callVoidFunction(5);
}

void onGainExperience(const std::shared_ptr<Player>& player, const std::shared_ptr<Creature>& source, uint64_t& exp,
                      uint64_t rawExp, bool sendText)
{
	// Player:onGainExperience(source, exp, rawExp, sendText)
	if (playerHandlers.onGainExperience == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onGainExperience] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onGainExperience, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onGainExperience);

	tfs::lua::pushThing(L, player);

	if (source) {
		tfs::lua::pushThing(L, source);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushNumber(L, exp);
	tfs::lua::pushNumber(L, rawExp);
	tfs::lua::pushBoolean(L, sendText);

	if (tfs::lua::protectedCall(L, 5, 1) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		exp = tfs::lua::getNumber<uint64_t>(L, -1, exp);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
}

void onLoseExperience(const std::shared_ptr<Player>& player, uint64_t& exp)
{
	// Player:onLoseExperience(exp)
	if (playerHandlers.onLoseExperience == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLoseExperience] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLoseExperience, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLoseExperience);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushNumber(L, exp);

	if (tfs::lua::protectedCall(L, 2, 1) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		exp = tfs::lua::getNumber<uint64_t>(L, -1, exp);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
}

void onGainSkillTries(const std::shared_ptr<Player>& player, skills_t skill, uint64_t& tries)
{
	// Player:onGainSkillTries(skill, tries)
	if (playerHandlers.onGainSkillTries == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onGainSkillTries] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onGainSkillTries, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onGainSkillTries);

	tfs::lua::pushThing(L, player);

	tfs::lua::pushNumber(L, skill);
	tfs::lua::pushNumber(L, tries);

	if (tfs::lua::protectedCall(L, 3, 1) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		tries = tfs::lua::getNumber<uint64_t>(L, -1, tries);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
}

void onWrapItem(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item)
{
	// Player:onWrapItem(item)
	if (playerHandlers.onWrapItem == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onWrapItem] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onWrapItem, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onWrapItem);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::events::getScriptInterface().callVoidFunction(2);
}

void onInventoryUpdate(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, slots_t slot,
                       bool equip)
{
	// Player:onInventoryUpdate(item, slot, equip)
	if (playerHandlers.onInventoryUpdate == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onInventoryUpdate] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onInventoryUpdate, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onInventoryUpdate);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushNumber(L, slot);
	tfs::lua::pushBoolean(L, equip);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

void onNetworkMessage(const std::shared_ptr<Player>& player, uint8_t recvByte, std::unique_ptr<NetworkMessage> msg)
{
	// Player:onNetworkMessage(recvByte, msg)
	if (playerHandlers.onNetworkMessage == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onNetworkMessage] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onNetworkMessage, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onNetworkMessage);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushNumber(L, recvByte);
	tfs::lua::pushNetworkMessage(L, msg.release());
	tfs::events::getScriptInterface().callVoidFunction(3);
}

bool onSpellCheck(const std::shared_ptr<Player>& player, const Spell* spell)
{
	// Player:onSpellCheck(spell)
	if (playerHandlers.onSpellCheck == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onSpellCheck] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onSpellCheck, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onSpellCheck);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushSpell(L, *spell);
	return tfs::events::getScriptInterface().callFunction(2);
}

bool onLogin(const std::shared_ptr<Player>& player)
{
	// Player:onLogin()
	if (playerHandlers.onLogin == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLogin] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLogin, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLogin);

	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(1);
}

void onJoin(const std::shared_ptr<Player>& player)
{
	// Player:onJoin()
	if (playerHandlers.onJoin == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onJoin] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onJoin, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onJoin);

	tfs::lua::pushThing(L, player);
	tfs::events::getScriptInterface().callVoidFunction(1);
}

bool onLogout(const std::shared_ptr<Player>& player)
{
	// Player:onLogout()
	if (playerHandlers.onLogout == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onLogout] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onLogout, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onLogout);

	tfs::lua::pushThing(L, player);
	return tfs::events::getScriptInterface().callFunction(1);
}

void onReconnect(const std::shared_ptr<Player>& player)
{
	// Player:onReconnect()
	if (playerHandlers.onReconnect == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onReconnect] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onReconnect, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onReconnect);

	tfs::lua::pushThing(L, player);
	tfs::events::getScriptInterface().callVoidFunction(1);
}

void onAdvance(const std::shared_ptr<Player>& player, skills_t skill, uint32_t oldLevel, uint32_t newLevel)
{
	// Player:onAdvance(skill, oldLevel, newLevel)
	if (playerHandlers.onAdvance == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onAdvance] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onAdvance, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onAdvance);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushNumber(L, static_cast<uint32_t>(skill));
	tfs::lua::pushNumber(L, oldLevel);
	tfs::lua::pushNumber(L, newLevel);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

void onModalWindow(const std::shared_ptr<Player>& player, uint32_t modalWindowId, uint8_t buttonId, uint8_t choiceId)
{
	// Player:onModalWindow(modalWindowId, buttonId, choiceId)
	if (playerHandlers.onModalWindow == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onModalWindow] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onModalWindow, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onModalWindow);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushNumber(L, modalWindowId);
	tfs::lua::pushNumber(L, buttonId);
	tfs::lua::pushNumber(L, choiceId);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

bool onTextEdit(const std::shared_ptr<Player>& player, const std::shared_ptr<Item>& item, std::string_view text,
                const uint32_t windowTextId)
{
	// Player:onTextEdit(item, text, windowTextId)
	if (playerHandlers.onTextEdit == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onTextEdit] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onTextEdit, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onTextEdit);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushThing(L, item);
	tfs::lua::pushString(L, text);
	tfs::lua::pushNumber(L, windowTextId);
	return tfs::events::getScriptInterface().callFunction(4);
}

void onExtendedOpcode(const std::shared_ptr<Player>& player, uint8_t opcode, std::string_view buffer)
{
	// Player:onExtendedOpcode(opcode, buffer)
	if (playerHandlers.onExtendedOpcode == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::player::onExtendedOpcode] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(playerHandlers.onExtendedOpcode, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(playerHandlers.onExtendedOpcode);

	tfs::lua::pushThing(L, player);
	tfs::lua::pushNumber(L, opcode);
	tfs::lua::pushString(L, buffer);
	tfs::events::getScriptInterface().callVoidFunction(3);
}

} // namespace tfs::events::player
