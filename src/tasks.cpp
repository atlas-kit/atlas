// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "tasks.h"

#include "enums.h"
#include "game.h"

extern Game g_game;

Dispatcher g_dispatcher;

std::unique_ptr<Task> createTask(TaskFunc&& f) { return std::make_unique<Task>(std::move(f)); }

std::unique_ptr<Task> createTask(uint32_t expiration, TaskFunc&& f)
{
	return std::make_unique<Task>(expiration, std::move(f));
}

void Dispatcher::drain()
{
	std::vector<std::unique_ptr<Task>> tmpTaskList;
	{
		std::lock_guard<std::mutex> lockGuard(taskLock);
		tmpTaskList.swap(taskList);
	}

	for (auto& task : tmpTaskList) {
		if (!task->hasExpired()) {
			++dispatcherCycle;
			(*task)();
		}
	}
}

void Dispatcher::threadMain()
{
	// NOTE: second argument defer_lock is to prevent from immediate locking
	std::unique_lock<std::mutex> taskLockUnique(taskLock, std::defer_lock);

	while (getState() != THREAD_STATE_TERMINATED) {
		taskLockUnique.lock();
		if (taskList.empty()) {
			taskSignal.wait(taskLockUnique);
		}
		taskLockUnique.unlock();

		drain();
	}
}

void Dispatcher::addTask(std::unique_ptr<Task>&& task)
{
	bool doSignal = false;

	{
		std::lock_guard<std::mutex> lockGuard(taskLock);
		doSignal = taskList.empty();
		taskList.push_back(std::move(task));
	}

	if (doSignal) {
		taskSignal.notify_one();
	}
}

void Dispatcher::shutdown()
{
	auto task = createTask([this]() {
		setState(THREAD_STATE_TERMINATED);
		taskSignal.notify_one();
	});

	std::lock_guard<std::mutex> lockClass(taskLock);
	taskList.push_back(std::move(task));

	taskSignal.notify_one();
}
