#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaDBTransactionCreate(lua_State* L)
{
	// DBTransaction()
	tfs::lua::pushUserdata(L, new DBTransaction);
	tfs::lua::setMetatable(L, -1, "DBTransaction");
	return 1;
}

int luaDBTransactionBegin(lua_State* L)
{
	// transaction:begin()
	DBTransaction* transaction = tfs::lua::getUserdata<DBTransaction>(L, 1);
	if (transaction) {
		tfs::lua::pushBoolean(L, transaction->begin());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaDBTransactionCommit(lua_State* L)
{
	// transaction:commit()
	DBTransaction* transaction = tfs::lua::getUserdata<DBTransaction>(L, 1);
	if (transaction) {
		tfs::lua::pushBoolean(L, transaction->commit());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaDBTransactionDelete(lua_State* L)
{
	DBTransaction** transactionPtr = tfs::lua::getRawUserdata<DBTransaction>(L, 1);
	if (transactionPtr && *transactionPtr) {
		delete *transactionPtr;
		*transactionPtr = nullptr;
	}
	return 0;
}

} // namespace

void tfs::lua::registerDBTransaction(LuaScriptInterface& lsi)
{
	lsi.registerClass("DBTransaction", "", luaDBTransactionCreate);
	lsi.registerMetaMethod("DBTransaction", "__eq", tfs::lua::luaUserdataCompare);
	lsi.registerMetaMethod("DBTransaction", "__gc", luaDBTransactionDelete);

	lsi.registerMethod("DBTransaction", "begin", luaDBTransactionBegin);
	lsi.registerMethod("DBTransaction", "commit", luaDBTransactionCommit);
	lsi.registerMethod("DBTransaction", "rollback", luaDBTransactionDelete);
}
