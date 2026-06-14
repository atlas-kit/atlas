#pragma once

#include <boost/beast/http/status.hpp>
#include <boost/json/value.hpp>
#include <exception>

namespace tfs::http {

namespace detail {

struct ErrorResponseParams
{
	int code = 2;
	std::string_view message =
	    "Internal error. Please try again later or contact customer support if the problem persists.";
	boost::beast::http::status status = boost::beast::http::status::bad_request;
	boost::json::object additional_fields = {};
};

} // namespace detail

class ErrorResponse : public std::exception
{
public:
	ErrorResponse(boost::json::object obj) : obj{std::move(obj)} {}

	auto to_json() const { return obj; }

private:
	boost::json::object obj = {};
};

boost::json::value make_error_response(detail::ErrorResponseParams params = {});

} // namespace tfs::http
