// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_TYPE_NAME_H
#define FS_APPLICATION_TYPE_NAME_H

#include <string_view>
#include <typeinfo>

namespace tfs::application {

/**
 * @brief Gets a readable type name for diagnostics.
 *
 * @return The implementation-defined name of T.
 */
template <typename T>
std::string_view type_name()
{
	return typeid(T).name();
}

} // namespace tfs::application

#endif // FS_APPLICATION_TYPE_NAME_H
