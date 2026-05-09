// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "app.h"

#include "clock.h"
#include "events.h"
#include "resources.h"
#include "runtime.h"
#include "schedule.h"
#include "systems.h"
#include "timing/fixed_time.h"
#include "timing/real_time.h"
#include "timing/time.h"
#include "timing/virtual_time.h"

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>

namespace tfs::application {

App::App() : App(std::make_shared<SystemClock>()) {}

App::App(std::shared_ptr<Clock> clock) :
    runtime{std::make_shared<Runtime>()},
    events{std::make_shared<Events>()},
    resources{std::make_shared<Resources>()},
    systems{std::make_shared<Systems>()},
    clock{std::move(clock)},
    real_clock{std::make_shared<Time<Real>>()},
    virtual_clock{std::make_shared<Time<Virtual>>()},
    fixed_clock{std::make_shared<Time<Fixed>>()}
{
	if (!this->clock) {
		throw std::invalid_argument("Clock resource cannot be null");
	}

	resources->add(runtime);
	resources->add(events);
	resources->add(systems);
	resources->add(this->clock);
	resources->add(real_clock);
	resources->add(virtual_clock);
	resources->add(fixed_clock);
}

int App::run()
{
	auto event = events->invoke<StartupEvent>();
	if (event->is_cancelled()) {
		runtime->stop();
		return runtime->exit_code();
	}

	auto previous_time = clock->now();
	std::chrono::microseconds accumulated{0};

	while (runtime->is_running()) {
		const auto fixed_step = std::chrono::microseconds(1000000 / runtime->frequency());
		auto current_time = clock->now();
		auto frame_delta = std::chrono::duration_cast<std::chrono::microseconds>(current_time - previous_time);
		previous_time = current_time;

		real_clock->advance_by(frame_delta);
		virtual_clock->update_from_real(frame_delta);
		accumulated += virtual_clock->elapsed_since_last();

		while (accumulated >= fixed_step) {
			fixed_clock->advance_by(fixed_step);
			systems->run<FixedUpdate>();
			accumulated -= fixed_step;
		}

		const auto alpha = static_cast<float>(accumulated.count()) / fixed_step.count();
		fixed_clock->set_interpolation_factor(alpha);

		systems->run<Update>();

		if (runtime->is_running()) {
			std::this_thread::sleep_until(current_time + fixed_step);
		}
	}

	events->invoke<ShutdownEvent>();

	return runtime->exit_code();
}

} // namespace tfs::application
