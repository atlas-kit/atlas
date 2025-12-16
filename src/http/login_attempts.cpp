// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "login_attempts.h"

#include <mutex>

namespace tfs::http::login_attempts {

namespace {
using namespace std::chrono_literals;
} // namespace

int get_remaining_block_time_minutes(std::string_view ip)
{
	std::shared_lock lock(mutex);
	auto it = attempts.find(std::string(ip));
	if (it == attempts.end()) {
		return 0;
	}

	const auto& info = it->second;
	if (info.attempts < MAX_ATTEMPTS) {
		return 0;
	}

	auto now = std::chrono::system_clock::now();
	if (now >= info.blockUntil) {
		lock.unlock();
		std::unique_lock writeLock(mutex);
		attempts.erase(it);
		return 0;
	}

	auto remaining = std::chrono::duration_cast<std::chrono::minutes>(info.blockUntil - now).count();
	return remaining <= 0 ? 1 : static_cast<int>(remaining);
}

void record_failure(std::string_view ip)
{
	std::unique_lock lock(mutex);
	auto& info = attempts[std::string(ip)];

	info.attempts++;
	info.lastAttempt = std::chrono::system_clock::now();

	if (info.attempts >= MAX_ATTEMPTS) {
		info.blockUntil = info.lastAttempt + BLOCK_DURATION;
	}
}

void record_success(std::string_view ip)
{
	std::unique_lock lock(mutex);
	attempts.erase(std::string(ip));
}

} // namespace tfs::http::login_attempts
