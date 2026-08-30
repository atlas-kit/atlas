#include "../otpch.h"

#include "error.h"
#include "router.h"
#include "validation.h"

namespace json = boost::json;

json::value tfs::http::routes::handle_check_email(const json::object& body, std::string_view)
{
	const auto emailField = body.if_contains("email");
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

	if (!is_valid_email(email)) {
		return make_error_response(
		    {.code = 59,
		     .message = "This email address has an invalid format. Please enter a correct email address!",
		     .additional_fields = {{"EMail", *emailField}}});
	}

	return {{"IsValid", true}, {"EMail", *emailField}};
}
