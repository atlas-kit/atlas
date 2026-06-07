#define BOOST_TEST_MODULE reactor

#include "../otpch.h"

#include "../reactor.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_send_executes)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);
}

BOOST_AUTO_TEST_CASE(test_schedule_immediate_executes)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);
}

BOOST_AUTO_TEST_CASE(test_send_before_schedule_immediate)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.schedule(0ms, [&] { executionOrder.push_back(2); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 2);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 2);
}

BOOST_AUTO_TEST_CASE(test_send_executes_before_schedule)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.send([&] { executionOrder.push_back(2); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 2);
	BOOST_TEST(executionOrder[0] == 2);
	BOOST_TEST(executionOrder[1] == 1);
}

BOOST_AUTO_TEST_CASE(test_multiple_sends_in_order)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.send([&] { executionOrder.push_back(2); });
	reactor.send([&] { executionOrder.push_back(3); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 3);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 2);
	BOOST_TEST(executionOrder[2] == 3);
}

BOOST_AUTO_TEST_CASE(test_multiple_schedules_in_order)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.schedule(0ms, [&] { executionOrder.push_back(2); });
	reactor.schedule(0ms, [&] { executionOrder.push_back(3); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 3);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 2);
	BOOST_TEST(executionOrder[2] == 3);
}

BOOST_AUTO_TEST_CASE(test_cancel_scheduled_task)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	auto identifier = reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.cancel(identifier);
	reactor.runOnce();

	BOOST_TEST(executionOrder.empty());
}

BOOST_AUTO_TEST_CASE(test_cancel_one_of_multiple)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	auto identifier = reactor.schedule(0ms, [&] { executionOrder.push_back(2); });
	reactor.schedule(0ms, [&] { executionOrder.push_back(3); });
	reactor.cancel(identifier);
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 2);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 3);
}

BOOST_AUTO_TEST_CASE(test_cancel_nonexistent_identifier)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.cancel(999);
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);
}

BOOST_AUTO_TEST_CASE(test_cancel_identifier_zero)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.cancel(0);
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);
}

BOOST_AUTO_TEST_CASE(test_send_with_expiration_drops_after_deadline)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send(-1ms, [&] { executionOrder.push_back(1); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.empty());
}

BOOST_AUTO_TEST_CASE(test_send_with_expiration_executes_before_deadline)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send(1h, [&] { executionOrder.push_back(1); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);
}

BOOST_AUTO_TEST_CASE(test_interleaved_send_and_schedule)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.send([&] { executionOrder.push_back(2); });
	reactor.schedule(0ms, [&] { executionOrder.push_back(3); });
	reactor.send([&] { executionOrder.push_back(4); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 4);
	BOOST_TEST(executionOrder[0] == 2);
	BOOST_TEST(executionOrder[1] == 4);
	BOOST_TEST(executionOrder[2] == 1);
	BOOST_TEST(executionOrder[3] == 3);
}

BOOST_AUTO_TEST_CASE(test_scheduled_task_does_not_fire_before_delay)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(1h, [&] { executionOrder.push_back(1); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.empty());
}

BOOST_AUTO_TEST_CASE(test_scheduled_task_does_not_block_send)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.schedule(1h, [&] { executionOrder.push_back(1); });
	reactor.send([&] { executionOrder.push_back(2); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 2);
}

BOOST_AUTO_TEST_CASE(test_cancel_after_runOnce)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	auto identifier = reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.runOnce();
	BOOST_TEST(executionOrder.size() == 1);
	BOOST_TEST(executionOrder[0] == 1);

	reactor.cancel(identifier);
	reactor.runOnce();
	BOOST_TEST(executionOrder.size() == 1);
}

BOOST_AUTO_TEST_CASE(test_multiple_runOnce_accumulates)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	reactor.send([&] { executionOrder.push_back(1); });
	reactor.runOnce();
	reactor.send([&] { executionOrder.push_back(2); });
	reactor.runOnce();
	reactor.send([&] { executionOrder.push_back(3); });
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 3);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 2);
	BOOST_TEST(executionOrder[2] == 3);
}

BOOST_AUTO_TEST_CASE(test_schedule_returns_unique_identifiers)
{
	TaskReactor reactor;

	auto id1 = reactor.schedule(0ms, [] {});
	auto id2 = reactor.schedule(0ms, [] {});
	auto id3 = reactor.schedule(0ms, [] {});

	BOOST_TEST(id1 != id2);
	BOOST_TEST(id1 != id3);
	BOOST_TEST(id2 != id3);
	BOOST_TEST(id1 == 1);
	BOOST_TEST(id2 == 2);
	BOOST_TEST(id3 == 3);
}

BOOST_AUTO_TEST_CASE(test_cancel_expired_identifier_is_noop)
{
	TaskReactor reactor;
	std::vector<int> executionOrder;

	auto identifier = reactor.schedule(0ms, [&] { executionOrder.push_back(1); });
	reactor.runOnce();
	BOOST_TEST(executionOrder.size() == 1);

	reactor.schedule(0ms, [&] { executionOrder.push_back(2); });
	reactor.cancel(identifier);
	reactor.runOnce();

	BOOST_TEST(executionOrder.size() == 2);
	BOOST_TEST(executionOrder[0] == 1);
	BOOST_TEST(executionOrder[1] == 2);
}
