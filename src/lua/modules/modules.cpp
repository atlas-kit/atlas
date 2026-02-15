#include "../../otpch.h"

#include "../../player.h"
#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"
#include "../modules.h"

namespace {

int luaModuleModule(lua_State* L)
{
	std::string name = tfs::lua::popString(L); // Adjust to your string getter

	Module* mod = tfs::lua::modules::findByName(name);
	if (!mod) {
		return luaL_error(L, "Module '%s' not found.", name.c_str());
	}

	if (mod->state == ModuleState::LOADED) {
		return 0; // Already loaded, do nothing
	}

	if (mod->state == ModuleState::LOADING) {
		return luaL_error(L, "Circular dependency detected while loading module '%s'.", name.c_str());
	}

	// Start loading
	mod->state = ModuleState::LOADING;

	// Use your existing interface to load the file
	// Note: lsi must be accessible here
	if (lsi.loadFile(mod->path.string()) == -1) {
		std::string err = lsi.getLastLuaError();
		mod->state = ModuleState::NOT_LOADED; // Reset on failure
		return luaL_error(L, "Failed to load module '%s': %s", name.c_str(), err.c_str());
	}

	mod->state = ModuleState::LOADED;
	return 0;
}

} // namespace

void tfs::lua::registerModalWindow(LuaScriptInterface& lsi) {}
