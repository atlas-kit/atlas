// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "game.h"

#include "../lua/env.h"
#include "../lua/script.h"
#include "events.h"

namespace {

struct GameHandlers
{
	int32_t onStartup = -1;
	int32_t onShutdown = -1;
	int32_t onSave = -1;
} gameHandlers;

void loadGameScripts()
{
	gameHandlers = {};

	auto& scriptInterface = tfs::events::getScriptInterface();
	if (scriptInterface.loadFile("data/scripts/events/game.lua") != 0) {
		std::cout << "[Warning - tfs::events::game::loadGameScripts] Cannot load game events." << std::endl;
		std::cout << scriptInterface.getLastLuaError() << std::endl;
		return;
	}

	gameHandlers.onStartup = scriptInterface.getMetaEvent("Game", "onStartup");
	gameHandlers.onShutdown = scriptInterface.getMetaEvent("Game", "onShutdown");
	gameHandlers.onSave = scriptInterface.getMetaEvent("Game", "onSave");
}

} // namespace

namespace tfs::events::game {

void load()
{
	loadGameScripts();
}

void reload()
{
	loadGameScripts();
}

void onStartup()
{
	// Game:onStartup()
	if (gameHandlers.onStartup == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::game::onStartup] Call stack overflow" << std::endl;
		return;
	}

	auto& scriptInterface = tfs::events::getScriptInterface();
	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onStartup, &scriptInterface);

	scriptInterface.pushFunction(gameHandlers.onStartup);
	scriptInterface.callVoidFunction(0);
}

void onShutdown()
{
	// Game:onShutdown()
	if (gameHandlers.onShutdown == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::game::onShutdown] Call stack overflow" << std::endl;
		return;
	}

	auto& scriptInterface = tfs::events::getScriptInterface();
	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onShutdown, &scriptInterface);

	scriptInterface.pushFunction(gameHandlers.onShutdown);
	scriptInterface.callVoidFunction(0);
}

void onSave()
{
	// Game:onSave()
	if (gameHandlers.onSave == -1) {
		return;
	}

	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - tfs::events::game::onSave] Call stack overflow" << std::endl;
		return;
	}

	auto& scriptInterface = tfs::events::getScriptInterface();
	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onSave, &scriptInterface);

	scriptInterface.pushFunction(gameHandlers.onSave);
	scriptInterface.callVoidFunction(0);
}

} // namespace tfs::events::game
