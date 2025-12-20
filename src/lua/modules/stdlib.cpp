#include "../../otpch.h"

#include "../api.h"
#include "../script.h"
#include "../../tools.h"
#include "../register.h"

namespace {

int luaSystemTime(lua_State* L)
{
	// os.mtime()
	tfs::lua::pushNumber(L, OTSYS_TIME());
	return 1;
}

int luaTableCreate(lua_State* L)
{
	// table.create(arrayLength, keyLength)
	lua_createtable(L, tfs::lua::getNumber<int32_t>(L, 1), tfs::lua::getNumber<int32_t>(L, 2));
	return 1;
}

int luaTablePack(lua_State* L)
{
	// table.pack(...)
	int n = lua_gettop(L);         /* number of elements to pack */
	lua_createtable(L, n, 1);      /* create result table */
	lua_insert(L, 1);              /* put it at index 1 */
	for (int i = n; i >= 1; i--) { /* assign elements */
		lua_rawseti(L, 1, i);
	}
	if (luaL_callmeta(L, -1, "__index") != 0) {
		lua_replace(L, -2);
	}
	tfs::lua::pushNumber(L, n);
	lua_setfield(L, 1, "n"); /* t.n = number of elements */
	return 1;                /* return table */
}

} // namespace

void tfs::lua::registerStdLib(LuaScriptInterface& lsi)
{
	lsi.registerMethod("os", "mtime", luaSystemTime);

	lsi.registerMethod("table", "create", luaTableCreate);
	lsi.registerMethod("table", "pack", luaTablePack);
}
