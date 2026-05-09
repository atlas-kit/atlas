// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_SCHEDULE_H
#define FS_APPLICATION_SCHEDULE_H

namespace tfs::application {

/**
 * @struct Schedule
 * @brief Base tag for system execution schedules.
 */
struct Schedule
{
	constexpr Schedule() = default;
	virtual ~Schedule() = default;

	Schedule(const Schedule&) = delete;
	Schedule& operator=(const Schedule&) = delete;
};

/**
 * @struct FixedUpdate
 * @brief Schedule for fixed-timestep systems.
 */
struct FixedUpdate final : Schedule
{
};

/**
 * @struct Update
 * @brief Schedule for frame update systems.
 */
struct Update final : Schedule
{
};

} // namespace tfs::application

#endif // FS_APPLICATION_SCHEDULE_H
