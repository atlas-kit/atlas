// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_SCHEDULER_H
#define FS_SCHEDULER_H

#include "tasks.h"
#include "thread_holder_base.h"

inline constexpr auto SCHEDULER_MINTICKS = 50ms;

class SchedulerTask : public Task
{
public:
	SchedulerTask(std::chrono::milliseconds delay, TaskFunc&& f) : Task(std::move(f)), delay(delay) {}

	void setEventId(uint32_t id) { eventId = id; }
	uint32_t getEventId() const { return eventId; }

	auto getDelay() const { return delay; }

private:
	uint32_t eventId = 0;
	std::chrono::milliseconds delay = std::chrono::milliseconds::zero();

	friend std::unique_ptr<SchedulerTask> createSchedulerTask(std::chrono::milliseconds, TaskFunc&&);
};

std::unique_ptr<SchedulerTask> createSchedulerTask(std::chrono::milliseconds delay, TaskFunc&& f);

class Scheduler : public ThreadHolder<Scheduler>
{
public:
	uint32_t addEvent(std::unique_ptr<SchedulerTask>&& task);
	void stopEvent(uint32_t eventId);

	void shutdown();

	void threadMain() { io_context.run(); }

private:
	std::atomic<uint32_t> lastEventId{0};
	std::unordered_map<uint32_t, boost::asio::steady_timer> eventIdTimerMap;
	boost::asio::io_context io_context;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work{io_context.get_executor()};
};

#endif // FS_SCHEDULER_H
