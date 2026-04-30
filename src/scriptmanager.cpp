// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "scriptmanager.h"

#include "actions.h"
#include "chat.h"
#include "events.h"
#include "globalevent.h"
#include "movement.h"
#include "script.h"
#include "spells.h"
#include "talkaction.h"
#include "weapons.h"

std::unique_ptr<Actions> g_actions = nullptr;
Chat g_chat;
std::unique_ptr<GlobalEvents> g_globalEvents = nullptr;
std::unique_ptr<Spells> g_spells = nullptr;
std::unique_ptr<TalkActions> g_talkActions = nullptr;
std::unique_ptr<MoveEvents> g_moveEvents = nullptr;
std::unique_ptr<Weapons> g_weapons = nullptr;
std::unique_ptr<Scripts> g_scripts = nullptr;

extern LuaEnvironment g_luaEnvironment;

ScriptingManager::~ScriptingManager()
{
	g_weapons.reset();
	g_spells.reset();
	g_actions.reset();
	g_talkActions.reset();
	g_moveEvents.reset();
	g_globalEvents.reset();
	g_scripts.reset();
}

bool ScriptingManager::loadScriptSystems()
{
	if (g_luaEnvironment.loadFile("data/global.lua") == -1) {
		std::cout << "[Warning - ScriptingManager::loadScriptSystems] Can not load data/global.lua" << std::endl;
	}

	g_scripts = std::make_unique<Scripts>();
	std::cout << ">> Loading lua libs" << std::endl;
	if (!g_scripts->loadScripts("scripts/lib", true, false)) {
		std::cout << "> ERROR: Unable to load lua libs!" << std::endl;
		return false;
	}

	g_weapons = std::make_unique<Weapons>();
	g_weapons->loadDefaults();

	g_spells = std::make_unique<Spells>();
	if (!g_spells->loadFromXml()) {
		std::cout << "> ERROR: Unable to load spells!" << std::endl;
		return false;
	}

	g_actions = std::make_unique<Actions>();
	g_talkActions = std::make_unique<TalkActions>();

	g_moveEvents = std::make_unique<MoveEvents>();
	if (!g_moveEvents->loadFromXml()) {
		std::cout << "> ERROR: Unable to load move events!" << std::endl;
		return false;
	}

	g_globalEvents = std::make_unique<GlobalEvents>();
	if (!g_globalEvents->loadFromXml()) {
		std::cout << "> ERROR: Unable to load global events!" << std::endl;
		return false;
	}

	tfs::events::load();
	return true;
}
