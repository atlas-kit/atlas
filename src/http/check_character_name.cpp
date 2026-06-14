#include "../otpch.h"

#include "error.h"
#include "router.h"
#include "validation.h"

namespace json = boost::json;

json::value tfs::http::routes::handle_check_character_name(const json::object& body, std::string_view)
{
	auto characterNameField = body.if_contains("CharacterName");
	if (!characterNameField || !characterNameField->is_string()) {
		return make_error_response({
		    .code = 6,
		    .message = "Please enter a name for your character!",
		    .additional_fields = {{"CharacterName", ""}, {"IsAvailable", false}},
		});
	}

	std::string characterName{characterNameField->get_string()};
	if (auto msg = is_valid_character_name(characterName)) {
		return make_error_response({
		    .code = 99,
		    .message = msg.value(),
		    .additional_fields = {{"CharacterName", characterName}, {"IsAvailable", false}},
		});
	}

	return {{{"CharacterName", characterName}, {"IsAvailable", true}}};
}