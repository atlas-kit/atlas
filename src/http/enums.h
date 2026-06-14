#pragma once

#include "../game.h"

namespace tfs::http::detail {

int getPvpTypeIndex(WorldType_t worldType);
std::string_view getPvpTypeName(WorldType_t worldType);

} // namespace tfs::http::detail