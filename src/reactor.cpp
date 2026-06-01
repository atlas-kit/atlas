// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "reactor.h"

TaskReactor g_reactor;

void TaskReactor::send(Callback&& callback)
{
	const auto sequence = ++nextSequence;

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		const auto now = chrono::steady_clock::now();
		sendInbox.push_back(Task{now, chrono::steady_clock::time_point::max(), 0, sequence, std::move(callback)});

		conditionVariable.notify_one();
	}
}

void TaskReactor::send(chrono::milliseconds expirationTime, Callback&& callback)
{
	const auto sequence = ++nextSequence;

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		const auto now = chrono::steady_clock::now();
		sendInbox.push_back(Task{now, now + expirationTime, 0, sequence, std::move(callback)});

		conditionVariable.notify_one();
	}
}

uint32_t TaskReactor::schedule(chrono::milliseconds delay, Callback&& callback)
{
	const auto identifier = ++nextIdentifier;
	const auto sequence = ++nextSequence;

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		const auto fireAt = chrono::steady_clock::now() + delay;
		activeIdentifiers.insert(identifier);
		scheduleInbox.push_back(
		    Task{fireAt, chrono::steady_clock::time_point::max(), identifier, sequence, std::move(callback)});

		conditionVariable.notify_one();
	}

	return identifier;
}

void TaskReactor::cancel(uint32_t taskIdentifier)
{
	if (taskIdentifier == 0) {
		return;
	}

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		if (activeIdentifiers.find(taskIdentifier) != activeIdentifiers.end()) {
			cancelInbox.push_back(taskIdentifier);
		}
	}
}

void TaskReactor::runLoop()
{
	threadState.store(THREAD_STATE_RUNNING, std::memory_order_relaxed);

	while (threadState.load(std::memory_order_relaxed) != THREAD_STATE_TERMINATED) {
		runOnce();
	}
}

void TaskReactor::runOnce()
{
	std::vector<Callback> readyCallbacks;

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		const auto now = chrono::steady_clock::now();

		// Move send inbox tasks into the heap
		for (auto& task : sendInbox) {
			taskHeap.push_back(std::move(task));
			std::push_heap(taskHeap.begin(), taskHeap.end(), std::greater<>{});
		}
		sendInbox.clear();

		// Move schedule inbox tasks into the heap
		for (auto& task : scheduleInbox) {
			taskHeap.push_back(std::move(task));
			std::push_heap(taskHeap.begin(), taskHeap.end(), std::greater<>{});
		}
		scheduleInbox.clear();

		// Process cancellation requests
		for (auto identifier : cancelInbox) {
			if (activeIdentifiers.find(identifier) != activeIdentifiers.end()) {
				cancelled.insert(identifier);
			}
		}
		cancelInbox.clear();

		// Pop expired tasks from the heap
		while (!taskHeap.empty() && taskHeap.front().fireAt <= now) {
			std::pop_heap(taskHeap.begin(), taskHeap.end(), std::greater<>{});
			Task readyTask = std::move(taskHeap.back());
			taskHeap.pop_back();

			// Remove from active identifiers and check cancellation
			if (readyTask.identifier != 0) {
				activeIdentifiers.erase(readyTask.identifier);
				if (cancelled.erase(readyTask.identifier) != 0) {
					continue;
				}
			}

			// Check deadline expiration
			const auto deadlineReached =
			    readyTask.deadline != chrono::steady_clock::time_point::max() && readyTask.deadline <= now;
			if (deadlineReached) {
				continue;
			}

			readyCallbacks.push_back(std::move(readyTask.function));
		}
	}

	// Execute all ready callbacks outside the lock
	for (auto& callback : readyCallbacks) {
		callback();
	}

	// Stop if shutdown was requested
	if (threadState.load(std::memory_order_relaxed) == THREAD_STATE_TERMINATED) {
		return;
	}

	waitForWork();
}

void TaskReactor::shutdown()
{
	threadState.store(THREAD_STATE_TERMINATED, std::memory_order_relaxed);

	conditionVariable.notify_one();
}

void TaskReactor::waitForWork()
{
	auto timeout = chrono::milliseconds(100);

	if (!taskHeap.empty()) {
		const auto now = chrono::steady_clock::now();
		if (taskHeap.front().fireAt > now) {
			timeout = chrono::duration_cast<chrono::milliseconds>(taskHeap.front().fireAt - now);
		} else {
			timeout = chrono::milliseconds::zero();
		}
	}

	auto wakePredicate = [this]() {
		return threadState.load(std::memory_order_relaxed) == THREAD_STATE_TERMINATED || !sendInbox.empty() ||
		       !scheduleInbox.empty() || !cancelInbox.empty();
	};

	std::unique_lock<std::mutex> uniqueLock(mutex);

	conditionVariable.wait_for(uniqueLock, timeout, wakePredicate);
}
