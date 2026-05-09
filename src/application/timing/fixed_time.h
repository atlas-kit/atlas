// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TIMING_FIXED_TIME_H
#define FS_APPLICATION_TIMING_FIXED_TIME_H

#include "time.h"

namespace tfs::application {

/**
 * @class Time<Fixed>
 * @brief Time resource for fixed-step simulation updates.
 */
template <>
class Time<Fixed> final : public TimeBase<Fixed>
{
public:
	Time() = default;
	~Time() override = default;

	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;

	/**
	 * @brief Reads the interpolation factor between fixed updates.
	 *
	 * @return A value from 0.0 to 1.0 representing leftover accumulated time.
	 */
	float interpolation_factor() const { return alpha; }

protected:
	/**
	 * @brief Sets the interpolation factor between fixed updates.
	 *
	 * @param {factor} The interpolation factor.
	 */
	void set_interpolation_factor(float factor) { alpha = factor; }

private:
	float alpha = 0.0f;

	friend class App;
};

} // namespace tfs::application

#endif // FS_APPLICATION_TIMING_FIXED_TIME_H
