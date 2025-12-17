#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaDBInsertCreate(lua_State* L)
{
	// DBInsert(query)
	if (lua_isstring(L, 2)) {
		tfs::lua::pushUserdata(L, new DBInsert(tfs::lua::getString(L, 2)));
		tfs::lua::setMetatable(L, -1, "DBInsert");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaDBInsertAddRow(lua_State* L)
{
	// insert:addRow(row)
	DBInsert* insert = tfs::lua::getUserdata<DBInsert>(L, 1);
	if (insert) {
		tfs::lua::pushBoolean(L, insert->addRow(tfs::lua::getString(L, 2)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaDBInsertExecute(lua_State* L)
{
	// insert:execute()
	DBInsert* insert = tfs::lua::getUserdata<DBInsert>(L, 1);
	if (insert) {
		tfs::lua::pushBoolean(L, insert->execute());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaDBInsertDelete(lua_State* L)
{
	DBInsert** insertPtr = tfs::lua::getRawUserdata<DBInsert>(L, 1);
	if (insertPtr && *insertPtr) {
		delete *insertPtr;
		*insertPtr = nullptr;
	}
	return 0;
}

} // namespace

void tfs::lua::registerDBInsert(LuaScriptInterface& i)
{
	i.registerClass("DBInsert", "", luaDBInsertCreate);
	i.registerMetaMethod("DBInsert", "__gc", luaDBInsertDelete);

	i.registerMethod("DBInsert", "addRow", luaDBInsertAddRow);
	i.registerMethod("DBInsert", "execute", luaDBInsertExecute);
}
