#include "../otpch.h"

#include "router.h"

#include "cacheinfo.h"
#include "createcharacter.h"
#include "error.h"
#include "getaccountcreationstatus.h"
#include "login.h"
#include "serverinfo.h"

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <iostream>

namespace {

auto router(std::string_view type, const json::object& body, std::string_view ip)
{
	using namespace tfs::http;

	if (type == "cacheinfo") {
		return handle_cacheinfo(body, ip);
	}
	if (type == "login") {
		return handle_login(body, ip);
	}
	if (type == "serverinfo") {
		return handle_serverinfo(body, ip);
	}
	if (type == "getaccountcreationstatus") {
		return handle_worlds_info(body, ip);
	}
	if (type == "createCharacter") {
		return handle_create_character(body, ip);
	}

	return make_error_response();
}

thread_local json::monotonic_resource mr;

json::object normalizeKeys(const json::object& obj)
{
	json::object normalized;

	for (auto const& [key, value] : obj) {
		std::string k(key);

		std::ranges::transform(k, k.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		normalized[k] = value;
	}

	return normalized;
}

} // namespace

beast::http::message_generator tfs::http::handle_request(const beast::http::request<beast::http::string_body>& req,
                                                         std::string_view ip)
{
	auto&& [status, responseBody] = [&req, ip]() {
		boost::system::error_code ec;
		auto requestBody = json::parse(req.body(), ec, &mr);
		if (ec || !requestBody.is_object()) {
			return make_error_response({.code = 2, .message = "Invalid request body."});
		}

		const auto& requestBodyObj = normalizeKeys(requestBody.get_object());
		auto typeField = requestBodyObj.if_contains("type");
		if (!typeField || !typeField->is_string()) {
			return make_error_response({.code = 2, .message = "Invalid request type."});
		}

		return router(typeField->get_string(), requestBodyObj, ip);
	}();

	beast::http::response<beast::http::string_body> res{status, req.version()};
	res.body() = json::serialize(responseBody);
	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}
