// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "app_loop.h"

AppLoop g_appLoop;

void AppLoop::start()
{
	eventQueue.start();
	running.store(true, std::memory_order_release);
}

void AppLoop::enqueue(AppLoopCallback&& callback)
{
	eventQueue.push(std::move(callback));
}

void AppLoop::enqueue(uint32_t expiration, AppLoopCallback&& callback)
{
	eventQueue.push(expiration, std::move(callback));
}

uint32_t AppLoop::schedule(std::unique_ptr<DelayedAppLoopEvent>&& event)
{
	// check if the event has a valid id
	if (event->getEventId() == 0) {
		event->setEventId(++lastEventId);
	}

	uint32_t eventId = event->getEventId();

	const auto due_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(event->getDelay());
	std::lock_guard<std::mutex> lockClass(eventLock);
	if (running.load(std::memory_order_acquire)) {
		events[eventId] = EventRecord{due_time, ++sequence, std::move(event)};
	}

	return eventId;
}

void AppLoop::cancel(uint32_t eventId)
{
	if (eventId == 0) {
		return;
	}

	std::lock_guard<std::mutex> lockClass(eventLock);
	events.erase(eventId);
}

void AppLoop::shutdown()
{
	running.store(false, std::memory_order_release);
	eventQueue.shutdown();

	std::lock_guard<std::mutex> lockClass(eventLock);
	events.clear();
}

void AppLoop::enqueueReadyEvents()
{
	std::vector<EventRecord> dueEvents;
	const auto now = std::chrono::steady_clock::now();

	{
		std::lock_guard<std::mutex> lockClass(eventLock);
		for (auto it = events.begin(); it != events.end();) {
			if (it->second.due_time > now) {
				++it;
				continue;
			}

			dueEvents.push_back(EventRecord{it->second.due_time, it->second.sequence, std::move(it->second.event)});
			it = events.erase(it);
		}
	}

	std::sort(dueEvents.begin(), dueEvents.end(), [](const EventRecord& lhs, const EventRecord& rhs) {
		if (lhs.due_time == rhs.due_time) {
			return lhs.sequence < rhs.sequence;
		}
		return lhs.due_time < rhs.due_time;
	});

	for (auto& record : dueEvents) {
		eventQueue.push(record.due_time, record.event->releaseCallback());
	}
}

void AppLoop::runReadyEvents()
{
	enqueueReadyEvents();
	eventQueue.runReadyEvents();
}

std::unique_ptr<DelayedAppLoopEvent> createDelayedAppLoopEvent(uint32_t delay, AppLoopCallback&& f)
{
	return std::make_unique<DelayedAppLoopEvent>(delay, std::move(f));
}
