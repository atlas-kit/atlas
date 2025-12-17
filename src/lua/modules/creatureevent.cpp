#include "../api.h"
#include "../env.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaCreateCreatureEvent(lua_State* L)
{
	// CreatureEvent(eventName)
	if (tfs::lua::getScriptEnv()->getScriptInterface() != &g_scripts->getScriptInterface()) {
		tfs::lua::reportError(L, "CreatureEvents can only be registered in the Scripts interface.");
		lua_pushnil(L);
		return 1;
	}

	CreatureEvent* creatureEvent = new CreatureEvent(tfs::lua::getScriptEnv()->getScriptInterface());
	creatureEvent->setName(tfs::lua::getString(L, 2));
	creatureEvent->fromLua = true;
	tfs::lua::pushUserdata(L, creatureEvent);
	tfs::lua::setMetatable(L, -1, "CreatureEvent");
	return 1;
}

int luaCreatureEventType(lua_State* L)
{
	// creatureevent:type(callback)
	CreatureEvent* creature = tfs::lua::getUserdata<CreatureEvent>(L, 1);
	if (creature) {
		std::string typeName = tfs::lua::getString(L, 2);
		std::string tmpStr = boost::algorithm::to_lower_copy(typeName);
		if (tmpStr == "login") {
			creature->setEventType(CREATURE_EVENT_LOGIN);
		} else if (tmpStr == "logout") {
			creature->setEventType(CREATURE_EVENT_LOGOUT);
		} else if (tmpStr == "reconnect") {
			creature->setEventType(CREATURE_EVENT_RECONNECT);
		} else if (tmpStr == "think") {
			creature->setEventType(CREATURE_EVENT_THINK);
		} else if (tmpStr == "preparedeath") {
			creature->setEventType(CREATURE_EVENT_PREPAREDEATH);
		} else if (tmpStr == "death") {
			creature->setEventType(CREATURE_EVENT_DEATH);
		} else if (tmpStr == "kill") {
			creature->setEventType(CREATURE_EVENT_KILL);
		} else if (tmpStr == "advance") {
			creature->setEventType(CREATURE_EVENT_ADVANCE);
		} else if (tmpStr == "modalwindow") {
			creature->setEventType(CREATURE_EVENT_MODALWINDOW);
		} else if (tmpStr == "textedit") {
			creature->setEventType(CREATURE_EVENT_TEXTEDIT);
		} else if (tmpStr == "healthchange") {
			creature->setEventType(CREATURE_EVENT_HEALTHCHANGE);
		} else if (tmpStr == "manachange") {
			creature->setEventType(CREATURE_EVENT_MANACHANGE);
		} else if (tmpStr == "extendedopcode") {
			creature->setEventType(CREATURE_EVENT_EXTENDED_OPCODE);
		} else {
			std::cout << "[Error - CreatureEvent::configureLuaEvent] Invalid type for creature event: " << typeName
			          << '\n';
			tfs::lua::pushBoolean(L, false);
		}
		creature->setLoaded(true);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaCreatureEventRegister(lua_State* L)
{
	// creatureevent:register()
	CreatureEvent* creature = tfs::lua::getUserdata<CreatureEvent>(L, 1);
	if (creature) {
		if (!creature->isScripted()) {
			tfs::lua::pushBoolean(L, false);
			return 1;
		}
		tfs::lua::pushBoolean(L, g_creatureEvents->registerLuaEvent(creature));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaCreatureEventOnCallback(lua_State* L)
{
	// creatureevent:onLogin / logout / etc. (callback)
	CreatureEvent* creature = tfs::lua::getUserdata<CreatureEvent>(L, 1);
	if (creature) {
		if (!creature->loadCallback()) {
			tfs::lua::pushBoolean(L, false);
			return 1;
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerCreatureEvent(LuaScriptInterface& i)
{
	i.registerClass("CreatureEvent", "", luaCreateCreatureEvent);
	i.registerMethod("CreatureEvent", "type", luaCreatureEventType);
	i.registerMethod("CreatureEvent", "register", luaCreatureEventRegister);
	i.registerMethod("CreatureEvent", "onLogin", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onLogout", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onReconnect", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onThink", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onPrepareDeath", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onDeath", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onKill", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onAdvance", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onModalWindow", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onTextEdit", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onHealthChange", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onManaChange", luaCreatureEventOnCallback);
	i.registerMethod("CreatureEvent", "onExtendedOpcode", luaCreatureEventOnCallback);
}
