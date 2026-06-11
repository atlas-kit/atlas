#include "../otpch.h"

#include "check_email.h"

#include "error.h"

#include <regex>

namespace {

const std::regex email_regex{R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)"};

} // namespace

std::pair<beast::http::status, json::value> tfs::http::handle_check_email(const json::object& body, std::string_view)
{
	using namespace std::chrono;

	auto emailField = body.if_contains("email");
	if (!emailField) {
		return make_error_response(
		    {.code = 57, .message = "Please enter your email address!", .additional_fields = {{"EMail", ""}}});
	}

	if (!emailField->is_string()) {
		return make_error_response(
		    {.code = 59,
		     .message = "This email address has an invalid format. Please enter a correct email address!",
		     .additional_fields = {{"EMail", *emailField}}});
	}

	const std::string email{emailField->get_string()};
	if (email.empty()) {
		return make_error_response(
		    {.code = 57, .message = "Please enter your email address!", .additional_fields = {{"EMail", ""}}});
	}

	if (!std::regex_match(email, email_regex)) {
		return make_error_response(
		    {.code = 59,
		     .message = "This email address has an invalid format. Please enter a correct email address!",
		     .additional_fields = {{"EMail", *emailField}}});
	}

	return {
	    beast::http::status::ok,
	    {
	        {"IsValid", true},
	        {"EMail", *emailField},
	    },
	};
}
