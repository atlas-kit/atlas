// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TIMING_TIMER_H
#define FS_APPLICATION_TIMING_TIMER_H

#include <chrono>
#include <cstdint>
#include <stdexcept>

namespace tfs::application {

/**
 * @enum TimerMode
 * @brief Controls whether a timer stops after finishing or repeats.
 */
enum class TimerMode
{
	Once,
	Repeating,
};

/**
 * @class Timer
 * @brief Tracks elapsed duration and reports completion when ticked by frame delta.
 */
class Timer final
{
public:
	/**
	 * @brief Creates a timer with a positive duration.
	 *
	 * @param {duration} The amount of elapsed time required to finish.
	 * @param {mode} Whether the timer stops after finishing or repeats.
	 * @throws std::invalid_argument when duration is zero or negative.
	 */
	explicit Timer(std::chrono::microseconds duration, TimerMode mode = TimerMode::Once) :
	    duration_time{duration}, mode{mode}
	{
		if (duration.count() <= 0) {
			throw std::invalid_argument("Timer duration must be positive");
		}
	}

	/**
	 * @brief Advances the timer by a non-negative delta.
	 *
	 * @param {delta} Time elapsed since the previous tick.
	 * @return This timer, for chaining state checks.
	 */
	Timer& tick(std::chrono::microseconds delta)
	{
		if (delta.count() < 0) {
			throw std::invalid_argument("Timer delta must be non-negative");
		}

		just_finished_count = 0;
		if (paused || delta.count() == 0) {
			return *this;
		}

		elapsed_time += delta;
		if (elapsed_time < duration_time) {
			if (mode == TimerMode::Repeating) {
				finished_state = false;
			}
			return *this;
		}

		if (mode == TimerMode::Once) {
			if (!finished_state) {
				just_finished_count = 1;
			}
			elapsed_time = duration_time;
			finished_state = true;
			return *this;
		}

		just_finished_count = static_cast<uint32_t>(elapsed_time.count() / duration_time.count());
		elapsed_time = std::chrono::microseconds(elapsed_time.count() % duration_time.count());
		finished_state = just_finished_count > 0;
		return *this;
	}

	/**
	 * @brief Resets elapsed time and finish state.
	 */
	void reset()
	{
		elapsed_time = std::chrono::microseconds{0};
		finished_state = false;
		just_finished_count = 0;
	}

	/**
	 * @brief Pauses ticking until unpause is called.
	 */
	void pause() { paused = true; }

	/**
	 * @brief Resumes ticking after a pause.
	 */
	void unpause() { paused = false; }

	/**
	 * @brief Checks whether ticking is paused.
	 *
	 * @return true when paused, false otherwise.
	 */
	bool is_paused() const { return paused; }

	/**
	 * @brief Checks whether the timer is currently finished.
	 *
	 * For repeating timers this is true only on ticks that complete at least one interval.
	 *
	 * @return true when finished, false otherwise.
	 */
	bool finished() const { return finished_state; }

	/**
	 * @brief Checks whether the most recent tick completed the timer.
	 *
	 * @return true when the last tick finished one or more intervals.
	 */
	bool just_finished() const { return just_finished_count > 0; }

	/**
	 * @brief Reads how many intervals were completed by the most recent tick.
	 *
	 * @return The number of completed intervals.
	 */
	uint32_t times_finished_this_tick() const { return just_finished_count; }

	/**
	 * @brief Reads elapsed time within the current interval.
	 *
	 * @return The elapsed time.
	 */
	std::chrono::microseconds elapsed() const { return elapsed_time; }

	/**
	 * @brief Reads the configured duration.
	 *
	 * @return The timer duration.
	 */
	std::chrono::microseconds duration() const { return duration_time; }

	/**
	 * @brief Reads the configured timer mode.
	 *
	 * @return The timer mode.
	 */
	TimerMode timer_mode() const { return mode; }

	/**
	 * @brief Reads the time left before the current interval finishes.
	 *
	 * @return The remaining time, or zero when the timer is finished.
	 */
	std::chrono::microseconds remaining() const
	{
		if (elapsed_time >= duration_time) {
			return std::chrono::microseconds{0};
		}
		return duration_time - elapsed_time;
	}

	/**
	 * @brief Changes the duration and clamps elapsed time to the new value.
	 *
	 * @param {value} The new positive duration.
	 * @throws std::invalid_argument when value is zero or negative.
	 */
	void set_duration(std::chrono::microseconds value)
	{
		if (value.count() <= 0) {
			throw std::invalid_argument("Timer duration must be positive");
		}

		duration_time = value;
		if (elapsed_time > duration_time) {
			elapsed_time = duration_time;
		}
		finished_state = elapsed_time >= duration_time;
		just_finished_count = 0;
	}

	/**
	 * @brief Changes the timer mode without resetting elapsed time.
	 *
	 * @param {value} The new timer mode.
	 */
	void set_mode(TimerMode value)
	{
		mode = value;
		finished_state = elapsed_time >= duration_time;
		just_finished_count = 0;
	}

private:
	std::chrono::microseconds duration_time;
	std::chrono::microseconds elapsed_time{0};
	TimerMode mode;
	bool finished_state = false;
	bool paused = false;
	uint32_t just_finished_count = 0;
};

} // namespace tfs::application

#endif // FS_APPLICATION_TIMING_TIMER_H
