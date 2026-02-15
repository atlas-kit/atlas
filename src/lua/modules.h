#pragma once

#include "script.h"

namespace tfs::lua::modules {

bool load();
bool reload();
void import(LuaScriptInterface& lsi);

} // namespace tfs::lua::modules
