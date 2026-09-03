#pragma once

#include <boost/beast/http/status.hpp>
#include <boost/json/value.hpp>

namespace beast = boost::beast;
namespace json = boost::json;

namespace tfs::http {

std::pair<beast::http::status, json::value> handle_create_character(const json::object& body, std::string_view ip);

}
