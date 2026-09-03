// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_TASKS_H
#define FS_TASKS_H

#include "probes.h"
#include "thread_holder_base.h"

#include <source_location>

using TaskFunc = std::move_only_function<void(void)>;
const int DISPATCHER_TASK_EXPIRATION = 2000;

class Task
{
public:
	explicit Task(TaskFunc&& f) : func(std::move(f)) {}
	Task(uint32_t ms, TaskFunc&& f) :
	    expiration(std::chrono::steady_clock::now() + std::chrono::milliseconds(ms)), func(std::move(f))
	{}

	virtual ~Task() = default;
	void operator()()
	{
		if (ATLAS_TASK_EXECUTION_START_ENABLED())
			ATLAS_TASK_EXECUTION_START(source_loc.file_name(), source_loc.line(), source_loc.function_name());
		func();
		if (ATLAS_TASK_EXECUTION_END_ENABLED())
			ATLAS_TASK_EXECUTION_END(source_loc.file_name(), source_loc.line(), source_loc.function_name());
	}

	void setDontExpire() { expiration = std::chrono::steady_clock::time_point::min(); }

	bool hasExpired() const
	{
		if (expiration == std::chrono::steady_clock::time_point::min()) {
			return false;
		}
		return expiration < std::chrono::steady_clock::now();
	}

	void setSourceLocation([[maybe_unused]] const std::source_location loc) noexcept
	{
#ifdef ENABLE_USDT_PROBES
		source_loc = loc;
#endif
	}

protected:
	std::chrono::steady_clock::time_point expiration = std::chrono::steady_clock::time_point::min();

private:
	// Expiration has another meaning for scheduler tasks, then it is the time the task should be added to the
	// dispatcher
	TaskFunc func;
#ifdef ENABLE_USDT_PROBES
	std::source_location source_loc;
#endif
};

std::unique_ptr<Task> createTask(TaskFunc&& f, const std::source_location loc = std::source_location::current());
std::unique_ptr<Task> createTask(uint32_t expiration, TaskFunc&& f,
                                 const std::source_location loc = std::source_location::current());

class Dispatcher : public ThreadHolder<Dispatcher>
{
public:
	void addTask(std::unique_ptr<Task>&& task);

	void addTask(TaskFunc&& f, const std::source_location loc = std::source_location::current())
	{
		auto task = std::make_unique<Task>(std::move(f));
		if (ATLAS_TASK_EXECUTION_START_ENABLED()) task->setSourceLocation(loc);

		addTask(std::move(task));
	}

	void addTask(uint32_t expiration, TaskFunc&& f, const std::source_location loc = std::source_location::current())
	{
		auto task = std::make_unique<Task>(expiration, std::move(f));
		if (ATLAS_TASK_EXECUTION_START_ENABLED()) task->setSourceLocation(loc);

		addTask(std::move(task));
	}

	void shutdown();

	uint64_t getDispatcherCycle() const { return dispatcherCycle; }

	void threadMain();

private:
	std::mutex taskLock;
	std::condition_variable taskSignal;

	std::vector<std::unique_ptr<Task>> taskList;
	uint64_t dispatcherCycle = 0;
};

#endif // FS_TASKS_H
