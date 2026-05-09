// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_PENDING_APP_LOOP_EVENTS_H
#define FS_PENDING_APP_LOOP_EVENTS_H

using AppLoopCallback = std::move_only_function<void(void)>;
const int APP_LOOP_EVENT_EXPIRATION = 2000;
const auto APP_LOOP_TIME_ZERO = std::chrono::system_clock::time_point(std::chrono::milliseconds(0));

class AppLoopEvent
{
public:
	explicit AppLoopEvent(AppLoopCallback&& f) : callback(std::move(f)) {}
	AppLoopEvent(uint32_t ms, AppLoopCallback&& f) :
	    expiration(std::chrono::system_clock::now() + std::chrono::milliseconds(ms)), callback(std::move(f))
	{}

	void operator()() { callback(); }

	bool hasExpired() const
	{
		if (expiration == APP_LOOP_TIME_ZERO) {
			return false;
		}
		return expiration < std::chrono::system_clock::now();
	}

private:
	std::chrono::system_clock::time_point expiration = APP_LOOP_TIME_ZERO;
	AppLoopCallback callback;
};

class PendingAppLoopEvents final
{
public:
	void start();
	void push(std::unique_ptr<AppLoopEvent>&& event);
	void push(std::chrono::steady_clock::time_point readyTime, std::unique_ptr<AppLoopEvent>&& event);

	void push(AppLoopCallback&& f) { push(std::make_unique<AppLoopEvent>(std::move(f))); }
	void push(std::chrono::steady_clock::time_point readyTime, AppLoopCallback&& f)
	{
		push(readyTime, std::make_unique<AppLoopEvent>(std::move(f)));
	}

	void push(uint32_t expiration, AppLoopCallback&& f)
	{
		push(std::make_unique<AppLoopEvent>(expiration, std::move(f)));
	}

	void shutdown();
	void stop() { shutdown(); }

	uint64_t getCycle() const { return cycle; }

private:
	struct Entry
	{
		std::chrono::steady_clock::time_point readyTime;
		uint64_t sequence = 0;
		std::unique_ptr<AppLoopEvent> event;
	};

	std::mutex eventLock;

	std::vector<Entry> events;
	bool running = false;
	uint64_t cycle = 0;
	uint64_t sequence = 0;

	void runReadyEvents();

	friend class AppLoop;
};

#endif // FS_PENDING_APP_LOOP_EVENTS_H
