// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "game.h"

#include "../item.h"
#include "../lua/env.h"

namespace {

LuaScriptInterface gameScriptInterface{"Game-Events Interface"};

struct GameHandlers
{
	int32_t onStartup = -1;
	int32_t onShutdown = -1;
	int32_t onSave = -1;
} gameHandlers;

void loadGameScripts()
{
	gameHandlers = {};

	if (gameScriptInterface.loadFile("data/scripts/events/game.lua") != 0) {
		std::cout << "[Warning - tfs::events::game::load_from_scripts] Cannot load game events." << std::endl;
		std::cout << gameScriptInterface.getLastLuaError() << std::endl;
		return;
	}

	gameHandlers.onStartup = gameScriptInterface.getMetaEvent("Game", "onStartup");
	gameHandlers.onShutdown = gameScriptInterface.getMetaEvent("Game", "onShutdown");
	gameHandlers.onSave = gameScriptInterface.getMetaEvent("Game", "onSave");
}

} // namespace

namespace tfs::events::game {

void load()
{
	gameScriptInterface.initState();

	loadGameScripts();
}

void reload()
{
	gameScriptInterface.reInitState();

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

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onStartup, &gameScriptInterface);

	gameScriptInterface.pushFunction(gameHandlers.onStartup);

	gameScriptInterface.callVoidFunction(0);
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

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onShutdown, &gameScriptInterface);

	gameScriptInterface.pushFunction(gameHandlers.onShutdown);

	gameScriptInterface.callVoidFunction(0);
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

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(gameHandlers.onSave, &gameScriptInterface);

	gameScriptInterface.pushFunction(gameHandlers.onSave);

	gameScriptInterface.callVoidFunction(0);
}

} // namespace tfs::events::game
