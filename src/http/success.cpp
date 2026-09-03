#include "success.h"

std::pair<beast::http::status, json::value> tfs::http::make_success_response(json::object body)
{
	body["Success"] = true;

	return std::make_pair(beast::http::status::ok, std::move(body));
}
