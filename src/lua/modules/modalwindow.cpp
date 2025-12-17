#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaModalWindowCreate(lua_State* L)
{
	// ModalWindow(id, title, message)
	const std::string& message = tfs::lua::getString(L, 4);
	const std::string& title = tfs::lua::getString(L, 3);
	uint32_t id = tfs::lua::getNumber<uint32_t>(L, 2);

	tfs::lua::pushUserdata(L, new ModalWindow(id, title, message));
	tfs::lua::setMetatable(L, -1, "ModalWindow");
	return 1;
}

int luaModalWindowDelete(lua_State* L)
{
	ModalWindow** windowPtr = tfs::lua::getRawUserdata<ModalWindow>(L, 1);
	if (windowPtr && *windowPtr) {
		delete *windowPtr;
		*windowPtr = nullptr;
	}
	return 0;
}

int luaModalWindowGetId(lua_State* L)
{
	// modalWindow:getId()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushNumber(L, window->id);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetTitle(lua_State* L)
{
	// modalWindow:getTitle()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushString(L, window->title);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetMessage(lua_State* L)
{
	// modalWindow:getMessage()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushString(L, window->message);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSetTitle(lua_State* L)
{
	// modalWindow:setTitle(text)
	const std::string& text = tfs::lua::getString(L, 2);
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->title = text;
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSetMessage(lua_State* L)
{
	// modalWindow:setMessage(text)
	const std::string& text = tfs::lua::getString(L, 2);
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->message = text;
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetButtonCount(lua_State* L)
{
	// modalWindow:getButtonCount()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushNumber(L, window->buttons.size());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetChoiceCount(lua_State* L)
{
	// modalWindow:getChoiceCount()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushNumber(L, window->choices.size());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowAddButton(lua_State* L)
{
	// modalWindow:addButton(id, text)
	const std::string& text = tfs::lua::getString(L, 3);
	uint8_t id = tfs::lua::getNumber<uint8_t>(L, 2);
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->buttons.emplace_back(text, id);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowAddChoice(lua_State* L)
{
	// modalWindow:addChoice(id, text)
	const std::string& text = tfs::lua::getString(L, 3);
	uint8_t id = tfs::lua::getNumber<uint8_t>(L, 2);
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->choices.emplace_back(text, id);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetDefaultEnterButton(lua_State* L)
{
	// modalWindow:getDefaultEnterButton()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushNumber(L, window->defaultEnterButton);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSetDefaultEnterButton(lua_State* L)
{
	// modalWindow:setDefaultEnterButton(buttonId)
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->defaultEnterButton = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowGetDefaultEscapeButton(lua_State* L)
{
	// modalWindow:getDefaultEscapeButton()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushNumber(L, window->defaultEscapeButton);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSetDefaultEscapeButton(lua_State* L)
{
	// modalWindow:setDefaultEscapeButton(buttonId)
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->defaultEscapeButton = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowHasPriority(lua_State* L)
{
	// modalWindow:hasPriority()
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		tfs::lua::pushBoolean(L, window->priority);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSetPriority(lua_State* L)
{
	// modalWindow:setPriority(priority)
	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		window->priority = tfs::lua::getBoolean(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaModalWindowSendToPlayer(lua_State* L)
{
	// modalWindow:sendToPlayer(player)
	const auto& player = tfs::lua::getPlayer(L, 2);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	ModalWindow* window = tfs::lua::getUserdata<ModalWindow>(L, 1);
	if (window) {
		if (!player->hasModalWindowOpen(window->id)) {
			player->sendModalWindow(*window);
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerModalWindow(LuaScriptInterface& i)
{
	i.registerClass("ModalWindow", "", luaModalWindowCreate);
	i.registerMetaMethod("ModalWindow", "__eq", tfs::lua::luaUserdataCompare);
	i.registerMetaMethod("ModalWindow", "__gc", luaModalWindowDelete);
	i.registerMethod("ModalWindow", "delete", luaModalWindowDelete);

	i.registerMethod("ModalWindow", "getId", luaModalWindowGetId);
	i.registerMethod("ModalWindow", "getTitle", luaModalWindowGetTitle);
	i.registerMethod("ModalWindow", "getMessage", luaModalWindowGetMessage);

	i.registerMethod("ModalWindow", "setTitle", luaModalWindowSetTitle);
	i.registerMethod("ModalWindow", "setMessage", luaModalWindowSetMessage);

	i.registerMethod("ModalWindow", "getButtonCount", luaModalWindowGetButtonCount);
	i.registerMethod("ModalWindow", "getChoiceCount", luaModalWindowGetChoiceCount);

	i.registerMethod("ModalWindow", "addButton", luaModalWindowAddButton);
	i.registerMethod("ModalWindow", "addChoice", luaModalWindowAddChoice);

	i.registerMethod("ModalWindow", "getDefaultEnterButton", luaModalWindowGetDefaultEnterButton);
	i.registerMethod("ModalWindow", "setDefaultEnterButton", luaModalWindowSetDefaultEnterButton);

	i.registerMethod("ModalWindow", "getDefaultEscapeButton", luaModalWindowGetDefaultEscapeButton);
	i.registerMethod("ModalWindow", "setDefaultEscapeButton", luaModalWindowSetDefaultEscapeButton);

	i.registerMethod("ModalWindow", "hasPriority", luaModalWindowHasPriority);
	i.registerMethod("ModalWindow", "setPriority", luaModalWindowSetPriority);

	i.registerMethod("ModalWindow", "sendToPlayer", luaModalWindowSendToPlayer);
}
