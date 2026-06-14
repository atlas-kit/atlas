#include "../otpch.h"

#include "router.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "error.h"

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <flat_map>

namespace beast = boost::beast;
namespace json = boost::json;
namespace routes = tfs::http::routes;

namespace {

thread_local json::monotonic_resource mr;

const auto handlers =
    std::flat_map<std::string_view, std::function<json::value(const json::object&, std::string_view)>>{
        {{"cacheinfo", routes::handle_cache_info},
         {"checkcharactername", routes::handle_check_character_name},
         {"createaccountandcharacter", routes::handle_create_account},
         {"login", routes::handle_login},
         {"serverinfo", routes::handle_server_info}}};

auto router(const beast::http::request<beast::http::string_body>& req, std::string_view ip)
{
	using namespace tfs::http;

	boost::system::error_code ec;
	auto requestBody = json::parse(req.body(), ec, &mr);
	if (ec || !requestBody.is_object()) {
		return make_error_response({.code = 2, .message = "Invalid request body."});
	}

	const auto& body = requestBody.get_object();
	auto typeField = body.if_contains("type");
	if (!typeField || !typeField->is_string()) {
		return make_error_response({.code = 2, .message = "Invalid request type."});
	}

	auto type = boost::algorithm::to_lower_copy(typeField->get_string());

	if (auto handler = handlers.find(type); handler != handlers.end()) {
		return handler->second(body, ip);
	}

	return make_error_response();
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
