// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "reactor.h"

TaskReactor g_reactor;

void TaskReactor::send(Closure&& fn)
{
	{
		std::lock_guard<std::mutex> lock(taskLock);
		taskList.push_back({std::move(fn), chrono::steady_clock::time_point::max()});
	}

	taskSignal.notify_one();
}

void TaskReactor::send(chrono::milliseconds expiration, Closure&& fn)
{
	{
		std::lock_guard<std::mutex> lock(taskLock);
		taskList.push_back({std::move(fn), chrono::steady_clock::now() + expiration});
	}

	taskSignal.notify_one();
}

uint32_t TaskReactor::schedule(std::unique_ptr<DelayedTask>&& delayed)
{
	auto id = delayed->getId();
	if (id == 0) {
		id = ++nextId;
		delayed->setId(id);
	} else if (id > nextId) {
		nextId.store(id, std::memory_order_relaxed);
	}

	auto fire_at = chrono::steady_clock::now() + delayed->getDelay();
	auto fn = delayed->extractFunc();

	{
		std::lock_guard<std::mutex> lock(scheduleLock);
		pendingSchedules.emplace_back(id, fire_at, std::move(fn));
	}

	{
		std::lock_guard<std::mutex> lock(taskLock);
		taskSignal.notify_one();
	}

	return id;
}

uint32_t TaskReactor::schedule(chrono::milliseconds delay, Closure&& fn)
{
	auto id = ++nextId;
	auto fire_at = chrono::steady_clock::now() + delay;

	{
		std::lock_guard<std::mutex> lock(scheduleLock);
		pendingSchedules.emplace_back(id, fire_at, std::move(fn));
	}

	{
		std::lock_guard<std::mutex> lock(taskLock);
		taskSignal.notify_one();
	}

	return id;
}

void TaskReactor::cancel(uint32_t taskId)
{
	if (taskId == 0) {
		return;
	}

	{
		std::lock_guard<std::mutex> lock(scheduleLock);
		pendingCancels.push_back(taskId);
	}
}

void TaskReactor::shutdown()
{
	threadState.store(THREAD_STATE_TERMINATED, std::memory_order_relaxed);
	taskSignal.notify_one();
}

void TaskReactor::run()
{
	threadState.store(THREAD_STATE_RUNNING, std::memory_order_relaxed);

	while (getState() != THREAD_STATE_TERMINATED) {
		drain();
	}
}

void TaskReactor::drain()
{
	// 1. Process pending schedules and cancellations
	{
		std::lock_guard<std::mutex> lock(scheduleLock);
		for (auto& [id, fire_at, func] : pendingSchedules) {
			heap.emplace(fire_at, id, std::move(func));
		}
		pendingSchedules.clear();

		for (auto id : pendingCancels) {
			cancelled.insert(id);
		}
		pendingCancels.clear();
	}

	// 2. Pop expired timers
	auto now = chrono::steady_clock::now();
	while (!heap.empty() && heap.top().fire_at <= now) {
		ScheduledTask task = std::move(const_cast<ScheduledTask&>(heap.top()));
		heap.pop();
		if (cancelled.erase(task.taskId) == 0) {
			task.func();
		}
	}

	// 3. Process immediate tasks
	{
		std::vector<ImmediateTask> tmpList;

		{
			std::lock_guard<std::mutex> lock(taskLock);
			tmpList.swap(taskList);
		}

		now = chrono::steady_clock::now();
		for (auto& im : tmpList) {
			if (im.deadline == chrono::steady_clock::time_point::max() || im.deadline > now) {
				im.func();
			}
		}
	}

	// 4. Wait if nothing to do
	if (getState() == THREAD_STATE_TERMINATED) {
		return;
	}

	if (!heap.empty()) {
		auto timeout = chrono::duration_cast<chrono::milliseconds>(heap.top().fire_at - chrono::steady_clock::now());
		if (timeout.count() > 0) {
			std::unique_lock<std::mutex> lock(taskLock);
			if (taskList.empty()) {
				taskSignal.wait_for(lock, timeout);
			}
		}
	} else {
		std::unique_lock<std::mutex> lock(taskLock);
		if (taskList.empty()) {
			taskSignal.wait_for(lock, chrono::milliseconds(100));
		}
	}
}
