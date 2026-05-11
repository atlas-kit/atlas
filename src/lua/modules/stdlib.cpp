#include "../../otpch.h"

#include "../../tools.h"
#include "../api.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaSystemTime(lua_State* L)
{
	// os.mtime()
	tfs::lua::pushNumber(
	    L, duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	return 1;
}

} // namespace

void tfs::lua::registerStdLib(LuaScriptInterface& lsi) { lsi.registerMethod("os", "mtime", luaSystemTime); }
