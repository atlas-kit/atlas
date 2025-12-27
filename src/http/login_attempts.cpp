// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "login_attempts.h"

namespace tfs::http::login_attempts {

namespace {
using namespace std::chrono_literals;
} // namespace

std::shared_mutex mutex;
std::unordered_map<std::string, AttemptInfo> attempts;

static void cleanup_expired_entries()
{
	std::unique_lock writeLock(mutex);
	const auto now = std::chrono::system_clock::now();
	for (auto it = attempts.begin(); it != attempts.end();) {
		const auto& info = it->second;
		if (info.attempts < MAX_ATTEMPTS) {
			if (info.lastAttempt + std::chrono::hours(1) < now) {
				it = attempts.erase(it);
				continue;
			}
		} else {
			if (info.blockUntil <= now) {
				it = attempts.erase(it);
				continue;
			}
		}
		++it;
	}
}

int get_remaining_block_time_minutes(std::string_view ip)
{
	cleanup_expired_entries();

	const std::string key(ip);
	std::shared_lock lock(mutex);
	auto it = attempts.find(key);
	if (it == attempts.end()) {
		return 0;
	}

	const auto info = it->second;
	if (info.attempts < MAX_ATTEMPTS) {
		return 0;
	}

	const auto now = std::chrono::system_clock::now();
	if (now >= info.blockUntil) {
		lock.unlock();
		std::unique_lock writeLock(mutex);
		attempts.erase(key);
		return 0;
	}

	const auto remaining = std::chrono::duration_cast<std::chrono::minutes>(info.blockUntil - now).count();
	return remaining <= 0 ? 1 : static_cast<int>(remaining);
}

void record_failure(std::string_view ip)
{
	cleanup_expired_entries();

	const std::string key(ip);
	std::unique_lock lock(mutex);
	auto it = attempts.find(key);
	if (it == attempts.end()) {
		it = attempts.emplace(key, AttemptInfo{}).first;
	}

	auto& info = it->second;
	info.attempts++;
	info.lastAttempt = std::chrono::system_clock::now();

	if (info.attempts >= MAX_ATTEMPTS) {
		info.blockUntil = info.lastAttempt + BLOCK_DURATION;
	}
}

void record_success(std::string_view ip)
{
	const std::string key(ip);
	std::unique_lock lock(mutex);
	auto it = attempts.find(key);
	if (it != attempts.end()) {
		attempts.erase(it);
	}
}

} // namespace tfs::http::login_attempts
