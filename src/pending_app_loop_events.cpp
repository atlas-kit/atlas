// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "pending_app_loop_events.h"

void PendingAppLoopEvents::start()
{
	std::lock_guard<std::mutex> lockClass(eventLock);
	running = true;
}

void PendingAppLoopEvents::runReadyEvents()
{
	while (true) {
		std::unique_ptr<AppLoopEvent> event;

		{
			std::lock_guard<std::mutex> lockClass(eventLock);
			if (events.empty()) {
				break;
			}

			auto nextEvent = std::min_element(events.begin(), events.end(), [](const Entry& lhs, const Entry& rhs) {
				if (lhs.readyTime == rhs.readyTime) {
					return lhs.sequence < rhs.sequence;
				}
				return lhs.readyTime < rhs.readyTime;
			});
			event = std::move(nextEvent->event);
			events.erase(nextEvent);
		}

		if (!event->hasExpired()) {
			++cycle;
			(*event)();
		}
	}
}

void PendingAppLoopEvents::push(std::unique_ptr<AppLoopEvent>&& event)
{
	push(std::chrono::steady_clock::now(), std::move(event));
}

void PendingAppLoopEvents::push(std::chrono::steady_clock::time_point readyTime, std::unique_ptr<AppLoopEvent>&& event)
{
	{
		std::lock_guard<std::mutex> lockClass(eventLock);
		if (!running) {
			return;
		}
		events.push_back(Entry{readyTime, ++sequence, std::move(event)});
	}
}

void PendingAppLoopEvents::shutdown()
{
	std::lock_guard<std::mutex> lockClass(eventLock);
	running = false;
}
