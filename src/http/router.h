#pragma once

#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json.hpp>

namespace tfs::http {

namespace routes {

boost::json::value handle_cache_info(const boost::json::object& body, std::string_view ip);
boost::json::value handle_check_character_name(const boost::json::object& body, std::string_view ip);
boost::json::value handle_check_email(const boost::json::object& body, std::string_view ip);
boost::json::value handle_check_password(const boost::json::object& body, std::string_view ip);
boost::json::value handle_create_account(const boost::json::object& body, std::string_view ip);
boost::json::value handle_login(const boost::json::object& body, std::string_view ip);
boost::json::value handle_server_info(const boost::json::object& body, std::string_view ip);

} // namespace routes

boost::beast::http::message_generator handle_request(
    const boost::beast::http::request<boost::beast::http::string_body>& req, std::string_view ip);

} // namespace tfs::http
