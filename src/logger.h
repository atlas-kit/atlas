// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_LOGGER_H
#define FS_LOGGER_H

#include <spdlog/spdlog.h>

// Simple wrapper to initialize spdlog
namespace logger {
void init();
void shutdown();
} // namespace logger

#endif // FS_LOGGER_H
