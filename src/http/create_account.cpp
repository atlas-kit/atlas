#include "../otpch.h"

#include "../database.h"
#include "../tools.h"
#include "characters.h"
#include "error.h"
#include "router.h"
#include "validation.h"

#include <chrono>
#include <utility>

namespace json = boost::json;

json::value tfs::http::routes::handle_create_account(const json::object& body, std::string_view ip)
{
	const auto passwordField = body.if_contains("password");
	if (!passwordField || !passwordField->is_string()) {
		return make_error_response({
		    .code = 87,
		    .message = "Your password does not meet the requirements",
		    .additional_fields = {{"PasswordValid", false}, {"Success", false}},
		});
	}

	const auto password = passwordField->get_string();
	if (auto requirements = check_password_strength(password); !requirements.is_valid()) {
		return make_error_response({
		    .code = 87,
		    .message = "Your password does not meet the requirements",
		    .additional_fields = {{"PasswordValid", false}, {"Success", false}},
		});
	}

	const auto emailField = body.if_contains("email");
	if (!emailField || !emailField->is_string()) {
		return make_error_response({
		    .code = 57,
		    .message = "Please enter your email address!",
		    .additional_fields = {{"EMail", ""}, {"Success", false}},
		});
	}

	const auto email = emailField->get_string();
	if (email.empty()) {
		return make_error_response({
		    .code = 57,
		    .message = "Please enter your email address!",
		    .additional_fields = {{"EMail", ""}, {"Success", false}},
		});
	}

	if (!is_valid_email(email)) {
		return make_error_response({
		    .code = 59,
		    .message = "This email address has an invalid format. Please enter a correct email address!",
		    .additional_fields = {{"EMail", email}, {"Success", false}},
		});
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

	thread_local auto& db = Database::getInstance();

	DBTransaction tx;

	if (!tx.begin()) {
		return make_error_response();
	}

	if (db.storeQuery(std::format("SELECT 1 FROM `accounts` WHERE `email` = {:s}", db.escapeString(email)))) {
		// TODO: figure out response code and message
		return make_error_response({.code = 1, .message = "This e-mail address is already in use."});
	}

	if (db.storeQuery(std::format("SELECT 1 FROM `players` WHERE `name` = {:s}", db.escapeString(characterName)))) {
		return make_error_response(
		    {.code = 54, .message = "This character name is already used. Please select another one!"});
	}

	const std::string passwordHash = transformToSHA1(password);

	if (!db.executeQuery(
	        std::format("INSERT INTO `accounts` (`email`, `name`, `password`) VALUES ({:s}, {:s}, HEX({:s}))",
	                    db.escapeString(email), db.escapeString(email), db.escapeString(passwordHash)))) {
		return make_error_response();
	}

	const auto accountId = db.getLastInsertId();
	if (accountId == 0) {
		return make_error_response();
	}

	if (!db.executeQuery(std::format(
	        "INSERT INTO `players` (`account_id`, `name`, `sex`, `looktype`) VALUES ({:d}, {:s}, {:d}, {:d})",
	        accountId, db.escapeString(characterName), std::to_underlying(sex), lookType))) {
		return make_error_response();
	}

	if (!tx.commit()) {
		return make_error_response();
	}

	return load_characters(
	    db, ip, accountId, 0,
	    duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
