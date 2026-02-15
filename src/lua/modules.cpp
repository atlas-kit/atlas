#include "../otpch.h"

#include "modules.h"

#include "../configmanager.h"
#include "api.h"
#include "register.h"

namespace fs = std::filesystem;

namespace {

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

LuaScriptInterface lsi("Modules Interface");

std::map<std::string, Module> loaded_modules;

void loadModulesFiles()
{
	loaded_modules.clear();

	const auto dir = fs::current_path() / "modules";
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		return;
	}

	for (fs::recursive_directory_iterator it(dir), end; it != end; ++it) {
		if (!fs::is_regular_file(*it)) {
			continue;
		}

		const auto& path = it->path();
		if (path.extension() != ".lua") {
			continue;
		}

		const auto relative = fs::relative(path, dir);
		if (relative.empty()) {
			continue;
		}

		const auto itPart = relative.begin();
		const auto name = itPart->string();
		const auto enabled = path.filename().string().find("#") == std::string::npos;

		auto isLib = false;
		for (const auto& part : relative) {
			if (part == "lib") {
				isLib = true;
				break;
			}
		}

		loaded_modules[name].name = name;
		loaded_modules[name].files.push_back({isLib, enabled, path});
	}

	for (auto& [name, module] : loaded_modules) {
		std::sort(module.files.begin(), module.files.end(), [](const auto& previous, const auto& current) {
			if (previous.isLib != current.isLib) {
				return previous.isLib > current.isLib;
			}
			return previous.path < current.path;
		});
	}
}

void loadModule(const std::string& name, bool reload)
{
	auto it = loaded_modules.find(name);
	if (it == loaded_modules.end()) {
		throw std::runtime_error("Module '" + name + "' not found!");
	}

	auto& module = it->second;
	if (module.state == MODULE_STATE_LOADED) {
		return;
	}

	if (module.state == MODULE_STATE_LOADING) {
		throw std::runtime_error("Circular dependency detected in module: " + name);
	}

	module.state = MODULE_STATE_LOADING;

	for (const auto& file : module.files) {
		if (!file.enabled) {
			continue;
		}

		if (reload && file.isLib) {
			continue;
		}

		const auto fileName = file.path.filename().string();

		if (lsi.loadFile(file.path.string()) == -1) {
			if (getBoolean(ConfigManager::MODULES_CONSOLE_LOGS)) {
				if (file.isLib) {
					std::cout << "> [" << module.name << "] " << fileName << " [lib error]" << std::endl;
				} else {
					std::cout << "> [" << module.name << "] " << fileName << " [script error]" << std::endl;
				}

				std::cout << "^ " << lsi.getLastLuaError() << std::endl;
			}

			throw std::runtime_error("Failed to load " + name + " (" + file.path.filename().string() + ")");
		}

		if (getBoolean(ConfigManager::MODULES_CONSOLE_LOGS)) {
			if (reload) {
				if (file.isLib) {
					std::cout << "> [" << module.name << "] " << fileName << " [lib reloaded]" << std::endl;
				} else {
					std::cout << "> [" << module.name << "] " << fileName << " [script reloaded]" << std::endl;
				}
			} else {
				if (file.isLib) {
					std::cout << "> [" << module.name << "] " << fileName << " [lib loaded]" << std::endl;
				} else {
					std::cout << "> [" << module.name << "] " << fileName << " [script loaded]" << std::endl;
				}
			}
		}
	}

	module.state = MODULE_STATE_LOADED;
}

int luaModulesModule(lua_State* L)
{
	const auto name = tfs::lua::getString(L, 1);
	try {
		loadModule(name, false);
		return 1;
	} catch (const std::exception& e) {
		return luaL_error(L, "Failed to load module '%s': %s", name.c_str(), e.what());
	}
}

} // namespace

namespace tfs::lua::modules {

bool load()
{
	std::cout << ">> Loading modules" << std::endl;

	if (!lsi.initState()) {
		return false;
	}

	loadModulesFiles();

	for (const auto& [name, module] : loaded_modules) {
		if (module.state == MODULE_STATE_LOADED) {
			continue;
		}

		try {
			loadModule(name, false);
		} catch (const std::exception& e) {
			std::cout << "[Module error - tfs::lua::modules::load] " << e.what() << std::endl;
			return false;
		}
	}

	return true;
}

bool reload()
{
	std::cout << ">> Reloading modules" << std::endl;

	if (!lsi.reInitState()) {
		return false;
	}

	loadModulesFiles();

	for (const auto& [name, module] : loaded_modules) {
		if (module.state == MODULE_STATE_LOADED) {
			continue;
		}

		try {
			loadModule(name, true);
		} catch (const std::exception& e) {
			std::cout << "[Module error - tfs::lua::modules::reload] " << e.what() << std::endl;
			return false;
		}
	}

	return true;
}

void import(LuaScriptInterface& lsi)
{
	lsi.registerGlobalMethod("require_module", luaModulesModule);

	registerStdLib(lsi);
	registerGlobals(lsi);
	registerConfigManager(lsi);
	registerDatabase(lsi);
	registerAction(lsi);
	registerCombat(lsi);
	registerCondition(lsi);
	registerThing(lsi);

	registerCreature(lsi);
	registerMonster(lsi); // requires creature
	registerNpc(lsi);     // requires creature
	registerPlayer(lsi);  // requires creature

	registerGame(lsi);
	registerGlobalEvent(lsi);
	registerGroup(lsi);
	registerGuild(lsi);
	registerHouse(lsi);

	registerItem(lsi);
	registerContainer(lsi); // requires item
	registerPodium(lsi);    // requires item
	registerTeleport(lsi);  // requires item

	registerItemType(lsi);
	registerModalWindow(lsi);
	registerMonsters(lsi);
	registerMoveEvent(lsi);
	registerNetworkMessage(lsi);
	registerOutfit(lsi);
	registerParty(lsi);
	registerPosition(lsi);
	registerSpell(lsi);
	registerTalkAction(lsi);
	registerTile(lsi);
	registerVariant(lsi);
	registerVocation(lsi);
	registerWeapon(lsi);
	registerXml(lsi);
}

} // namespace tfs::lua::modules
