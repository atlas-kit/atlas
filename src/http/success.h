#pragma once

#include <boost/beast/http/status.hpp>
#include <boost/json/value.hpp>

namespace beast = boost::beast;
namespace json = boost::json;

namespace tfs::http {

std::pair<beast::http::status, json::value> make_success_response(json::object body = {});

} // namespace tfs::http
