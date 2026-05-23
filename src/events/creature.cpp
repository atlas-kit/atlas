// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "creature.h"

#include "../lua/env.h"
#include "../lua/error.h"
#include "../lua/script.h"
#include "events.h"

namespace {

struct CreatureHandlers
{
	int32_t onChangeOutfit = -1;
	int32_t onAreaCombat = -1;
	int32_t onTargetCombat = -1;
	int32_t onHear = -1;
	int32_t onChangeZone = -1;
	int32_t onUpdateStorage = -1;
	int32_t onChangeHealth = -1;
	int32_t onChangeMana = -1;
	int32_t onThink = -1;
	int32_t onPrepareDeath = -1;
	int32_t onDeath = -1;
	int32_t onKill = -1;
} creatureHandlers;

void loadCreatureScripts()
{
	creatureHandlers = {};

	auto& scriptInterface = tfs::events::getScriptInterface();
	if (scriptInterface.loadFile("data/scripts/events/creature.lua") != 0) {
		std::cout << "[Warning - tfs::events::creature::loadCreatureScripts] Cannot load creature events." << std::endl;
		std::cout << scriptInterface.getLastLuaError() << std::endl;
		return;
	}

	creatureHandlers.onChangeOutfit = scriptInterface.getMetaEvent("Creature", "onChangeOutfit");
	creatureHandlers.onAreaCombat = scriptInterface.getMetaEvent("Creature", "onAreaCombat");
	creatureHandlers.onTargetCombat = scriptInterface.getMetaEvent("Creature", "onTargetCombat");
	creatureHandlers.onHear = scriptInterface.getMetaEvent("Creature", "onHear");
	creatureHandlers.onChangeZone = scriptInterface.getMetaEvent("Creature", "onChangeZone");
	creatureHandlers.onUpdateStorage = scriptInterface.getMetaEvent("Creature", "onUpdateStorage");
	creatureHandlers.onChangeHealth = scriptInterface.getMetaEvent("Creature", "onChangeHealth");
	creatureHandlers.onChangeMana = scriptInterface.getMetaEvent("Creature", "onChangeMana");
	creatureHandlers.onThink = scriptInterface.getMetaEvent("Creature", "onThink");
	creatureHandlers.onPrepareDeath = scriptInterface.getMetaEvent("Creature", "onPrepareDeath");
	creatureHandlers.onDeath = scriptInterface.getMetaEvent("Creature", "onDeath");
	creatureHandlers.onKill = scriptInterface.getMetaEvent("Creature", "onKill");
}

} // namespace

namespace tfs::events::creature {

void load()
{
	loadCreatureScripts();
}

void reload()
{
	loadCreatureScripts();
}

int32_t getOnHearScriptId() { return creatureHandlers.onHear; }

bool onChangeOutfit(const std::shared_ptr<Creature>& creature, const Outfit_t& outfit)
{
	// Creature:onChangeOutfit(outfit)
	if (creatureHandlers.onChangeOutfit == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onChangeOutfit] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onChangeOutfit, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onChangeOutfit);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushOutfit(L, outfit);
	return tfs::events::getScriptInterface().callFunction(2);
}

