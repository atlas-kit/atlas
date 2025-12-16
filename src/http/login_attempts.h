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

namespace {
static constexpr int MAX_ATTEMPTS = 3;
static constexpr std::chrono::minutes BLOCK_DURATION{30};

std::shared_mutex mutex;
std::unordered_map<std::string, AttemptInfo> attempts;
} // namespace

int get_remaining_block_time_minutes(std::string_view ip);
void record_failure(std::string_view ip);
void record_success(std::string_view ip);

} // namespace tfs::http::login_attempts
