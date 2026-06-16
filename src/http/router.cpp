#include "../otpch.h"

#include "router.h"

#include "../tools.h"
#include "boost/algorithm/string/case_conv.hpp"
#include "error.h"

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

namespace beast = boost::beast;
namespace json = boost::json;
namespace routes = tfs::http::routes;

namespace {

const auto handlers =
    boost::container::flat_map<std::string_view, std::function<json::value(const json::object&, std::string_view)>>{{
        {"cacheinfo", routes::handle_cache_info},
        {"checkcharactername", routes::handle_check_character_name},
        {"checkemail", routes::handle_check_email},
        {"checkpassword", routes::handle_check_password},
        {"createaccountandcharacter", routes::handle_create_account},
        {"createcharacter", routes::handle_create_character},
        {"generatecharactername", routes::handle_generate_character_name},
        {"getaccountcreationstatus", routes::handle_worlds_info},
        {"login", routes::handle_login},
        {"serverinfo", routes::handle_server_info},
    }};

auto normalize_keys(const json::object& obj)
{
	json::object normalized;
	for (auto&& [key, value] : obj) {
		auto lower = boost::algorithm::to_lower_copy(std::string{key});
		normalized[lower] = value;
	}
	return normalized;
}

auto router(const beast::http::request<beast::http::string_body>& req, std::string_view ip)
{
	using tfs::http::make_error_response;

	thread_local json::monotonic_resource mr;
	tfs::scope_exit clear_mr{[] { mr.release(); }};

	boost::system::error_code ec;
	auto parsed_body = json::parse(req.body(), ec, &mr);
	if (ec || !parsed_body.is_object()) {
		spdlog::trace("Received request with invalid JSON body: {:s}", req.body());
		return make_error_response({.code = 2, .message = "Invalid request body."});
	}

	const auto body = normalize_keys(parsed_body.get_object());
	const auto typeField = body.if_contains("type");
	if (!typeField || !typeField->is_string()) {
		spdlog::trace("Received request with missing or invalid type field: {:s}", json::serialize(body));
		return make_error_response({.code = 2, .message = "Invalid request type."});
	}

	auto type = boost::algorithm::to_lower_copy(typeField->get_string());
	if (auto handler = handlers.find(type); handler != handlers.end()) {
		return handler->second(body, ip);
	}

	spdlog::debug("Received request with unknown type: {:s}", type.subview());
	return make_error_response({.code = 2, .message = "Invalid request type."});
}

} // namespace

beast::http::message_generator tfs::http::handle_request(const beast::http::request<beast::http::string_body>& req,
                                                         std::string_view ip)
{
	auto&& body = router(req, ip);

	beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
	res.body() = json::serialize(body);
	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}
