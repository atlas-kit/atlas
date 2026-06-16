#pragma once

#include "../game.h"

namespace tfs::http::detail {

std::string_view getPasswordStrengthColor(int strength);
int getPvpTypeIndex(WorldType_t worldType);
std::string_view getPvpTypeName(WorldType_t worldType);

} // namespace tfs::http::detail
