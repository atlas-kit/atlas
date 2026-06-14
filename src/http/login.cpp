#include "../otpch.h"

#include "../database.h"
#include "../game.h"
#include "characters.h"
#include "error.h"
#include "router.h"

#include <chrono>

extern Game g_game;
extern Vocations g_vocations;

namespace json = boost::json;

json::value tfs::http::routes::handle_login(const json::object& body, std::string_view ip)
{
	auto emailField = body.if_contains("email");
	if (!emailField || !emailField->is_string()) {
		return make_error_response(
		    {.code = 3, .message = "Tibia account email address or Tibia password is not correct."});
	}

	auto passwordField = body.if_contains("password");
	if (!passwordField || !passwordField->is_string()) {
		return make_error_response(
		    {.code = 3, .message = "Tibia account email address or Tibia password is not correct."});
	}

	thread_local auto& db = Database::getInstance();

	const auto& result = db.storeQuery(std::format(
	    "SELECT `id`, UNHEX(`password`) AS `password`, `secret`, `premium_ends_at` FROM `accounts` WHERE `email` = {:s}",
	    db.escapeString(emailField->get_string())));
	if (!result) {
		return make_error_response(
		    {.code = 3, .message = "Tibia account email address or Tibia password is not correct."});
	}

	auto password = result->getString("password");
	if (password != transformToSHA1(passwordField->get_string())) {
		return make_error_response(
		    {.code = 3, .message = "Tibia account email address or Tibia password is not correct."});
	}

	auto now = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	auto secret = result->getString("secret");
	if (!secret.empty()) {
		auto token = body.if_contains("token");
		if (!token || !token->is_string()) {
			return make_error_response({.code = 6, .message = "Two-factor token required for authentication."});
		}

		uint64_t ticks = now / AUTHENTICATOR_PERIOD;
		if (token->get_string() != generateToken(secret, ticks) &&
		    token->get_string() != generateToken(secret, ticks - 1) &&
		    token->get_string() != generateToken(secret, ticks + 1)) {
			return make_error_response({.code = 6, .message = "Two-factor token required for authentication."});
		}
	}

	return load_characters(db, ip, result->getNumber<uint64_t>("id"), result->getNumber<int64_t>("premium_ends_at"),
	                       now);
}
