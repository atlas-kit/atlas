#include "../otpch.h"

#include "../tools.h"
#include "router.h"

namespace json = boost::json;

namespace {

const auto onset = std::array{
    "",   "b",  "c",   "d",   "f",   "g",   "h",   "j",   "k",   "l",   "m",   "n",   "p",   "r",   "s",   "t",   "v",
    "w",  "y",  "z",   "bl",  "br",  "cl",  "cr",  "dr",  "fl",  "fr",  "gl",  "gr",  "kl",  "kr",  "pl",  "pr",  "sl",
    "sm", "sn", "sp",  "st",  "str", "sw",  "tr",  "tw",  "vl",  "vr",  "wr",  "ch",  "sh",  "th",  "ph",  "kh",  "zh",
    "dw", "gw", "hw",  "sk",  "sc",  "scr", "shr", "spl", "spr", "thr", "ael", "aer", "al",  "an",  "ar",  "ash", "el",
    "er", "ev", "fal", "gal", "hal", "kal", "lor", "mal", "nor", "or",  "ran", "sar", "tor", "val", "vor",
};

const auto vowel = std::array{
    "a",   "e",   "i",   "o",   "u",   "ae",  "ai",  "ao", "au", "ea", "ei", "eo", "eu",
    "ia",  "ie",  "io",  "iu",  "oa",  "oe",  "oi",  "ou", "ua", "ue", "ui", "uo", "aea",
    "aie", "eia", "eio", "iae", "iao", "ioa", "uai", "aa", "ee", "ii", "oo", "uu",
};

const auto coda = std::array{
    "",    "b",   "d",    "f",    "g",    "k",    "l",    "m",    "n",   "p",    "r",    "s",   "t",   "ck",
    "ct",  "ft",  "ld",   "lk",   "lm",   "ln",   "lp",   "lt",   "mp",  "nd",   "ng",   "nk",  "nt",  "rd",
    "rf",  "rk",  "rl",   "rm",   "rn",   "rp",   "rt",   "sk",   "sp",  "st",   "th",   "rch", "rth", "nth",
    "lth", "mir", "ndar", "rian", "rion", "riel", "thor", "dor",  "dan", "dran", "dris", "gar", "gor", "grim",
    "kas", "kir", "las",  "len",  "lin",  "lor",  "mar",  "mir",  "mon", "nar",  "nir",  "nor", "ras", "ren",
    "ric", "rin", "ron",  "ros",  "tan",  "ter",  "thal", "thor", "tor", "var",  "ven",  "vor", "wyn",
};

std::string generate_name()
{
	std::string name = "";
	while (name.size() < 4) {
		name += std::format("{}{}{}", onset[uniform_random(0, onset.size() - 1)],
		                    vowel[uniform_random(0, vowel.size() - 1)], coda[uniform_random(0, coda.size() - 1)]);
	}
	return ucfirst(name);
}

} // namespace

json::value tfs::http::routes::handle_generate_character_name(const json::object&, std::string_view)
{
	const auto name = std::format("{} {}", generate_name(), generate_name());

	return {{"GeneratedName", name}};
}
