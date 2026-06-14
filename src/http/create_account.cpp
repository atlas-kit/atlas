#include "../otpch.h"

#include "../database.h"
#include "../tools.h"
#include "characters.h"
#include "error.h"
#include "router.h"
#include "validation.h"

#include <chrono>
#include <regex>
#include <utility>

namespace json = boost::json;

namespace {

std::regex emailRegex{R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)"};

bool is_valid_email(std::string_view email)
{
	// 191 is the maximum length of the email field in the database (767 bytes for utf8mb4, which allows up to 4 bytes
	// per character)
	if (email.empty() || email.length() > 191) {
		return false;
	}

	return std::regex_match(email.begin(), email.end(), emailRegex);
}

bool is_valid_password(std::string_view password) { return !password.empty(); }

} // namespace

json::value tfs::http::routes::handle_create_account(const json::object& body, std::string_view ip)
{
	auto passwordField = body.if_contains("Password");
	if (!passwordField || !passwordField->is_string()) {
		return make_error_response({
		    .code = 87,
		    .message = "Your password does not meet the requirements",
		    .additional_fields = {{"PasswordValid", false}, {"Success", false}},
		});
	}

	std::string password(passwordField->get_string());
	if (!is_valid_password(password)) {
		return make_error_response({
		    .code = 87,
		    .message = "Your password does not meet the requirements",
		    .additional_fields = {{"PasswordValid", false}, {"Success", false}},
		});
	}

	auto emailField = body.if_contains("EMail");
	if (!emailField || !emailField->is_string()) {
		return make_error_response({
		    .code = 57,
		    .message = "Please enter your email address!",
		    .additional_fields = {{"EMail", ""}, {"Success", false}},
		});
	}

	std::string email(emailField->get_string());
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

	auto characterNameField = body.if_contains("CharacterName");
	if (!characterNameField || !characterNameField->is_string()) {
		return make_error_response({
		    .code = 6,
		    .message = "Please enter a name for your character!",
		    .additional_fields = {{"CharacterName", ""}, {"Success", false}},
		});
	}

	std::string characterName(characterNameField->get_string());
	if (characterName.empty()) {
		return make_error_response({
		    .code = 6,
		    .message = "Please enter a name for your character!",
		    .additional_fields = {{"CharacterName", ""}, {"Success", false}},
		});
	}

	if (auto msg = is_valid_character_name(characterName)) {
		return make_error_response({
		    .code = 99,
		    .message = msg.value(),
		    .additional_fields = {{"CharacterName", characterName}, {"Success", false}},
		});
	}

	auto sexField = body.if_contains("CharacterSex");
	if (!sexField || !sexField->is_string()) {
		// TODO: figure out response code and message
		return make_error_response();
	}

	PlayerSex_t sex;
	std::string sexValue(sexField->get_string());
	if (sexValue == "female") {
		sex = PLAYERSEX_FEMALE;
	} else if (sexValue == "male") {
		sex = PLAYERSEX_MALE;
	} else {
		// TODO: figure out response code and message
		return make_error_response();
	}

	thread_local auto& db = Database::getInstance();

	DBTransaction tx;
	tx.begin();

	if (db.storeQuery(std::format("SELECT `id` FROM `accounts` WHERE `email` = {:s}", db.escapeString(email)))) {
		// TODO: figure out response code and message
		return make_error_response({.code = 1, .message = "This e-mail address is already in use."});
	}

	if (db.storeQuery(std::format("SELECT `id` FROM `players` WHERE `name` = {:s}", db.escapeString(characterName)))) {
		return make_error_response(
		    {.code = 54, .message = "This character name is already used. Please select another one!"});
	}

	std::string passwordHash = transformToSHA1(password);

	if (!db.executeQuery(
	        std::format("INSERT INTO `accounts` (`email`, `name`, `password`) VALUES ({:s}, {:s}, HEX({:s}))",
	                    db.escapeString(email), db.escapeString(email), db.escapeString(passwordHash)))) {
		return make_error_response();
	}

	auto accountId = db.getLastInsertId();
	if (accountId == 0) {
		return make_error_response();
	}

	if (!db.executeQuery(std::format("INSERT INTO `players` (`account_id`, `name`, `sex`) VALUES ({:d}, {:s}, {:d})",
	                                 accountId, db.escapeString(characterName), std::to_underlying(sex)))) {
		return make_error_response();
	}

	tx.commit();

	return load_characters(
	    db, ip, accountId, 0,
	    duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}