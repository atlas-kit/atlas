// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_CLOCK_H
#define FS_APPLICATION_CLOCK_H

#include "resource.h"

#include <chrono>

namespace tfs::application {

/**
 * @class Clock
 * @brief Base resource used by App to read monotonic time.
 */
class Clock : public Resource
{
public:
	/**
	 * @typedef time_point
	 * @brief Represents a steady clock time point.
	 */
	using time_point = std::chrono::steady_clock::time_point;

	/**
	 * @typedef TimePoint
	 * @brief Alias for time_point.
	 */
	using TimePoint = time_point;

	/**
	 * @brief Creates a clock resource.
	 */
	Clock() = default;
	~Clock() override = default;

	Clock(const Clock&) = delete;
	Clock& operator=(const Clock&) = delete;

	/**
	 * @brief Reads the current monotonic time.
	 *
	 * @return The current time point.
	 */
	virtual time_point now() = 0;
};

/**
 * @class SystemClock
 * @brief Clock implementation backed by std::chrono::steady_clock.
 */
class SystemClock final : public Clock
{
public:
	/**
	 * @brief Creates a clock backed by std::chrono::steady_clock.
	 */
	SystemClock() = default;
	~SystemClock() override = default;

	/**
	 * @brief Reads the current steady clock time.
	 *
	 * @return The current steady clock time point.
	 */
	TimePoint now() override { return std::chrono::steady_clock::now(); }
};

} // namespace tfs::application

#endif // FS_APPLICATION_CLOCK_H
