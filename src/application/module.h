// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_MODULE_H
#define FS_APPLICATION_MODULE_H

namespace tfs::application {

class App;

/**
 * @class Module
 * @brief Represents an application module that registers resources, observers, and systems.
 */
class Module
{
public:
	Module() = default;
	virtual ~Module() = default;

	Module(const Module&) = delete;
	Module& operator=(const Module&) = delete;

	/**
	 * @brief Builds the module into the application.
	 *
	 * @param {app} The application receiving this module's registrations.
	 */
	virtual void build(App& app) = 0;
};

} // namespace tfs::application

#endif // FS_APPLICATION_MODULE_H