ReturnValue onAreaCombat(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Tile>& tile, bool aggressive)
{
	// Creature:onAreaCombat(tile, aggressive)
	if (creatureHandlers.onAreaCombat == -1) {
		return RETURNVALUE_NOERROR;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onAreaCombat] Call stack overflow" << std::endl;
		return RETURNVALUE_NOTPOSSIBLE;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onAreaCombat, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onAreaCombat);

	if (creature) {
		tfs::lua::pushThing(L, creature);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushThing(L, tile);
	tfs::lua::pushBoolean(L, aggressive);

	ReturnValue returnValue;
	if (tfs::lua::protectedCall(L, 3, 1) != 0) {
		returnValue = RETURNVALUE_NOTPOSSIBLE;
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		returnValue = tfs::lua::getNumber<ReturnValue>(L, -1);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
	return returnValue;
}

ReturnValue onTargetCombat(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& target)
{
	// Creature:onTargetCombat(target)
	if (creatureHandlers.onTargetCombat == -1) {
		return RETURNVALUE_NOERROR;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onTargetCombat] Call stack overflow" << std::endl;
		return RETURNVALUE_NOTPOSSIBLE;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onTargetCombat, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onTargetCombat);

	if (creature) {
		tfs::lua::pushThing(L, creature);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushThing(L, target);

	ReturnValue returnValue;
	if (tfs::lua::protectedCall(L, 2, 1) != 0) {
		returnValue = RETURNVALUE_NOTPOSSIBLE;
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		returnValue = tfs::lua::getNumber<ReturnValue>(L, -1, RETURNVALUE_NOERROR);
		lua_pop(L, 1);
	}

	tfs::lua::resetScriptEnv();
	return returnValue;
}

void onHear(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& speaker,
            const std::string& words, SpeakClasses type)
{
	// Creature:onHear(speaker, words, type)
	if (creatureHandlers.onHear == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onHear] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onHear, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onHear);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushThing(L, speaker);
	tfs::lua::pushString(L, words);
	tfs::lua::pushNumber(L, type);
	tfs::events::getScriptInterface().callVoidFunction(4);
}

void onChangeZone(const std::shared_ptr<Creature>& creature, ZoneType_t fromZone, ZoneType_t toZone)
{
	// Creature:onChangeZone(fromZone, toZone)
	if (creatureHandlers.onChangeZone == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onChangeZone] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onChangeZone, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onChangeZone);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushNumber(L, fromZone);
	tfs::lua::pushNumber(L, toZone);
	tfs::events::getScriptInterface().callVoidFunction(3);
}

void onUpdateStorage(const std::shared_ptr<Creature>& creature, uint32_t key, std::optional<int32_t> value,
                     std::optional<int32_t> oldValue, bool isSpawn)
{
	// Creature:onUpdateStorage(key, value, oldValue, isSpawn)
	if (creatureHandlers.onUpdateStorage == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onUpdateStorage] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onUpdateStorage, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onUpdateStorage);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushNumber(L, key);

	if (value) {
		tfs::lua::pushNumber(L, value.value());
	} else {
		lua_pushnil(L);
	}

	if (oldValue) {
		tfs::lua::pushNumber(L, oldValue.value());
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushBoolean(L, isSpawn);
	tfs::events::getScriptInterface().callVoidFunction(5);
}

void onChangeHealth(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& attacker,
                    CombatDamage& damage)
{
	// Creature:onChangeHealth(attacker, primaryDamage, primaryType, secondaryDamage, secondaryType, origin)
	if (creatureHandlers.onChangeHealth == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onChangeHealth] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onChangeHealth, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onChangeHealth);

	tfs::lua::pushThing(L, creature);

	if (attacker) {
		tfs::lua::pushThing(L, attacker);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushNumber(L, damage.primary.value);
	tfs::lua::pushNumber(L, damage.primary.type);
	tfs::lua::pushNumber(L, damage.secondary.value);
	tfs::lua::pushNumber(L, damage.secondary.type);
	tfs::lua::pushNumber(L, damage.origin);

	if (tfs::lua::protectedCall(L, 7, 4) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		damage.primary.value = std::abs(tfs::lua::getNumber<int32_t>(L, -4, damage.primary.value));
		damage.primary.type = tfs::lua::getNumber<CombatType_t>(L, -3, damage.primary.type);
		damage.secondary.value = std::abs(tfs::lua::getNumber<int32_t>(L, -2, damage.secondary.value));
		damage.secondary.type = tfs::lua::getNumber<CombatType_t>(L, -1, damage.secondary.type);
		lua_pop(L, 4);

		if (damage.primary.type != COMBAT_HEALING && damage.primary.type != COMBAT_NONE) {
			damage.primary.value = -damage.primary.value;
		}
		if (damage.secondary.type != COMBAT_HEALING && damage.secondary.type != COMBAT_NONE) {
			damage.secondary.value = -damage.secondary.value;
		}
	}

	tfs::lua::resetScriptEnv();
}

