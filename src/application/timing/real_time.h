// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TIMING_REAL_TIME_H
#define FS_APPLICATION_TIMING_REAL_TIME_H

#include "time.h"

namespace tfs::application {

/**
 * @class Time<Real>
 * @brief Time resource advanced directly from the application's clock.
 */
template <>
class Time<Real> final : public TimeBase<Real>
{
public:
	Time() = default;
	~Time() override = default;

	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;

	friend class App;
};

} // namespace tfs::application

#endif // FS_APPLICATION_TIMING_REAL_TIME_H
