#include "../otpch.h"

#include "getaccountcreationstatus.h"

#include "../game.h"
#include "error.h"

extern Game g_game;

namespace {

std::string_view getPvpType()
{
	switch (g_game.getWorldType()) {
		case WORLD_TYPE_PVP:
			return "Open PvP";

		case WORLD_TYPE_NO_PVP:
			return "Optional PvP";

		case WORLD_TYPE_PVP_ENFORCED:
			return "Hardcore PvP";
	}

	std::unreachable();
}

} // namespace

std::pair<beast::http::status, json::value> tfs::http::handle_worlds_info(const json::object&, std::string_view)
{
	return std::pair<beast::http::status, json::value>{beast::http::status::ok,
	                                                   {{"Worlds",
	                                                     {{{"Name", getString(ConfigManager::SERVER_NAME).c_str()},
	                                                       {"Region", getString(ConfigManager::LOCATION)},
	                                                       {"PvPType", getPvpType()},
	                                                       {"PlayersOnline", 0},
	                                                       {"CreationDate", 0},
	                                                       {"BattlEyeActivationTimestamp", 0},
	                                                       {"BattlEyeInitiallyActive", 0},
	                                                       {"PremiumOnly", 0},
	                                                       {"TransferType", "-"}}}},
	                                                    {"IsCaptchaDeactivated", true},
	                                                    {"RecommendedWorld", getString(ConfigManager::SERVER_NAME).c_str()}}};
}
