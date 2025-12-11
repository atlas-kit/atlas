// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logger {

void init() {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);
}

void shutdown() {
    spdlog::shutdown();
}

} // namespace logger