void onChangeMana(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& attacker,
                  CombatDamage& damage)
{
	// Creature:onChangeMana(attacker, primaryDamage, primaryType, secondaryDamage, secondaryType, origin)
	if (creatureHandlers.onChangeMana == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onChangeMana] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onChangeMana, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onChangeMana);

	tfs::lua::pushThing(L, creature);

	if (attacker) {
		tfs::lua::pushThing(L, attacker);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushNumber(L, damage.primary.value);
	tfs::lua::pushNumber(L, damage.primary.type);
	tfs::lua::pushNumber(L, damage.secondary.value);
	tfs::lua::pushNumber(L, damage.secondary.type);
	tfs::lua::pushNumber(L, damage.origin);

	if (tfs::lua::protectedCall(L, 7, 4) != 0) {
		tfs::lua::reportError(L, tfs::lua::popString(L));
	} else {
		damage.primary.value = tfs::lua::getNumber<int32_t>(L, -4, damage.primary.value);
		damage.primary.type = tfs::lua::getNumber<CombatType_t>(L, -3, damage.primary.type);
		damage.secondary.value = tfs::lua::getNumber<int32_t>(L, -2, damage.secondary.value);
		damage.secondary.type = tfs::lua::getNumber<CombatType_t>(L, -1, damage.secondary.type);
		lua_pop(L, 4);
	}

	tfs::lua::resetScriptEnv();
}

void onThink(const std::shared_ptr<Creature>& creature, std::chrono::milliseconds interval)
{
	// Creature:onThink(interval)
	if (creatureHandlers.onThink == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onThink] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onThink, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onThink);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushNumber(L, interval.count());
	tfs::events::getScriptInterface().callVoidFunction(2);
}

bool onPrepareDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& killer)
{
	// Creature:onPrepareDeath(killer)
	if (creatureHandlers.onPrepareDeath == -1) {
		return true;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onPrepareDeath] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onPrepareDeath, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onPrepareDeath);

	tfs::lua::pushThing(L, creature);

	if (killer) {
		tfs::lua::pushThing(L, killer);
	} else {
		lua_pushnil(L);
	}

	return tfs::events::getScriptInterface().callFunction(2);
}

void onDeath(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Item>& corpse,
             const std::shared_ptr<Creature>& killer, const std::shared_ptr<Creature>& mostDamageKiller,
             bool lastHitUnjustified, bool mostDamageUnjustified)
{
	// Creature:onDeath(corpse, killer, mostDamageKiller, lastHitUnjustified, mostDamageUnjustified)
	if (creatureHandlers.onDeath == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onDeath] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onDeath, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onDeath);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushThing(L, corpse);

	if (killer) {
		tfs::lua::pushThing(L, killer);
	} else {
		lua_pushnil(L);
	}

	if (mostDamageKiller) {
		tfs::lua::pushThing(L, mostDamageKiller);
	} else {
		lua_pushnil(L);
	}

	tfs::lua::pushBoolean(L, lastHitUnjustified);
	tfs::lua::pushBoolean(L, mostDamageUnjustified);
	tfs::events::getScriptInterface().callVoidFunction(6);
}

void onKill(const std::shared_ptr<Creature>& creature, const std::shared_ptr<Creature>& target)
{
	// Creature:onKill(target)
	if (creatureHandlers.onKill == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::creature::onKill] Call stack overflow" << std::endl;
		return;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(creatureHandlers.onKill, &tfs::events::getScriptInterface());

	const auto L = tfs::events::getScriptInterface().getLuaState();
	tfs::events::getScriptInterface().pushFunction(creatureHandlers.onKill);

	tfs::lua::pushThing(L, creature);
	tfs::lua::pushThing(L, target);
	tfs::events::getScriptInterface().callVoidFunction(2);
}

} // namespace tfs::events::creature
