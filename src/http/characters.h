#pragma once

#include <boost/json.hpp>

class Database;

namespace tfs::http {

boost::json::value load_characters(Database& db, std::string_view ip, uint32_t accountId, int64_t premiumEndsAt,
                                   int64_t now);

} // namespace tfs::http