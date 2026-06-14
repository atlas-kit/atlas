#include "../otpch.h"

#include "characters.h"

#include "../base64.h"
#include "../configmanager.h"
#include "../database.h"
#include "../game.h"
#include "enums.h"
#include "error.h"

namespace json = boost::json;

extern Game g_game;
extern Vocations g_vocations;

json::value tfs::http::load_characters(Database& db, std::string_view ip, uint32_t accountId, int64_t premiumEndsAt,
                                       int64_t now)
{
	std::string sessionKey = randomBytes(16);
	if (!db.executeQuery(
	        std::format("INSERT INTO `sessions` (`token`, `account_id`, `ip`) VALUES ({:s}, {:d}, INET6_ATON({:s}))",
	                    db.escapeString(sessionKey), accountId, db.escapeString(ip)))) {
		return make_error_response();
	}

	json::array characters;
	uint32_t lastLogin = 0;
	if (const auto& playersRes = db.storeQuery(std::format(
	        "SELECT `id`, `name`, `level`, `vocation`, `lastlogin`, `sex`, `looktype`, `lookhead`, `lookbody`, `looklegs`, `lookfeet`, `lookaddons` FROM `players` WHERE `account_id` = {:d}",
	        accountId))) {
		do {
			auto vocation = g_vocations.getVocation(playersRes->getNumber<uint32_t>("vocation"));
			assert(vocation);

			characters.push_back({
			    {"worldid", 0}, // not implemented
			    {"name", playersRes->getString("name")},
			    {"level", playersRes->getNumber<uint32_t>("level")},
			    {"vocation", vocation->getVocName()},
			    {"lastlogin", playersRes->getNumber<uint64_t>("lastlogin")},
			    {"ismale", playersRes->getNumber<uint16_t>("sex") == PLAYERSEX_MALE},
			    {"ishidden", false},        // not implemented
			    {"ismaincharacter", false}, // not implemented
			    {"tutorial", false},        // not implemented
			    {"outfitid", playersRes->getNumber<uint32_t>("looktype")},
			    {"headcolor", playersRes->getNumber<uint32_t>("lookhead")},
			    {"torsocolor", playersRes->getNumber<uint32_t>("lookbody")},
			    {"legscolor", playersRes->getNumber<uint32_t>("looklegs")},
			    {"detailcolor", playersRes->getNumber<uint32_t>("lookfeet")},
			    {"addonsflags", playersRes->getNumber<uint32_t>("lookaddons")},
			    {"dailyrewardstate", 0}, // not implemented
			});

			lastLogin = std::max(lastLogin, playersRes->getNumber<uint32_t>("lastlogin"));
		} while (playersRes->next());
	}

	json::array worlds{
	    {
	        {"id", 0}, // not implemented
	        {"name", getString(ConfigManager::SERVER_NAME)},
	        {"externaladdressprotected", getString(ConfigManager::IP)},
	        {"externalportprotected", getNumber(ConfigManager::GAME_PORT)},
	        {"externaladdressunprotected", getString(ConfigManager::IP)},
	        {"externalportunprotected", getNumber(ConfigManager::GAME_PORT)},
	        {"previewstate", 0}, // not implemented
	        {"location", getString(ConfigManager::LOCATION)},
	        {"anticheatprotection", false}, // not implemented
	        {"pvptype", detail::getPvpTypeIndex(g_game.getWorldType())},
	    },
	};

	const auto freePremium = getBoolean(ConfigManager::FREE_PREMIUM);

	return json::object{
	    {"session",
	     {
	         {"sessionkey", tfs::base64::encode(sessionKey)},
	         {"lastlogintime", lastLogin},
	         {"ispremium", freePremium || premiumEndsAt >= now},
	         {"premiumuntil", premiumEndsAt},
	         // not implemented
	         {"status", "active"},
	         {"returnernotification", false},
	         {"showrewardnews", true},
	         {"isreturner", true},
	         {"recoverysetupcomplete", true},
	         {"fpstracking", false},
	         {"optiontracking", false},
	     }},
	    {"playdata",
	     {
	         {"worlds", worlds},
	         {"characters", characters},
	     }},
	};
}