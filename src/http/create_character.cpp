#include "../otpch.h"

#include "../database.h"
#include "../game.h"
#include "characters.h"
#include "error.h"
#include "router.h"
#include "validation.h"

namespace json = boost::json;

extern Vocations g_vocations;
extern Game g_game;

json::value tfs::http::routes::handle_create_character(const json::object& body, std::string_view ip)
{
	using namespace std::chrono;
	thread_local auto& db = Database::getInstance();

	auto sessionKeyField = body.if_contains("sessionkey");
	if (!sessionKeyField || !sessionKeyField->is_string()) {
		return make_error_response({.code = 1, .message = "Invalid session."});
	}

	auto characterNameField = body.if_contains("charactername");
	if (!characterNameField || !characterNameField->is_string()) {
		return make_error_response({
		    .code = 6,
		    .message = "Please enter a name for your character!",
		    .additional_fields = {{"CharacterName", ""}, {"Success", false}},
		});
	}

	const auto characterName = characterNameField->get_string();
	if (characterName.empty()) {
		return make_error_response({
		    .code = 6,
		    .message = "Please enter a name for your character!",
		    .additional_fields = {{"CharacterName", ""}, {"Success", false}},
		});
	}

	if (const auto msg = is_valid_character_name(characterName)) {
		return make_error_response({
		    .code = 99,
		    .message = msg.value(),
		    .additional_fields = {{"CharacterName", characterName}, {"Success", false}},
		});
	}

	const auto sexField = body.if_contains("charactersex");
	if (!sexField || !sexField->is_string()) {
		// TODO: figure out response code and message
		return make_error_response();
	}

	PlayerSex_t sex;
	uint16_t lookType;

	const auto sexValue = sexField->get_string();
	if (sexValue == "female") {
		sex = PLAYERSEX_FEMALE;
		lookType = 128;
	} else if (sexValue == "male") {
		sex = PLAYERSEX_MALE;
		lookType = 136;
	} else {
		// TODO: figure out response code and message
		return make_error_response();
	}

	const auto now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

	const auto& sessionResult = db.storeQuery(std::format(
	    "SELECT `s`.`account_id`, `a`.`premium_ends_at` FROM `sessions` `s` INNER JOIN `accounts` `a` ON `a`.`id` = `s`.`account_id` WHERE `s`.`token` = FROM_BASE64({:s}) AND (`s`.`expired_at` IS NULL OR `s`.`expired_at` > {:d})",
	    db.escapeString(sessionKeyField->get_string()), now));

	if (!sessionResult) {
		return make_error_response({.code = 1, .message = "Invalid session."});
	}

	const uint32_t accountId = sessionResult->getNumber<uint32_t>("account_id");
	const int64_t premiumEndsAt = sessionResult->getNumber<int64_t>("premium_ends_at");

	if (db.storeQuery(std::format("SELECT 1 FROM `players` WHERE `name` = {:s}", db.escapeString(characterName)))) {
		return make_error_response(
		    {.code = 54, .message = "This character name is already used. Please select another one!"});
	}

	if (const auto& result = db.storeQuery(
	        std::format("SELECT COUNT(*) AS `count` FROM `players` WHERE `account_id` = {:d}", accountId))) {
		if (result->getNumber<uint32_t>("count") >= 20) {
			return make_error_response({.code = 1, .message = "Character limit reached."});
		}
	}

	if (!db.executeQuery(std::format(
	        "INSERT INTO `players` (`name`, `account_id`, `sex`, `looktype`) VALUES ({:s}, {:d}, {:d}, {:d})",
	        db.escapeString(characterName), accountId, static_cast<uint32_t>(sex), lookType))) {
		return make_error_response();
	}

	return load_characters(db, ip, accountId, premiumEndsAt, now);
}
