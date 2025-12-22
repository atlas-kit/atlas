// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "scheduler.h"

extern Dispatcher g_dispatcher;

Scheduler g_scheduler;

uint32_t Scheduler::addEvent(SchedulerTaskPtr task)
{
	// check if the event has a valid id
	if (task->getEventId() == 0) {
		task->setEventId(++lastEventId);
	}

	uint32_t eventId = task->getEventId();

	// Release ownership for async operations - managed manually through callbacks
	SchedulerTask* rawTask = task.release();

	boost::asio::post(io_context, [this, rawTask]() {
		// insert the event id in the list of active events
		auto it = eventIdTimerMap.emplace(rawTask->getEventId(), boost::asio::steady_timer{io_context});
		auto& timer = it.first->second;

		timer.expires_after(std::chrono::milliseconds(rawTask->getDelay()));
		timer.async_wait([this, rawTask](const boost::system::error_code& error) {
			eventIdTimerMap.erase(rawTask->getEventId());

			if (error == boost::asio::error::operation_aborted || getState() == THREAD_STATE_TERMINATED) {
				// the timer has been manually canceled(timer->cancel()) or Scheduler::shutdown has been called
				delete rawTask;
				return;
			}

			// Transfer ownership back to smart pointer for dispatcher
			g_dispatcher.addTask(TaskPtr(rawTask));
		});
	});

	return eventId;
}

void Scheduler::stopEvent(uint32_t eventId)
{
	if (eventId == 0) {
		return;
	}

	boost::asio::post(io_context, [this, eventId]() {
		// search the event id
		auto it = eventIdTimerMap.find(eventId);
		if (it != eventIdTimerMap.end()) {
			it->second.cancel();
		}
	});
}

void Scheduler::shutdown()
{
	setState(THREAD_STATE_TERMINATED);
	boost::asio::post(io_context, [this]() {
		// cancel all active timers
		for (auto&& timer : eventIdTimerMap | std::views::values) {
			timer.cancel();
		}

		io_context.stop();
	});
}

SchedulerTaskPtr createSchedulerTask(uint32_t delay, TaskFunc&& f)
{
	return std::unique_ptr<SchedulerTask>(new SchedulerTask(delay, std::move(f)));
}
