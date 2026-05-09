#define BOOST_TEST_MODULE app_loop

#include "../otpch.h"

#include "../application/app.h"
#include "../application/clock.h"
#include "../application/events.h"
#include "../application/module.h"
#include "../application/runtime.h"
#include "../application/schedule.h"
#include "../app_loop.h"
#include "../modules/app_loop/module.h"

#include <boost/test/unit_test.hpp>

using namespace tfs::application;
using namespace std::chrono_literals;

namespace {

class ManualClock final : public Clock
{
public:
	explicit ManualClock(std::chrono::microseconds step) : step(step) {}

	TimePoint now() override
	{
		auto result = current;
		current += step;
		return result;
	}

private:
	TimePoint current{};
	std::chrono::microseconds step;
};

class StopAfterUpdateModule final : public Module
{
public:
	void build(App& app) override
	{
		app.add_system<Update>([runtime = app.read_resource<Runtime>()] { runtime->stop(); });
	}
};

} // namespace

BOOST_AUTO_TEST_CASE(app_loop_module_runs_enqueued_callbacks_from_fixed_update_in_fifo_order)
{
	std::vector<int> order;
	App app{std::make_shared<ManualClock>(50000us)};

	class TestModule final : public Module
	{
	public:
		explicit TestModule(std::vector<int>& order) : order(order) {}

		void build(App& app) override
		{
			app.add_observer<StartupEvent>([this](const std::shared_ptr<StartupEvent>&) {
				g_appLoop.enqueue([this] {
					order.push_back(1);
					g_appLoop.enqueue([this] { order.push_back(3); });
				});
				g_appLoop.enqueue([this] { order.push_back(2); });
			});
		}

	private:
		std::vector<int>& order;
	};

	app.add_module<tfs::modules::app_loop::AppLoopModule>();
	app.add_module<TestModule>(order);
	app.add_module<StopAfterUpdateModule>();

	BOOST_TEST(app.run() == 0);
	BOOST_REQUIRE_EQUAL(order.size(), 3);
	BOOST_TEST(order[0] == 1);
	BOOST_TEST(order[1] == 2);
	BOOST_TEST(order[2] == 3);
}

BOOST_AUTO_TEST_CASE(app_loop_module_runs_delayed_events_in_fifo_order)
{
	std::vector<int> order;
	App app{std::make_shared<ManualClock>(50000us)};

	class TestModule final : public Module
	{
	public:
		explicit TestModule(std::vector<int>& order) : order(order) {}

		void build(App& app) override
		{
			app.add_observer<StartupEvent>([this](const std::shared_ptr<StartupEvent>&) {
				g_appLoop.schedule(createDelayedAppLoopEvent(0, [this] { order.push_back(1); }));
				g_appLoop.schedule(createDelayedAppLoopEvent(0, [this] { order.push_back(2); }));
			});
		}

	private:
		std::vector<int>& order;
	};

	app.add_module<tfs::modules::app_loop::AppLoopModule>();
	app.add_module<TestModule>(order);
	app.add_module<StopAfterUpdateModule>();

	BOOST_TEST(app.run() == 0);
	BOOST_REQUIRE_EQUAL(order.size(), 2);
	BOOST_TEST(order[0] == 1);
	BOOST_TEST(order[1] == 2);
}
