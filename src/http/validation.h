#pragma once

class Database;

namespace tfs::http {

std::optional<std::string_view> is_valid_character_name(std::string_view name);

}