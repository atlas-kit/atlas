#define BOOST_TEST_MODULE scope_exit

#include "../otpch.h"

#include "../tools.h"

#include <boost/test/unit_test.hpp>

// Test that the scope_exit destructor executes the cleanup function
// when the scope is exited normally.
BOOST_AUTO_TEST_SUITE(scope_exit)

BOOST_AUTO_TEST_CASE(test_scope_exit_runs_on_destruction)
{
	bool ran = false;
	{
		// Create a scope_exit with a lambda that sets ran to true
		tfs::scope_exit se{[&] { ran = true; }};
		// Verify the lambda hasn't run yet
		BOOST_TEST(!ran);
		// Exit the scope, triggering the destructor
	}
	// Verify the cleanup lambda was executed
	BOOST_TEST(ran);
}

// Test that scope_exit executes cleanup even when a function
// returns early, ensuring RAII behavior works correctly.
BOOST_AUTO_TEST_CASE(test_scope_exit_with_early_return)
{
	bool ran = false;
	auto func = [&]() -> int {
		tfs::scope_exit se{[&] { ran = true; }};
		// Return early from the function
		if (!ran) {
			return 42;
		}

		std::unreachable();
	};

	int v = func();
	// Verify the function returned correctly
	BOOST_TEST(v == 42);
	// Verify cleanup ran even with early return
	BOOST_TEST(ran);
}

// Test that scope_exit executes cleanup when an exception is thrown,
// verifying that stack unwinding properly triggers the destructor.
BOOST_AUTO_TEST_CASE(test_scope_exit_on_exception)
{
	bool ran = false;
	try {
		// Create a scope_exit that sets ran to true
		tfs::scope_exit se{[&] { ran = true; }};
		// Throw an exception to trigger stack unwinding
		throw std::runtime_error("boom");
		// scope_exit destructor runs during stack unwinding
	} catch (const std::exception& e) {
		// Verify cleanup ran before exception was caught
		BOOST_TEST(ran);
		// Verify we caught the correct exception
		BOOST_TEST(std::string(e.what()) == "boom");
	}
}

BOOST_AUTO_TEST_CASE(test_scope_exit_no_op)
{
	bool ran = false;
	{
		tfs::scope_exit se{[] {}};
		BOOST_TEST(!ran);
	}
	BOOST_TEST(!ran); // no-op lambda doesn't modify ran
}

BOOST_AUTO_TEST_CASE(test_scope_exit_multiple_reverse_order)
{
	std::vector<int> order;
	{
		tfs::scope_exit se1{[&] { order.push_back(1); }};
		tfs::scope_exit se2{[&] { order.push_back(2); }};
		tfs::scope_exit se3{[&] { order.push_back(3); }};
		BOOST_TEST(order.empty());
	}
	// Destructors run in reverse order of construction
	BOOST_REQUIRE(order.size() == 3);
	BOOST_TEST(order[0] == 3);
	BOOST_TEST(order[1] == 2);
	BOOST_TEST(order[2] == 1);
}

BOOST_AUTO_TEST_CASE(test_scope_exit_nested)
{
	bool outerRan = false;
	bool innerRan = false;
	{
		tfs::scope_exit outer{[&] { outerRan = true; }};
		{
			tfs::scope_exit inner{[&] { innerRan = true; }};
			BOOST_TEST(!innerRan);
			BOOST_TEST(!outerRan);
		}
		// inner destroyed when its scope exits
		BOOST_TEST(innerRan);
		BOOST_TEST(!outerRan);
	}
	BOOST_TEST(outerRan);
}

BOOST_AUTO_TEST_CASE(test_scope_exit_move_only_callable)
{
	auto ptr = std::make_unique<int>(42);
	{
		tfs::scope_exit se{[ptr = std::move(ptr)] {
			// ptr is moved into the lambda and destroyed with se
		}};
		BOOST_TEST(!ptr); // ptr was moved
	}
	// se was destroyed, lambda ran
}

BOOST_AUTO_TEST_CASE(test_scope_exit_function_pointer)
{
	bool ran = false;
	auto cleanup = [&] { ran = true; };

	auto fp = +[](void* arg) { // function pointer
		(*static_cast<bool*>(arg)) = true;
	};

	{
		// Use a lambda that wraps the function pointer
		tfs::scope_exit se{[&] { fp(&ran); }};
		BOOST_TEST(!ran);
	}
	BOOST_TEST(ran);
}

BOOST_AUTO_TEST_SUITE_END()
