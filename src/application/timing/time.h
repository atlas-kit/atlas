// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TIMING_TIME_H
#define FS_APPLICATION_TIMING_TIME_H

#include "../resource.h"

#include <chrono>
#include <stdexcept>
#include <type_traits>

namespace tfs::application {

/**
 * @struct Fixed
 * @brief Tag type for fixed simulation time.
 */
struct Fixed
{
};

/**
 * @struct Real
 * @brief Tag type for real frame time.
 */
struct Real
{
};

/**
 * @struct Virtual
 * @brief Tag type for scaled and pausable time.
 */
struct Virtual
{
};

/**
 * @class TimeBase
 * @brief Base class that stores elapsed time, frame delta, and optional wrapped elapsed time.
 */
template <typename T>
class TimeBase : public Resource
{
public:
	TimeBase() = default;
	~TimeBase() override = default;

	TimeBase(const TimeBase&) = delete;
	TimeBase& operator=(const TimeBase&) = delete;

	/**
	 * @brief Reads the elapsed time since the previous advance.
	 *
	 * @return The last delta in microseconds.
	 */
	std::chrono::microseconds elapsed_since_last() const { return delta; }

	/**
	 * @brief Reads the total elapsed time.
	 *
	 * @return The total elapsed time in microseconds.
	 */
	std::chrono::microseconds total_elapsed() const { return elapsed; }

	/**
	 * @brief Reads the configured wrap interval.
	 *
	 * @return The wrap interval in microseconds, or zero when wrapping is disabled.
	 */
	std::chrono::microseconds wrap_interval() const { return wrap_period; }

	/**
	 * @brief Reads total elapsed time after applying the wrap interval.
	 *
	 * @return The wrapped elapsed time in microseconds.
	 */
	std::chrono::microseconds total_elapsed_wrapped() const { return elapsed_wrapped; }

protected:
	/**
	 * @brief Sets the wrap interval for total elapsed time.
	 *
	 * @param {period} The wrap interval in microseconds.
	 */
	void set_wrap_period(std::chrono::microseconds period)
	{
		if (period.count() == 0) {
			throw std::invalid_argument("Wrap period cannot be zero");
		}
		wrap_period = period;
	}

	/**
	 * @brief Advances time by a delta.
	 *
	 * @param {value} The delta in microseconds.
	 */
	void advance_by(std::chrono::microseconds value)
	{
		if (value.count() < 0) {
			throw std::invalid_argument("Delta must be non-negative");
		}

		delta = value;
		elapsed += value;

		if (wrap_period.count() > 0) {
			elapsed_wrapped = std::chrono::microseconds(elapsed.count() % wrap_period.count());
		} else {
			elapsed_wrapped = elapsed;
		}
	}

	/**
	 * @brief Advances time to an absolute elapsed value.
	 *
	 * @param {target_elapsed} The target total elapsed time in microseconds.
	 */
	void advance_to(std::chrono::microseconds target_elapsed)
	{
		if (target_elapsed < elapsed) {
			throw std::invalid_argument("Cannot move Time backwards");
		}
		advance_by(target_elapsed - elapsed);
	}

private:
	std::chrono::microseconds delta{0};
	std::chrono::microseconds elapsed{0};
	std::chrono::microseconds wrap_period{0};
	std::chrono::microseconds elapsed_wrapped{0};
};

/**
 * @class Time
 * @brief Typed time resource. Supports only Fixed, Real, and Virtual tags.
 */
template <typename T>
class Time final : public TimeBase<T>
{
	static_assert(std::is_same_v<T, Fixed> || std::is_same_v<T, Real> || std::is_same_v<T, Virtual>,
	              "Time<T> supports only Fixed, Real or Virtual as arguments");

public:
	Time() = default;
	~Time() override = default;

	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;
};

} // namespace tfs::application

#endif // FS_APPLICATION_TIMING_TIME_H
