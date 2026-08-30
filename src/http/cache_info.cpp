#include "../otpch.h"

#include "../database.h"
#include "error.h"
#include "router.h"

namespace json = boost::json;

json::value tfs::http::routes::handle_cache_info(const json::object&, std::string_view)
{
	thread_local auto& db = Database::getInstance();

	const auto& result = db.storeQuery("SELECT COUNT(*) AS `count` FROM `players_online`");
	if (!result) {
		return make_error_response();
	}
	return json::object{{"playersonline", result->getNumber<uint32_t>("count")}};
}
