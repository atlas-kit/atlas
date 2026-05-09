// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APP_LOOP_H
#define FS_APP_LOOP_H

#include "pending_app_loop_events.h"

static constexpr int32_t APP_LOOP_MINTICKS = 50;

namespace tfs::modules::app_loop {

class AppLoopModule;

} // namespace tfs::modules::app_loop

class DelayedAppLoopEvent
{
public:
	DelayedAppLoopEvent(uint32_t delay, AppLoopCallback&& f) : delay(delay), callback(std::move(f)) {}

	void setEventId(uint32_t id) { eventId = id; }
	uint32_t getEventId() const { return eventId; }

	uint32_t getDelay() const { return delay; }
	AppLoopCallback releaseCallback() { return std::move(callback); }

private:
	uint32_t eventId = 0;
	uint32_t delay = 0;
	AppLoopCallback callback;

	friend std::unique_ptr<DelayedAppLoopEvent> createDelayedAppLoopEvent(uint32_t, AppLoopCallback&&);
};

std::unique_ptr<DelayedAppLoopEvent> createDelayedAppLoopEvent(uint32_t delay, AppLoopCallback&& f);

class AppLoop
{
public:
	void start();
	void enqueue(AppLoopCallback&& callback);
	void enqueue(uint32_t expiration, AppLoopCallback&& callback);
	uint32_t schedule(std::unique_ptr<DelayedAppLoopEvent>&& event);
	void cancel(uint32_t eventId);

	void shutdown();

private:
	struct EventRecord
	{
		std::chrono::steady_clock::time_point due_time;
		uint64_t sequence = 0;
		std::unique_ptr<DelayedAppLoopEvent> event;
	};

	std::atomic<uint32_t> lastEventId{0};
	std::atomic_bool running{false};
	std::mutex eventLock;
	std::unordered_map<uint32_t, EventRecord> events;
	PendingAppLoopEvents eventQueue;
	uint64_t sequence = 0;

	void enqueueReadyEvents();
	void runReadyEvents();

	friend class tfs::modules::app_loop::AppLoopModule;
};

extern AppLoop g_appLoop;

#endif // FS_APP_LOOP_H
