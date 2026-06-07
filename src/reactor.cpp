// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "reactor.h"

TaskReactor g_reactor;

namespace {

struct OrderedCallback
{
	uint64_t sequence;
	Callback function;
};

} // namespace

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

		cancelInbox.push_back(taskIdentifier);

		conditionVariable.notify_one();
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
	const auto now = chrono::steady_clock::now();

	std::vector<OrderedCallback> sendList;
	std::vector<OrderedCallback> heapList;
	std::vector<Callback> callbacks;

	{
		std::lock_guard<std::mutex> lockGuard(mutex);

		// Drain send inbox — preserve submission order (sorted by sequence)
		for (auto& task : sendInbox) {
			if (task.deadline != chrono::steady_clock::time_point::max() && task.deadline <= now) {
				continue;
			}
			sendList.push_back({task.sequence, std::move(task.function)});
		}
		sendInbox.clear();

		// Process cancellation requests
		for (auto identifier : cancelInbox) {
			cancelled.push_back(identifier);
		}
		cancelInbox.clear();

		// Only touch the heap if there is timer work
		if (!taskHeap.empty() || !scheduleInbox.empty()) {
			for (auto& task : scheduleInbox) {
				taskHeap.push_back(std::move(task));
				std::push_heap(taskHeap.begin(), taskHeap.end(), std::greater<>{});
			}
			scheduleInbox.clear();

			while (!taskHeap.empty() && taskHeap.front().fireAt <= now) {
				std::pop_heap(taskHeap.begin(), taskHeap.end(), std::greater<>{});
				Task readyTask = std::move(taskHeap.back());
				taskHeap.pop_back();

				if (readyTask.identifier != 0) {
					auto it = std::find(cancelled.begin(), cancelled.end(), readyTask.identifier);
					if (it != cancelled.end()) {
						*it = cancelled.back();
						cancelled.pop_back();
						continue;
					}
				}

				if (readyTask.deadline != chrono::steady_clock::time_point::max() && readyTask.deadline <= now) {
					continue;
				}

				heapList.push_back({readyTask.sequence, std::move(readyTask.function)});
			}
		}
	}

	// Merge sendList and heapList — both sorted by sequence number
	callbacks.reserve(sendList.size() + heapList.size());
	auto sit = sendList.begin();
	auto hit = heapList.begin();
	while (sit != sendList.end() && hit != heapList.end()) {
		if (sit->sequence < hit->sequence) {
			callbacks.push_back(std::move(sit->function));
			++sit;
		} else {
			callbacks.push_back(std::move(hit->function));
			++hit;
		}
	}

	while (sit != sendList.end()) {
		callbacks.push_back(std::move(sit->function));
		++sit;
	}

	while (hit != heapList.end()) {
		callbacks.push_back(std::move(hit->function));
		++hit;
	}

	// Execute all callbacks in interleaved order
	for (auto& callback : callbacks) {
		callback();
	}

	if (threadState.load(std::memory_order_relaxed) == THREAD_STATE_TERMINATED) {
		return;
	}

	if (taskHeap.empty() && !cancelled.empty()) {
		cancelled.clear();
	}

	waitForWork(now);
}

void TaskReactor::shutdown()
{
	threadState.store(THREAD_STATE_TERMINATED, std::memory_order_relaxed);

	conditionVariable.notify_one();
}

void TaskReactor::waitForWork(chrono::steady_clock::time_point now)
{
	auto wakePredicate = [this]() {
		return threadState.load(std::memory_order_relaxed) == THREAD_STATE_TERMINATED || !sendInbox.empty() ||
		       !scheduleInbox.empty() || !cancelInbox.empty();
	};

	std::unique_lock<std::mutex> uniqueLock(mutex);

	auto timeout = chrono::milliseconds(100);
	if (!taskHeap.empty()) {
		if (taskHeap.front().fireAt > now) {
			timeout = chrono::duration_cast<chrono::milliseconds>(taskHeap.front().fireAt - now);
		} else {
			timeout = chrono::milliseconds::zero();
		}
	}

	conditionVariable.wait_for(uniqueLock, timeout, wakePredicate);
}
