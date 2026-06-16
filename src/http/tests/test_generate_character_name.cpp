#define BOOST_TEST_MODULE http_generate_character_name

#include "../../otpch.h"

#include "../router.h"
#include "../validation.h"

#include <boost/test/unit_test.hpp>

using namespace std::chrono;

namespace routes = tfs::http::routes;

BOOST_AUTO_TEST_CASE(test_generate_character_name_only_valid_names)
{
	for (auto i = 0; i < 100'000; ++i) {
		const auto name = routes::handle_generate_character_name({}, "").as_object().at("GeneratedName").as_string();

		BOOST_TEST(!tfs::http::is_valid_character_name(name).has_value());
	}
}
