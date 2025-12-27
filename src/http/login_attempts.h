// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

#include <chrono>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace tfs::http::login_attempts {

struct AttemptInfo
{
	int attempts{0};
	std::chrono::system_clock::time_point lastAttempt;
	std::chrono::system_clock::time_point blockUntil;
};

inline constexpr int MAX_ATTEMPTS = 3;
inline constexpr std::chrono::minutes BLOCK_DURATION{30};

extern std::shared_mutex mutex;
extern std::unordered_map<std::string, AttemptInfo> attempts;

int get_remaining_block_time_minutes(std::string_view ip);
void record_failure(std::string_view ip);
void record_success(std::string_view ip);

} // namespace tfs::http::login_attempts
