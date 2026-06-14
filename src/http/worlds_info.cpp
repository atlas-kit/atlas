#include "../otpch.h"

#include "../game.h"
#include "enums.h"
#include "router.h"

namespace json = boost::json;

extern Game g_game;

json::value tfs::http::routes::handle_worlds_info(const json::object&, std::string_view)
{
	return {
	    {"Worlds",
	     {{{"Name", getString(ConfigManager::SERVER_NAME)},
	       {"Region", getString(ConfigManager::LOCATION)},
	       {"PvPType", detail::getPvpTypeName(g_game.getWorldType())},
	       {"PlayersOnline", 0},
	       {"CreationDate", 0},
	       {"BattlEyeActivationTimestamp", 0},
	       {"BattlEyeInitiallyActive", 0},
	       {"PremiumOnly", 0},
	       {"TransferType", "-"}}}},
	    {"IsCaptchaDeactivated", true},
	    {"RecommendedWorld", getString(ConfigManager::SERVER_NAME)},
	};
}
