#include "../otpch.h"

#include "enums.h"
#include "error.h"
#include "router.h"
#include "validation.h"

namespace json = boost::json;

json::value tfs::http::routes::handle_check_password(const json::object& body, std::string_view)
{
	const auto passwordField = body.if_contains("password1");
	if (!passwordField) {
		return make_error_response(
		    {.code = 11, .message = "Please enter a password.", .additional_fields = {{"Password1", nullptr}}});
	}

	// CipSoft accepts numbers for the field in this route, but the client always sends as a string
	if (!passwordField->is_string()) {
		return make_error_response(
		    {.code = 11, .message = "Please enter a password.", .additional_fields = {{"Password1", *passwordField}}});
	}

	const auto password = passwordField->get_string();
	if (password.empty()) {
		return make_error_response(
		    {.code = 11, .message = "Please enter your password address!", .additional_fields = {{"Password1", ""}}});
	}

	const auto requirements = check_password_strength(password);
	const auto strength = requirements.strength();

	return {
	    {"PasswordRequirements",
	     {
	         {"PasswordLength", requirements.length},
	         // CipSoft calls it "InvalidCharacters" in the response, but it means only valid characters are present
	         {"InvalidCharacters", requirements.validChars},
	         {"HasLowerCase", requirements.lowercase},
	         {"HasUpperCase", requirements.uppercase},
	         {"HasNumber", requirements.digit},
	     }},
	    {"Password1", password},
	    {"PasswordStrength", strength},
	    {"PasswordStrengthColor", detail::getPasswordStrengthColor(strength)},
	    {"PasswordValid", requirements.is_valid()},
	};
}