#define BOOST_TEST_MODULE application

#include "../otpch.h"

#include "../application/app.h"
#include "../application/clock.h"
#include "../application/context.h"
#include "../application/events.h"
#include "../application/module.h"
#include "../application/resources.h"
#include "../application/runtime.h"
#include "../application/schedule.h"
#include "../application/systems.h"
#include "../application/timing/fixed_time.h"
#include "../application/timing/real_time.h"
#include "../application/timing/time.h"
#include "../application/timing/timer.h"
#include "../application/timing/virtual_time.h"

#include <boost/test/unit_test.hpp>

using namespace tfs::application;
using namespace std::chrono_literals;

namespace {

class TestResource final : public Resource
{
public:
	explicit TestResource(int value = 0) : value(value) {}

	int value = 0;
};

class OtherResource final : public Resource
{
};

class ManualClock final : public Clock
{
public:
	explicit ManualClock(std::chrono::microseconds step = 0us) : step(step) {}

	TimePoint now() override
	{
		auto result = current;
		current += step;
		return result;
	}

	void set_step(std::chrono::microseconds value) { step = value; }

private:
	TimePoint current{};
	std::chrono::microseconds step;
};

class TestEvent final : public Event
{
public:
	explicit TestEvent(int value) : value(value) {}

	int value = 0;
};

class TestCancellableEvent final : public CancellableEvent
{
};

struct TestSchedule final : Schedule
{
};

template <typename T>
class TestClock final : public TimeBase<T>
{
public:
	using TimeBase<T>::advance_by;
	using TimeBase<T>::advance_to;
	using TimeBase<T>::set_wrap_period;
};

class StopOnStartupModule final : public Module
{
public:
	void build(App& app) override
	{
		app.insert_resource<TestResource>(7);
		app.add_observer<StartupEvent>([runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) {
			runtime->stop();
		});
	}
};

class LoopModule final : public Module
{
public:
	void build(App& app) override
	{
		auto ticks = app.insert_resource<TestResource>();

		app.add_observer<StartupEvent>([runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) {
			runtime->set_frequency(120);
		});

		app.add_system<Update>([ticks, runtime = app.read_resource<Runtime>()] {
			++ticks->value;
			if (ticks->value >= 3) {
				runtime->stop();
			}
		});
	}
};

} // namespace

