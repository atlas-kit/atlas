#include "../otpch.h"

#include "createcharacter.h"

#include "../database.h"
#include "../game.h"
#include "../iologindata.h"
#include "error.h"
#include "success.h"

extern Vocations g_vocations;
extern Game g_game;

namespace {

int getPvpType()
{
	switch (g_game.getWorldType()) {
		case WORLD_TYPE_PVP:
			return 0;
		case WORLD_TYPE_NO_PVP:
			return 1;
		case WORLD_TYPE_PVP_ENFORCED:
			return 2;
	}

	std::unreachable();
}

} // namespace

std::pair<beast::http::status, json::value> tfs::http::handle_create_character(const json::object& body,
                                                                               std::string_view)
{
	using namespace std::chrono;
	thread_local auto& db = Database::getInstance();

	auto sessionKeyField = body.if_contains("sessionkey");
	if (!sessionKeyField || !sessionKeyField->is_string()) {
		return make_error_response({.code = 1, .message = "Invalid session."});
	}

	auto nameField = body.if_contains("charactername");
	if (!nameField || !nameField->is_string()) {
		return make_error_response({.code = 1, .message = "Invalid character name."});
	}

	auto sexField = body.if_contains("charactersex");
	if (!sexField || !sexField->is_string()) {
		return make_error_response({.code = 1, .message = "Invalid character sex."});
	}

	const auto& sessionResult =
	    db.storeQuery(std::format("SELECT s.`account_id`, a.`premium_ends_at` "
	                              "FROM `sessions` s "
	                              "INNER JOIN `accounts` a ON a.`id` = s.`account_id` "
	                              "WHERE s.`token` = FROM_BASE64({:s}) "
	                              "AND (s.`expired_at` IS NULL OR s.`expired_at` > NOW())",
	                              db.escapeString(sessionKeyField->get_string())));

	if (!sessionResult) {
		return make_error_response({.code = 1, .message = "Invalid session."});
	}

	const uint32_t accountId = sessionResult->getNumber<uint32_t>("account_id");

	const std::string characterName(nameField->get_string());

	if (characterName.empty()) {
		return make_error_response({.code = 1, .message = "Invalid character name."});
	}

	if (IOLoginData::getGuidByName(characterName) != 0) {
		return make_error_response({.code = 1, .message = "Character name already exists."});
	}

	if (const auto& result = db.storeQuery(std::format("SELECT COUNT(*) AS `count` "
	                                                   "FROM `players` "
	                                                   "WHERE `account_id` = {:d}",
	                                                   accountId))) {
		if (result->getNumber<uint32_t>("count") >= 20) {
			return make_error_response({.code = 1, .message = "Character limit reached."});
		}
	}

	PlayerSex_t sex;
	uint16_t lookType;

	if (sexField->get_string() == "male") {
		sex = PLAYERSEX_MALE;
		lookType = 128;
	} else if (sexField->get_string() == "female") {
		sex = PLAYERSEX_FEMALE;
		lookType = 136;
	} else {
		return make_error_response({.code = 1, .message = "Invalid character sex."});
	}

	if (!db.executeQuery(std::format("INSERT INTO `players` "
	                                 "(`name`, `account_id`, `sex`, `looktype`) "
	                                 "VALUES ({:s}, {:d}, {:d}, {:d})",
	                                 db.escapeString(characterName), accountId, static_cast<uint32_t>(sex),
	                                 lookType))) {
		return make_error_response();
	}

	json::array characters;
	if (const auto& playersRes = db.storeQuery(std::format(
	        "SELECT `id`, `name`, `level`, `vocation`, `lastlogin`, `sex`, `looktype`, `lookhead`, `lookbody`, `looklegs`, `lookfeet`, `lookaddons` FROM `players` WHERE `account_id` = {:d}",
	        accountId))) {
		uint32_t lastLogin = 0;
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
	        {"pvptype", getPvpType()},
	    },
	};

	return make_success_response({
	    {"playdata",
	     {
	         {"worlds", worlds},
	         {"characters", characters},
	     }},
	});
}
