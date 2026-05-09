// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_RUNTIME_H
#define FS_APPLICATION_RUNTIME_H

#include "resource.h"

#include <cstdint>
#include <stdexcept>

namespace tfs::application {

/**
 * @class Runtime
 * @brief Resource that controls whether the application loop keeps running and how often it ticks.
 */
class Runtime final : public Resource
{
public:
	/**
	 * @brief Creates a running runtime with the default fixed update frequency.
	 */
	Runtime() = default;
	~Runtime() = default;

	Runtime(const Runtime&) = delete;
	Runtime& operator=(const Runtime&) = delete;

	/**
	 * @brief Checks whether the application should continue running.
	 *
	 * @return true when running, false otherwise.
	 */
	bool is_running() const { return running; }

	/**
	 * @brief Stops the application with exit code 0.
	 */
	void stop() { exit(0); }

	/**
	 * @brief Resumes the runtime and resets the exit code.
	 */
	void keep_running()
	{
		running = true;
		code = 0;
	}

	/**
	 * @brief Stops the application with an explicit exit code.
	 *
	 * @param {exit_code} The exit code returned by App::run.
	 */
	void exit(int exit_code)
	{
		code = exit_code;
		running = false;
	}

	/**
	 * @brief Reads the current exit code.
	 *
	 * @return The configured exit code.
	 */
	int exit_code() const { return code; }

	/**
	 * @brief Reads the fixed update frequency.
	 *
	 * @return The frequency in hertz.
	 */
	std::uint16_t frequency() const { return hz; }

	/**
	 * @brief Sets the fixed update frequency.
	 *
	 * @param {frequency} The frequency in hertz. Valid values are 1 through 120.
	 * @throws std::invalid_argument when frequency is zero or above 120.
	 */
	void set_frequency(std::uint16_t frequency)
	{
		if (frequency == 0 || frequency > 120) {
			throw std::invalid_argument("Frequency must be between 1 and 120 Hz");
		}
		hz = frequency;
	}

private:
	bool running = true;
	int code = 0;
	std::uint16_t hz = 20;
};

} // namespace tfs::application

#endif // FS_APPLICATION_RUNTIME_H
