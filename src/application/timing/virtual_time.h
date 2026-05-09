// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TIMING_VIRTUAL_TIME_H
#define FS_APPLICATION_TIMING_VIRTUAL_TIME_H

#include "time.h"

#include <chrono>
#include <stdexcept>

namespace tfs::application {

/**
 * @class Time<Virtual>
 * @brief Time resource that can clamp, scale, or pause real elapsed time.
 */
template <>
class Time<Virtual> final : public TimeBase<Virtual>
{
public:
	Time() = default;
	~Time() override = default;

	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;

	/**
	 * @brief Reads the maximum real delta applied to virtual time.
	 *
	 * @return The maximum delta in microseconds.
	 */
	std::chrono::microseconds max_delta() const { return max_delta_value; }

	/**
	 * @brief Reads the virtual time speed multiplier.
	 *
	 * @return The speed multiplier.
	 */
	float relative_speed() const { return speed; }

	/**
	 * @brief Checks whether virtual time is paused.
	 *
	 * @return true when paused, false otherwise.
	 */
	bool is_paused() const { return paused; }

	/**
	 * @brief Sets the maximum real delta applied to virtual time.
	 *
	 * @param {value} The maximum delta in microseconds.
	 */
	void set_max_delta(std::chrono::microseconds value)
	{
		if (value.count() <= 0) {
			throw std::invalid_argument("Max delta must be positive");
		}
		max_delta_value = value;
	}

	/**
	 * @brief Sets the virtual time speed multiplier.
	 *
	 * @param {value} The new speed multiplier.
	 */
	void set_relative_speed(float value)
	{
		if (value < 0.0f) {
			throw std::invalid_argument("Relative speed must be non-negative");
		}
		speed = value;
	}

	/**
	 * @brief Pauses virtual time advancement.
	 */
	void pause() { paused = true; }

	/**
	 * @brief Resumes virtual time advancement.
	 */
	void unpause() { paused = false; }

protected:
	/**
	 * @brief Advances virtual time from a real delta.
	 *
	 * @param {real_delta} The real elapsed time in microseconds.
	 */
	void update_from_real(std::chrono::microseconds real_delta)
	{
		if (real_delta.count() < 0) {
			throw std::invalid_argument("Delta must be non-negative");
		}

		if (paused) {
			advance_by(std::chrono::microseconds(0));
			return;
		}

		const auto clamped_delta = real_delta > max_delta_value ? max_delta_value : real_delta;
		const auto scaled_delta = std::chrono::duration_cast<std::chrono::microseconds>(clamped_delta * speed);
		advance_by(scaled_delta);
	}

private:
	std::chrono::microseconds max_delta_value{250000};
	float speed = 1.0f;
	bool paused = false;

	friend class App;
};

} // namespace tfs::application

#endif // FS_APPLICATION_TIMING_VIRTUAL_TIME_H
