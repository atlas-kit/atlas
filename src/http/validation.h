#pragma once

#include <algorithm>
class Database;

namespace tfs::http {

namespace detail {

struct PasswordRequirements
{
	bool length = false;
	bool uppercase = false;
	bool lowercase = false;
	bool digit = false;
	bool validChars = false;

	bool is_valid() const { return length && validChars; }

	int strength() const
	{
		int value = 0;
		if (length) ++value;
		if (uppercase) ++value;
		if (lowercase) ++value;
		if (digit) ++value;
		return std::clamp(value, 0, 4);
	}
};

struct PasswordStrength
{
	std::string_view color;
	int value;
};

} // namespace detail

std::optional<std::string_view> is_valid_character_name(std::string_view name);
bool is_valid_email(std::string_view email);
detail::PasswordRequirements check_password_strength(std::string_view password);

} // namespace tfs::http
