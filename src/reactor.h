// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_REACTOR_H
#define FS_REACTOR_H

#include "enums.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace chrono = std::chrono;

using Callback = std::move_only_function<void()>;

inline constexpr auto MIN_TASK_INTERVAL = 50ms;

class TaskReactor
{
public:
	void send(Callback&& callback);
	void send(chrono::milliseconds expirationTime, Callback&& callback);
	uint32_t schedule(chrono::milliseconds delay, Callback&& callback);
	void cancel(uint32_t taskIdentifier);

	void runLoop();
	void runOnce();
	void shutdown();

private:
	struct Task
	{
		chrono::steady_clock::time_point fireAt;
		chrono::steady_clock::time_point deadline;
		uint32_t identifier;
		uint64_t sequence;
		Callback function;

		bool operator>(const Task& other) const
		{
			if (fireAt != other.fireAt) {
				return fireAt > other.fireAt;
			}
			return sequence > other.sequence;
		}
	};

	void waitForWork();

	// Single mutex for all producer inboxes. The reactor thread acquires it
	// only to drain pending tasks into the task heap (microseconds), never
	// during callback execution. Producer threads push in O(1) and release
	// before any application code runs. Lock contention is negligible for
	// this workload.
	std::mutex mutex;
	std::condition_variable conditionVariable;

	std::vector<Task> sendInbox;
	std::vector<Task> scheduleInbox;
	std::vector<uint32_t> cancelInbox;

	std::unordered_set<uint32_t> cancelled;
	std::unordered_set<uint32_t> activeIdentifiers;
	std::vector<Task> taskHeap;

	std::atomic<uint32_t> nextIdentifier{0};
	std::atomic<uint64_t> nextSequence{0};
	std::atomic<ThreadState> threadState{THREAD_STATE_TERMINATED};
};

#endif // FS_REACTOR_H
