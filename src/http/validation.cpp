#include "../otpch.h"

#include "validation.h"

#include <ranges>

static constexpr std::string_view vowels = "aeiouyAEIOUY";

std::optional<std::string_view> tfs::http::is_valid_character_name(std::string_view name)
{
	if (name.length() < 2 || name.length() > 29) {
		return "A name must have at least 2 but no more than 29 letters.";
	}

	if (name[0] == ' ') {
		return "This name contains a space at the beginning. Please remove this space.";
	}

	if (name[name.length() - 1] == ' ') {
		return "This name contains a space at the end. Please remove this space.";
	}

	if (name[0] < 'A' || name[0] > 'Z') {
		return "The first letter of a name must be an uppercase letter.";
	}

	if (!std::all_of(name.begin(), name.end(), [](char c) { return std::isalpha(c) || c == ' '; })) {
		return "This name contains invalid letters. Please use only A-Z, a-z and space!";
	}

	int words = 0;
	for (const auto& word : std::views::split(name, ' ')) {
		if (word.empty()) {
			return "This name contains more than one space between words. Please use only one space between words.";
		}

		if (word.size() == 1) {
			return "This name contains a word with only one letter. Please use more than one letter for each word.";
		}

		if (word.size() > 14) {
			return "This name contains a word that is too long. Please use no more than 14 letters for each word.";
		}

		if (std::ranges::none_of(word, [](char c) { return vowels.contains(c); })) {
			return "This name contains a word without vowels. Please choose another name.";
		}

		if (std::ranges::any_of(word | std::views::drop(1), [](char c) { return std::isupper(c); })) {
			return "In names capital letters are only allowed at the beginning of a word.";
		}

		++words;
	}

	if (words > 3) {
		return "This name contains more than 3 words. Please choose another name.";
	}

	return std::nullopt;
}
