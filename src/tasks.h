// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_TASKS_H
#define FS_TASKS_H

#include "thread_holder_base.h"

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
	void operator()() { func(); }

	void setDontExpire() { expiration = std::chrono::steady_clock::time_point::min(); }

	bool hasExpired() const
	{
		if (expiration == std::chrono::steady_clock::time_point::min()) {
			return false;
		}
		return expiration < std::chrono::steady_clock::now();
	}

protected:
	std::chrono::steady_clock::time_point expiration = std::chrono::steady_clock::time_point::min();

private:
	// Expiration has another meaning for scheduler tasks, then it is the time the task should be added to the
	// dispatcher
	TaskFunc func;
};

std::unique_ptr<Task> createTask(TaskFunc&& f);
std::unique_ptr<Task> createTask(uint32_t expiration, TaskFunc&& f);

class Dispatcher : public ThreadHolder<Dispatcher>
{
public:
	void addTask(std::unique_ptr<Task>&& task);

	void addTask(TaskFunc&& f) { addTask(std::make_unique<Task>(std::move(f))); }

	void addTask(uint32_t expiration, TaskFunc&& f) { addTask(std::make_unique<Task>(expiration, std::move(f))); }

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
