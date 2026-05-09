// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_RESOURCE_H
#define FS_APPLICATION_RESOURCE_H

namespace tfs::application {

/**
 * @class Resource
 * @brief Base class for shared application state stored in Resources.
 */
class Resource
{
public:
	constexpr Resource() = default;
	virtual ~Resource() = default;

	Resource(const Resource&) = delete;
	Resource& operator=(const Resource&) = delete;
};

} // namespace tfs::application

#endif // FS_APPLICATION_RESOURCE_H
