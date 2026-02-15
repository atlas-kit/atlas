#pragma once

#include "script.h"

enum ModuleState
{
	MODULE_STATE_NOT_LOADED,
	MODULE_STATE_LOADING,
	MODULE_STATE_LOADED
};

struct ModuleFile
{
	bool isLib;
	bool enabled;
	std::filesystem::path path;
};

struct Module
{
	std::string name;
	std::vector<ModuleFile> files;
	ModuleState state = MODULE_STATE_NOT_LOADED;
};

namespace tfs::lua::modules {

bool load();
bool reload();
void import(LuaScriptInterface& lsi);

} // namespace tfs::lua::modules