BOOST_AUTO_TEST_CASE(runtime_starts_running_and_validates_frequency)
{
	Runtime runtime;

	BOOST_TEST(runtime.is_running());
	BOOST_TEST(runtime.frequency() == 20);

	runtime.stop();
	BOOST_TEST(!runtime.is_running());
	BOOST_TEST(runtime.exit_code() == 0);

	runtime.keep_running();
	BOOST_TEST(runtime.is_running());
	BOOST_TEST(runtime.exit_code() == 0);

	runtime.exit(7);
	BOOST_TEST(!runtime.is_running());
	BOOST_TEST(runtime.exit_code() == 7);

	runtime.keep_running();

	runtime.set_frequency(120);
	BOOST_TEST(runtime.frequency() == 120);

	BOOST_CHECK_THROW(runtime.set_frequency(0), std::invalid_argument);
	BOOST_CHECK_THROW(runtime.set_frequency(121), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(timer_once_finishes_once_and_clamps_elapsed)
{
	Timer timer{100ms};

	BOOST_TEST(timer.duration().count() == 100000);
	BOOST_TEST(timer.remaining().count() == 100000);
	BOOST_TEST(!timer.finished());
	BOOST_TEST(!timer.just_finished());

	timer.tick(40ms);
	BOOST_TEST(timer.elapsed().count() == 40000);
	BOOST_TEST(timer.remaining().count() == 60000);
	BOOST_TEST(!timer.finished());
	BOOST_TEST(!timer.just_finished());

	timer.tick(60ms);
	BOOST_TEST(timer.elapsed().count() == 100000);
	BOOST_TEST(timer.remaining().count() == 0);
	BOOST_TEST(timer.finished());
	BOOST_TEST(timer.just_finished());
	BOOST_TEST(timer.times_finished_this_tick() == 1U);

	timer.tick(50ms);
	BOOST_TEST(timer.elapsed().count() == 100000);
	BOOST_TEST(timer.finished());
	BOOST_TEST(!timer.just_finished());
	BOOST_TEST(timer.times_finished_this_tick() == 0U);

	timer.reset();
	BOOST_TEST(timer.elapsed().count() == 0);
	BOOST_TEST(!timer.finished());
}

BOOST_AUTO_TEST_CASE(timer_repeating_wraps_and_counts_finished_intervals)
{
	Timer timer{100ms, TimerMode::Repeating};

	timer.tick(250ms);
	BOOST_TEST(timer.elapsed().count() == 50000);
	BOOST_TEST(timer.remaining().count() == 50000);
	BOOST_TEST(timer.finished());
	BOOST_TEST(timer.just_finished());
	BOOST_TEST(timer.times_finished_this_tick() == 2U);

	timer.tick(25ms);
	BOOST_TEST(timer.elapsed().count() == 75000);
	BOOST_TEST(!timer.finished());
	BOOST_TEST(!timer.just_finished());

	timer.tick(25ms);
	BOOST_TEST(timer.elapsed().count() == 0);
	BOOST_TEST(timer.finished());
	BOOST_TEST(timer.just_finished());
	BOOST_TEST(timer.times_finished_this_tick() == 1U);
}

BOOST_AUTO_TEST_CASE(timer_pause_and_validation)
{
	Timer timer{100ms};
	timer.pause();
	timer.tick(100ms);
	BOOST_TEST(timer.elapsed().count() == 0);
	BOOST_TEST(!timer.finished());

	timer.unpause();
	timer.tick(100ms);
	BOOST_TEST(timer.finished());

	BOOST_CHECK_THROW(Timer{0ms}, std::invalid_argument);
	BOOST_CHECK_THROW(timer.tick(-1ms), std::invalid_argument);
	BOOST_CHECK_THROW(timer.set_duration(0ms), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(cancellable_event_tracks_cancelled_state)
{
	TestCancellableEvent event;

	BOOST_TEST(!event.is_cancelled());
	event.set_cancelled(true);
	BOOST_TEST(event.is_cancelled());
	event.set_cancelled(false);
	BOOST_TEST(!event.is_cancelled());
}

BOOST_AUTO_TEST_CASE(context_writes_reads_and_rejects_duplicates)
{
	auto resources = std::make_shared<Resources>();
	Context context{resources, std::type_index(typeid(context_writes_reads_and_rejects_duplicates))};

	BOOST_TEST(!context.has<TestResource>());
	BOOST_TEST(context.try_read<TestResource>() == nullptr);
	BOOST_CHECK_THROW(context.read<TestResource>(), std::runtime_error);

	auto written = context.write<TestResource>(42);
	BOOST_TEST(written->value == 42);
	BOOST_TEST(context.has<TestResource>());
	BOOST_TEST(context.try_read<TestResource>() == written);
	BOOST_TEST(context.read<TestResource>() == written);
	BOOST_TEST(context.has_resource<TestResource>());
	BOOST_TEST(context.try_read_resource<TestResource>() == written);
	BOOST_TEST(context.read_resource<TestResource>() == written);

	context.set_module_key(std::type_index(typeid(OtherResource)));
	BOOST_CHECK_THROW(context.write_resource<TestResource>(13), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(context_registers_observers_and_systems)
{
	int lifecycle = 0;

	class ShortcutModule final : public Module
	{
	public:
		explicit ShortcutModule(int& lifecycle) : lifecycle(lifecycle) {}

		void build(App& app) override
		{
			app.add_observer<StartupEvent>([this](const std::shared_ptr<StartupEvent>&) { lifecycle += 1; });
			app.add_system<Update>([this, runtime = app.read_resource<Runtime>()] {
				lifecycle += 10;
				runtime->stop();
			});
			app.add_observer<ShutdownEvent>([this](const std::shared_ptr<ShutdownEvent>&) { lifecycle += 100; });
		}

	private:
		int& lifecycle;
	};

	App app;
	app.add_module<ShortcutModule>(lifecycle);

	BOOST_TEST(app.run() == 0);
	BOOST_TEST(lifecycle == 111);
}

BOOST_AUTO_TEST_CASE(events_invoke_callbacks_in_registration_order)
{
	Events events;
	std::vector<int> calls;

	const auto first = events.add<TestEvent>([&](const std::shared_ptr<TestEvent>& event) {
		calls.push_back(event->value);
	});
	const auto second = events.add<TestEvent>([&](const std::shared_ptr<TestEvent>& event) {
		calls.push_back(event->value + 1);
	});

	auto event = events.invoke<TestEvent>(10);
	BOOST_TEST(event->value == 10);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	BOOST_TEST(calls[0] == 10);
	BOOST_TEST(calls[1] == 11);

	events.remove<TestEvent>(first);
	events.remove<TestEvent>(second);
	calls.clear();

	events.invoke<TestEvent>(20);
	BOOST_TEST(calls.empty());
}

BOOST_AUTO_TEST_CASE(events_ignore_remove_for_missing_id_and_reject_empty_callback)
{
	Events events;

	events.remove<TestEvent>(999);
	BOOST_CHECK_THROW(events.add<TestEvent>({}), std::invalid_argument);

	auto event = events.invoke<TestEvent>(5);
	BOOST_TEST(event->value == 5);
}

BOOST_AUTO_TEST_CASE(events_defer_add_and_remove_until_after_dispatch)
{
	Events events;
	std::vector<int> calls;
	Events::EventId second_id = 0;
	Events::EventId first_id = 0;

	first_id = events.add<TestEvent>([&](const std::shared_ptr<TestEvent>&) {
		calls.push_back(1);
		events.remove<TestEvent>(first_id);
		second_id = events.add<TestEvent>([&](const std::shared_ptr<TestEvent>&) { calls.push_back(2); });
	});

	events.invoke<TestEvent>(1);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	BOOST_TEST(calls[0] == 1);

	calls.clear();
	events.invoke<TestEvent>(1);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	BOOST_TEST(calls[0] == 2);

	events.remove<TestEvent>(second_id);
	calls.clear();
	events.invoke<TestEvent>(1);
	BOOST_TEST(calls.empty());
}

BOOST_AUTO_TEST_CASE(cancellable_events_stop_dispatch_when_cancelled)
{
	Events events;
	int calls = 0;

	events.add<TestCancellableEvent>([&](const std::shared_ptr<TestCancellableEvent>& event) {
		++calls;
		event->set_cancelled(true);
	});
	events.add<TestCancellableEvent>([&](const std::shared_ptr<TestCancellableEvent>&) { ++calls; });

	auto event = events.invoke<TestCancellableEvent>();
	BOOST_TEST(event->is_cancelled());
	BOOST_TEST(calls == 1);
}

BOOST_AUTO_TEST_CASE(startup_event_cancellation_stops_later_startup_observers)
{
	int startup_calls = 0;
	bool update_ran = false;
	bool shutdown_ran = false;

	class CancellingStartupModule final : public Module
	{
	public:
		CancellingStartupModule(int& startup_calls, bool& update_ran, bool& shutdown_ran) :
		    startup_calls(startup_calls), update_ran(update_ran), shutdown_ran(shutdown_ran)
		{}

		void build(App& app) override
		{
			app.add_observer<StartupEvent>([](Context& context, StartupEvent& event) {
				context.read_resource<Runtime>()->exit(42);
				event.set_cancelled(true);
			});
			app.add_observer<StartupEvent>([this] { ++startup_calls; });
			app.add_system<Update>([this] { update_ran = true; });
			app.add_observer<ShutdownEvent>([this] { shutdown_ran = true; });
		}

	private:
		int& startup_calls;
		bool& update_ran;
		bool& shutdown_ran;
	};

	App app;
	app.add_module<CancellingStartupModule>(startup_calls, update_ran, shutdown_ran);

	BOOST_TEST(app.run() == 42);
	BOOST_TEST(startup_calls == 0);
	BOOST_TEST(!update_ran);
	BOOST_TEST(shutdown_ran);
}

BOOST_AUTO_TEST_CASE(systems_run_via_app_and_support_removal)
{
	class SystemsModule final : public Module
	{
	public:
		void build(App& app) override
		{
			auto counter = app.insert_resource<TestResource>();
			auto systems = app.read_resource<Systems>();

			const auto removed = systems->add<Update>([counter] { counter->value += 100; });
			systems->remove<Update>(removed);
			app.add_system<Update>([counter, runtime = app.read_resource<Runtime>()] {
				++counter->value;
				runtime->stop();
			});
		}
	};

	App app;
	app.add_module<SystemsModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(systems_reject_empty_system)
{
	Systems systems;

	BOOST_CHECK_THROW(systems.add<TestSchedule>({}), std::invalid_argument);
	systems.remove<TestSchedule>(123);
}

BOOST_AUTO_TEST_CASE(time_base_advances_wraps_and_rejects_invalid_values)
{
	TestClock<Real> clock;

	BOOST_TEST(clock.elapsed_since_last().count() == 0);
	BOOST_TEST(clock.total_elapsed().count() == 0);
	BOOST_TEST(clock.wrap_interval().count() == 0);
	BOOST_TEST(clock.total_elapsed_wrapped().count() == 0);

	clock.advance_by(10us);
	BOOST_TEST(clock.elapsed_since_last().count() == 10);
	BOOST_TEST(clock.total_elapsed().count() == 10);
	BOOST_TEST(clock.total_elapsed_wrapped().count() == 10);

	clock.set_wrap_period(15us);
	clock.advance_by(10us);
	BOOST_TEST(clock.total_elapsed().count() == 20);
	BOOST_TEST(clock.total_elapsed_wrapped().count() == 5);

	clock.advance_to(25us);
	BOOST_TEST(clock.elapsed_since_last().count() == 5);
	BOOST_TEST(clock.total_elapsed().count() == 25);
	BOOST_TEST(clock.total_elapsed_wrapped().count() == 10);

	BOOST_CHECK_THROW(clock.set_wrap_period(0us), std::invalid_argument);
	BOOST_CHECK_THROW(clock.advance_by(-1us), std::invalid_argument);
	BOOST_CHECK_THROW(clock.advance_to(1us), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(virtual_time_exposes_settings_and_validates_inputs)
{
	Time<Virtual> clock;

	BOOST_TEST(clock.max_delta().count() == 250000);
	BOOST_TEST(clock.relative_speed() == 1.0f);
	BOOST_TEST(!clock.is_paused());

	clock.set_max_delta(100us);
	BOOST_TEST(clock.max_delta().count() == 100);

	clock.set_relative_speed(0.5f);
	BOOST_TEST(clock.relative_speed() == 0.5f);

	clock.pause();
	BOOST_TEST(clock.is_paused());

	clock.unpause();
	BOOST_TEST(!clock.is_paused());

	BOOST_CHECK_THROW(clock.set_max_delta(0us), std::invalid_argument);
	BOOST_CHECK_THROW(clock.set_relative_speed(-0.1f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(fixed_time_exposes_default_interpolation_factor)
{
	Time<Fixed> clock;

	BOOST_TEST(clock.interpolation_factor() == 0.0f);
}

BOOST_AUTO_TEST_CASE(app_registers_core_resources_and_runs_lifecycle_events)
{
	class LifecycleModule final : public Module
	{
	public:
		void build(App& app) override
		{
			BOOST_TEST(app.has_resource<Runtime>());
			BOOST_TEST(app.has_resource<Events>());
			BOOST_TEST(app.has_resource<Systems>());
			BOOST_TEST(app.has_resource<Clock>());
			BOOST_TEST(app.has_resource<Time<Real>>());
			BOOST_TEST(app.has_resource<Time<Virtual>>());
			BOOST_TEST(app.has_resource<Time<Fixed>>());

			auto state = app.insert_resource<TestResource>();

			app.add_observer<StartupEvent>([state, runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) {
				state->value += 1;
				runtime->stop();
			});
			app.add_observer<ShutdownEvent>([state](const std::shared_ptr<ShutdownEvent>&) { state->value += 10; });
		}
	};

	App app;
	app.add_module<LifecycleModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_runs_update_systems_until_runtime_stops)
{
	App app;
	app.add_module<LoopModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_runs_fixed_update_when_fixed_time_accumulates)
{
	class FixedUpdateModule final : public Module
	{
	public:
		void build(App& app) override
		{
			auto fixed_ticks = app.insert_resource<TestResource>();
			auto runtime = app.read_resource<Runtime>();

			app.add_observer<StartupEvent>(
			    [runtime](const std::shared_ptr<StartupEvent>&) { runtime->set_frequency(120); });

			app.add_system<FixedUpdate>([fixed_ticks, runtime] {
				++fixed_ticks->value;
				runtime->stop();
			});
		}
	};

	App app{std::make_shared<ManualClock>(10000us)};
	app.add_module<FixedUpdateModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_advances_real_virtual_and_fixed_time_inside_main_loop)
{
	int fixed_ticks = 0;
	bool update_checked = false;

	class TimeModule final : public Module
	{
	public:
		TimeModule(int& fixed_ticks, bool& update_checked) :
		    fixed_ticks(fixed_ticks), update_checked(update_checked)
		{}

		void build(App& app) override
		{
			auto runtime = app.read_resource<Runtime>();
			auto real_time = app.read_resource<Time<Real>>();
			auto virtual_time = app.read_resource<Time<Virtual>>();
			auto fixed_time = app.read_resource<Time<Fixed>>();

			app.add_observer<StartupEvent>(
			    [runtime](const std::shared_ptr<StartupEvent>&) { runtime->set_frequency(20); });

			app.add_system<FixedUpdate>([this] { ++fixed_ticks; });
			app.add_system<Update>([this, runtime, real_time, virtual_time, fixed_time] {
				BOOST_TEST(fixed_ticks == 1);
				BOOST_TEST(real_time->elapsed_since_last().count() == 50000);
				BOOST_TEST(real_time->total_elapsed().count() == 50000);
				BOOST_TEST(virtual_time->elapsed_since_last().count() == 50000);
				BOOST_TEST(fixed_time->elapsed_since_last().count() == 50000);
				BOOST_TEST(fixed_time->interpolation_factor() == 0.0f);
				update_checked = true;
				runtime->stop();
			});
		}

	private:
		int& fixed_ticks;
		bool& update_checked;
	};

	App app{std::make_shared<ManualClock>(50000us)};
	app.add_module<TimeModule>(fixed_ticks, update_checked);

	BOOST_TEST(app.run() == 0);
	BOOST_TEST(update_checked);
}

BOOST_AUTO_TEST_CASE(app_returns_runtime_exit_code)
{
	class ExitCodeModule final : public Module
	{
	public:
		void build(App& app) override
		{
			app.add_observer<StartupEvent>(
			    [runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) {
				    runtime->exit(9);
			    });
		}
	};

	App app;
	app.add_module<ExitCodeModule>();

	BOOST_TEST(app.run() == 9);
}

BOOST_AUTO_TEST_CASE(app_rejects_null_clock)
{
	BOOST_CHECK_THROW(App{std::shared_ptr<Clock>{}}, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(app_allows_module_resources_to_be_read_by_later_modules)
{
	class ReaderModule final : public Module
	{
	public:
		void build(App& app) override
		{
			BOOST_TEST(app.read_resource<TestResource>()->value == 7);
			app.read_resource<Runtime>()->stop();
		}
	};

	App app;
	app.add_module<StopOnStartupModule>();
	app.add_module<ReaderModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_runs_shutdown_event_when_startup_throws)
{
	bool shutdown_ran = false;

	class ThrowingModule final : public Module
	{
	public:
		explicit ThrowingModule(bool& shutdown_ran) : shutdown_ran(shutdown_ran) {}

		void build(App& app) override
		{
			app.add_observer<StartupEvent>([](const std::shared_ptr<StartupEvent>&) {
				throw std::runtime_error("startup failed");
			});
			app.add_observer<ShutdownEvent>([this](const std::shared_ptr<ShutdownEvent>&) { shutdown_ran = true; });
		}

	private:
		bool& shutdown_ran;
	};

	App app;
	app.add_module<ThrowingModule>(shutdown_ran);

	BOOST_CHECK_THROW(app.run(), std::runtime_error);
	BOOST_TEST(shutdown_ran);
}

BOOST_AUTO_TEST_CASE(app_runs_shutdown_event_when_update_throws)
{
	bool shutdown_ran = false;

	class ThrowingUpdateModule final : public Module
	{
	public:
		explicit ThrowingUpdateModule(bool& shutdown_ran) : shutdown_ran(shutdown_ran) {}

		void build(App& app) override
		{
			app.add_system<Update>([] { throw std::runtime_error("update failed"); });
			app.add_observer<ShutdownEvent>([this](const std::shared_ptr<ShutdownEvent>&) { shutdown_ran = true; });
		}

	private:
		bool& shutdown_ran;
	};

	App app;
	app.add_module<ThrowingUpdateModule>(shutdown_ran);

	BOOST_CHECK_THROW(app.run(), std::runtime_error);
	BOOST_TEST(shutdown_ran);
}

BOOST_AUTO_TEST_CASE(app_rejects_duplicate_inserted_resources)
{
	class DuplicateResourceModule final : public Module
	{
	public:
		void build(App& app) override
		{
			app.insert_resource<TestResource>();
			BOOST_CHECK_THROW(app.insert_resource<TestResource>(), std::invalid_argument);
			app.read_resource<Runtime>()->stop();
		}
	};

	App app;
	app.add_module<DuplicateResourceModule>();

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_rejects_resource_insertion_outside_module_build)
{
	App app;

	BOOST_CHECK_THROW(app.insert_resource<TestResource>(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(app_restores_module_build_state_when_build_throws)
{
	class ThrowingBuildModule final : public Module
	{
	public:
		void build(App&) override { throw std::runtime_error("build failed"); }
	};

	class LaterModule final : public Module
	{
	public:
		void build(App& app) override
		{
			app.insert_resource<TestResource>(17);
			app.read_resource<Runtime>()->stop();
		}
	};

	App app;

	BOOST_CHECK_THROW(app.add_module<ThrowingBuildModule>(), std::runtime_error);
	BOOST_CHECK_THROW(app.insert_resource<OtherResource>(), std::logic_error);

	app.add_module<LaterModule>();
	BOOST_TEST(app.read_resource<TestResource>()->value == 17);
	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_builds_modules_in_order_and_exposes_previous_resources)
{
	std::vector<int> calls;

	class FirstModule final : public Module
	{
	public:
		explicit FirstModule(std::vector<int>& calls) : calls(calls) {}

		void build(App& app) override
		{
			app.insert_resource<TestResource>(23);
			calls.push_back(1);
			app.add_observer<StartupEvent>([this](const std::shared_ptr<StartupEvent>&) { calls.push_back(3); });
		}

	private:
		std::vector<int>& calls;
	};

	class SecondModule final : public Module
	{
	public:
		explicit SecondModule(std::vector<int>& calls) : calls(calls) {}

		void build(App& app) override
		{
			BOOST_TEST(app.read_resource<TestResource>()->value == 23);
			calls.push_back(2);
			app.add_observer<StartupEvent>(
			    [this, runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) {
				    calls.push_back(4);
				    runtime->stop();
			    });
		}

	private:
		std::vector<int>& calls;
	};

	App app;
	app.add_module<FirstModule>(calls).add_module<SecondModule>(calls);

	BOOST_TEST(app.run() == 0);
	BOOST_REQUIRE_EQUAL(calls.size(), 4);
	BOOST_TEST(calls[0] == 1);
	BOOST_TEST(calls[1] == 2);
	BOOST_TEST(calls[2] == 3);
	BOOST_TEST(calls[3] == 4);
}

BOOST_AUTO_TEST_CASE(systems_defer_add_and_remove_until_after_run)
{
	class MutatingSystemsModule final : public Module
	{
	public:
		void build(App& app) override
		{
			auto counter = app.insert_resource<TestResource>();
			auto systems = app.read_resource<Systems>();
			auto runtime = app.read_resource<Runtime>();
			auto second_id = std::make_shared<Systems::SystemId>(0);
			auto first_id = std::make_shared<Systems::SystemId>(0);

			*first_id = systems->add<Update>([counter, systems, first_id, second_id] {
				counter->value += 1;
				systems->remove<Update>(*first_id);
				*second_id = systems->add<Update>([counter] { counter->value += 10; });
			});

			app.add_system<Update>([counter, systems, runtime, second_id] {
				if (counter->value == 11) {
					systems->remove<Update>(*second_id);
				} else if (counter->value >= 21) {
					runtime->stop();
				}
			});
		}
	};

	App app;
	app.add_module<MutatingSystemsModule>();

	BOOST_TEST(app.run() == 0);
	BOOST_TEST(app.read_resource<TestResource>()->value == 21);
}

BOOST_AUTO_TEST_CASE(app_honors_paused_virtual_time_inside_main_loop)
{
	int fixed_ticks = 0;

	class PausedTimeModule final : public Module
	{
	public:
		explicit PausedTimeModule(int& fixed_ticks) : fixed_ticks(fixed_ticks) {}

		void build(App& app) override
		{
			auto runtime = app.read_resource<Runtime>();
			auto virtual_time = app.read_resource<Time<Virtual>>();
			auto fixed_time = app.read_resource<Time<Fixed>>();

			app.add_observer<StartupEvent>([virtual_time](const std::shared_ptr<StartupEvent>&) {
				virtual_time->pause();
			});
			app.add_system<FixedUpdate>([this] { ++fixed_ticks; });
			app.add_system<Update>([this, runtime, virtual_time, fixed_time] {
				BOOST_TEST(fixed_ticks == 0);
				BOOST_TEST(virtual_time->elapsed_since_last().count() == 0);
				BOOST_TEST(fixed_time->interpolation_factor() == 0.0f);
				runtime->stop();
			});
		}

	private:
		int& fixed_ticks;
	};

	App app{std::make_shared<ManualClock>(50000us)};
	app.add_module<PausedTimeModule>(fixed_ticks);

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_honors_scaled_virtual_time_inside_main_loop)
{
	int fixed_ticks = 0;

	class ScaledTimeModule final : public Module
	{
	public:
		explicit ScaledTimeModule(int& fixed_ticks) : fixed_ticks(fixed_ticks) {}

		void build(App& app) override
		{
			auto runtime = app.read_resource<Runtime>();
			auto virtual_time = app.read_resource<Time<Virtual>>();
			auto fixed_time = app.read_resource<Time<Fixed>>();

			app.add_observer<StartupEvent>([virtual_time](const std::shared_ptr<StartupEvent>&) {
				virtual_time->set_relative_speed(0.5f);
			});
			app.add_system<FixedUpdate>([this] { ++fixed_ticks; });
			app.add_system<Update>([this, runtime, virtual_time, fixed_time] {
				BOOST_TEST(fixed_ticks == 0);
				BOOST_TEST(virtual_time->elapsed_since_last().count() == 25000);
				BOOST_TEST(fixed_time->interpolation_factor() == 0.5f);
				runtime->stop();
			});
		}

	private:
		int& fixed_ticks;
	};

	App app{std::make_shared<ManualClock>(50000us)};
	app.add_module<ScaledTimeModule>(fixed_ticks);

	BOOST_TEST(app.run() == 0);
}

BOOST_AUTO_TEST_CASE(app_uses_updated_frequency_on_following_loop_iteration)
{
	int updates = 0;
	int fixed_ticks = 0;

	class DynamicFrequencyModule final : public Module
	{
	public:
		DynamicFrequencyModule(int& updates, int& fixed_ticks) : updates(updates), fixed_ticks(fixed_ticks) {}

		void build(App& app) override
		{
			auto runtime = app.read_resource<Runtime>();

			app.add_observer<StartupEvent>(
			    [runtime](const std::shared_ptr<StartupEvent>&) { runtime->set_frequency(20); });
			app.add_system<FixedUpdate>([this] { ++fixed_ticks; });
			app.add_system<Update>([this, runtime] {
				++updates;
				if (updates == 1) {
					BOOST_TEST(fixed_ticks == 0);
					runtime->set_frequency(100);
				} else {
					BOOST_TEST(fixed_ticks == 2);
					runtime->stop();
				}
			});
		}

	private:
		int& updates;
		int& fixed_ticks;
	};

	App app{std::make_shared<ManualClock>(10000us)};
	app.add_module<DynamicFrequencyModule>(updates, fixed_ticks);

	BOOST_TEST(app.run() == 0);
	BOOST_TEST(updates == 2);
}

BOOST_AUTO_TEST_CASE(app_propagates_shutdown_event_failures_on_clean_shutdown)
{
	class ThrowingShutdownModule final : public Module
	{
	public:
		void build(App& app) override
		{
			app.add_observer<StartupEvent>(
			    [runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) { runtime->stop(); });
			app.add_observer<ShutdownEvent>(
			    [](const std::shared_ptr<ShutdownEvent>&) { throw std::runtime_error("shutdown failed"); });
		}
	};

	App app;
	app.add_module<ThrowingShutdownModule>();

	BOOST_CHECK_THROW(app.run(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(app_preserves_original_error_when_shutdown_also_throws)
{
	class ThrowingStartupAndShutdownModule final : public Module
	{
	public:
		void build(App& app) override
		{
			app.add_observer<StartupEvent>(
			    [](const std::shared_ptr<StartupEvent>&) { throw std::invalid_argument("startup failed"); });
			app.add_observer<ShutdownEvent>(
			    [](const std::shared_ptr<ShutdownEvent>&) { throw std::runtime_error("shutdown failed"); });
		}
	};

	App app;
	app.add_module<ThrowingStartupAndShutdownModule>();

	BOOST_CHECK_THROW(app.run(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(app_exposes_injected_clock_as_base_clock_resource)
{
	auto manual_clock = std::make_shared<ManualClock>(10000us);

	class ClockReaderModule final : public Module
	{
	public:
		explicit ClockReaderModule(const std::shared_ptr<Clock>& expected_clock) : expected_clock(expected_clock) {}

		void build(App& app) override
		{
			BOOST_TEST(app.read_resource<Clock>() == expected_clock);
			app.add_observer<StartupEvent>(
			    [runtime = app.read_resource<Runtime>()](const std::shared_ptr<StartupEvent>&) { runtime->stop(); });
		}

	private:
		std::shared_ptr<Clock> expected_clock;
	};

	App app{manual_clock};
	app.add_module<ClockReaderModule>(manual_clock);

	BOOST_TEST(app.run() == 0);
}
