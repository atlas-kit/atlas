// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_REACTOR_H
#define FS_REACTOR_H

#include "enums.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace chrono = std::chrono;

using Closure = std::move_only_function<void(void)>;

inline constexpr auto MIN_TASK_INTERVAL = 50ms;
inline constexpr auto TASK_EXPIRATION = 2000ms;

class DelayedTask
{
public:
	DelayedTask(chrono::milliseconds delay, Closure&& f) : delay(delay), func(std::move(f)) {}

	void setId(uint32_t id) { id_ = id; }
	uint32_t getId() const { return id_; }
	auto getDelay() const { return delay; }
	Closure extractFunc() { return std::move(func); }

private:
	uint32_t id_ = 0;
	chrono::milliseconds delay = chrono::milliseconds::zero();
	Closure func;
};

class TaskReactor
{
public:
	void send(Closure&& fn);
	void send(chrono::milliseconds expiration, Closure&& fn);
	uint32_t schedule(chrono::milliseconds delay, Closure&& fn);
	uint32_t schedule(std::unique_ptr<DelayedTask>&& delayed);
	void cancel(uint32_t taskId);

	void shutdown();
	void run();

	ThreadState getState() const { return threadState.load(std::memory_order_relaxed); }

private:
	void drain();

	struct ImmediateTask
	{
		Closure func;
		chrono::steady_clock::time_point deadline;
	};

	struct ScheduledTask
	{
		chrono::steady_clock::time_point fire_at;
		uint32_t taskId;
		Closure func;

		bool operator>(const ScheduledTask& other) const { return fire_at > other.fire_at; }
	};

	std::mutex taskLock;
	std::condition_variable taskSignal;
	std::vector<ImmediateTask> taskList;

	std::mutex scheduleLock;
	std::vector<std::tuple<uint32_t, chrono::steady_clock::time_point, Closure>> pendingSchedules;
	std::vector<uint32_t> pendingCancels;
	std::unordered_set<uint32_t> cancelled;

	std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<>> heap;
	std::atomic<uint32_t> nextId{0};
	std::atomic<ThreadState> threadState{THREAD_STATE_TERMINATED};
};

#endif // FS_REACTOR_H
