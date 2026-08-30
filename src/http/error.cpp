#include "../otpch.h"

#include "error.h"

namespace json = boost::json;

json::value tfs::http::make_error_response(detail::ErrorResponseParams params /*= {}*/)
{
	json::object obj;

	for (const auto& [key, value] : params.additional_fields) {
		obj[key] = value;
	}

	obj["errorCode"] = params.code;
	obj["errorMessage"] = params.message;

	return obj;
}
