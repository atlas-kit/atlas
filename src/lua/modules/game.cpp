#include "../../otpch.h"

#include "../../game.h"

#include "../../actions.h"
#include "../../chat.h"
#include "../../configmanager.h"
#include "../../events/events.h"
#include "../../events/monster.h"
#include "../../monster.h"
#include "../../movement.h"
#include "../../npc.h"
#include "../../script.h"
#include "../../spells.h"
#include "../../talkaction.h"
#include "../../tasks.h"
#include "../../weapons.h"
#include "../api.h"
#include "../env.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

extern Actions* g_actions;
extern Chat g_chat;
extern Game g_game;
extern LuaEnvironment g_luaEnvironment;
extern MoveEvents* g_moveEvents;
extern Spells* g_spells;
extern Monsters g_monsters;
extern Scripts* g_scripts;
extern Dispatcher g_dispatcher;
extern TalkActions* g_talkActions;
extern Vocations g_vocations;
extern std::unique_ptr<Weapons> g_weapons;

namespace {

int luaGameGetSpectators(lua_State* L)
{
	// Game.getSpectators(position[, multifloor = false[, onlyPlayer = false[, minRangeX = 0[, maxRangeX = 0[, minRangeY
	// = 0[, maxRangeY = 0]]]]]])
	const Position& position = tfs::lua::getPosition(L, 1);
	bool multifloor = tfs::lua::getBoolean(L, 2, false);
	bool onlyPlayers = tfs::lua::getBoolean(L, 3, false);
	int32_t minRangeX = tfs::lua::getNumber<int32_t>(L, 4, 0);
	int32_t maxRangeX = tfs::lua::getNumber<int32_t>(L, 5, 0);
	int32_t minRangeY = tfs::lua::getNumber<int32_t>(L, 6, 0);
	int32_t maxRangeY = tfs::lua::getNumber<int32_t>(L, 7, 0);

	SpectatorVec spectators;
	g_game.map.getSpectators(spectators, position, multifloor, onlyPlayers, minRangeX, maxRangeX, minRangeY, maxRangeY);

	lua_createtable(L, spectators.size(), 0);

	int index = 0;
	for (const auto& creature : spectators) {
		tfs::lua::pushSharedPtr(L, creature);
		tfs::lua::setCreatureMetatable(L, -1, creature);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetPlayers(lua_State* L)
{
	// Game.getPlayers()
	const auto& players = g_game.getPlayers() | tfs::views::lock_weak_ptrs | std::ranges::to<std::vector>();
	lua_createtable(L, players.size(), 0);

	int index = 0;
	for (const auto& player : players) {
		tfs::lua::pushSharedPtr(L, player);
		tfs::lua::setMetatable(L, -1, "Player");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetNpcs(lua_State* L)
{
	// Game.getNpcs()
	const auto& npcs = g_game.getNpcs() | tfs::views::lock_weak_ptrs | std::ranges::to<std::vector>();
	lua_createtable(L, npcs.size(), 0);

	int index = 0;
	for (const auto& npc : npcs) {
		tfs::lua::pushSharedPtr(L, npc);
		tfs::lua::setMetatable(L, -1, "Npc");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetMonsters(lua_State* L)
{
	// Game.getMonsters()
	const auto& monsters = g_game.getMonsters() | tfs::views::lock_weak_ptrs | std::ranges::to<std::vector>();
	lua_createtable(L, monsters.size(), 0);

	int index = 0;
	for (const auto& monster : monsters) {
		tfs::lua::pushSharedPtr(L, monster);
		tfs::lua::setMetatable(L, -1, "Monster");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameLoadMap(lua_State* L)
{
	// Game.loadMap(path)
	const std::string& path = tfs::lua::getString(L, 1);
	g_dispatcher.addTask([path]() {
		try {
			g_game.loadMap(path, true);
		} catch (const std::invalid_argument& e) {
			std::cout << "[Error - luaGameLoadMap] Failed to load map: " << e.what() << '\n';
		}
	});
	return 0;
}

int luaGameGetExperienceStage(lua_State* L)
{
	// Game.getExperienceStage(level)
	uint32_t level = tfs::lua::getNumber<uint32_t>(L, 1);
	tfs::lua::pushNumber(L, ConfigManager::getExperienceStage(level));
	return 1;
}

int luaGameGetExperienceForLevel(lua_State* L)
{
	// Game.getExperienceForLevel(level)
	const uint32_t level = tfs::lua::getNumber<uint32_t>(L, 1);
	if (level == 0) {
		tfs::lua::pushNumber(L, 0);
	} else {
		tfs::lua::pushNumber(L, Player::getExpForLevel(level));
	}
	return 1;
}

int luaGameGetMonsterCount(lua_State* L)
{
	// Game.getMonsterCount()
	tfs::lua::pushNumber(L, g_game.getMonstersOnline());
	return 1;
}

int luaGameGetPlayerCount(lua_State* L)
{
	// Game.getPlayerCount()
	tfs::lua::pushNumber(L, g_game.getPlayersOnline());
	return 1;
}

int luaGameGetNpcCount(lua_State* L)
{
	// Game.getNpcCount()
	tfs::lua::pushNumber(L, g_game.getNpcsOnline());
	return 1;
}

int luaGameGetMonsterTypes(lua_State* L)
{
	// Game.getMonsterTypes()
	auto& type = g_monsters.monsters;
	lua_createtable(L, type.size(), 0);

	for (const auto& [name, mType] : type) {
		tfs::lua::pushUserdata(L, &mType);
		tfs::lua::setMetatable(L, -1, "MonsterType");
		lua_setfield(L, -2, name.data());
	}
	return 1;
}

int luaGameGetBestiary(lua_State* L)
{
	// Game.getBestiary()
	lua_createtable(L, 0, g_monsters.bestiary.size());
	int classIndex = 0;
	for (const auto& [className, monsters] : g_monsters.bestiary) {
		lua_createtable(L, 0, 2);
		tfs::lua::pushString(L, className);
		lua_setfield(L, -2, "name");

		lua_createtable(L, 0, monsters.size());
		int index = 0;
		for (const auto& monsterName : monsters) {
			tfs::lua::pushUserdata(L, g_monsters.getMonsterType(monsterName));
			tfs::lua::setMetatable(L, -1, "MonsterType");
			lua_rawseti(L, -2, ++index);
		}

		lua_setfield(L, -2, "monsterTypes");
		lua_rawseti(L, -2, ++classIndex);
	}
	return 1;
}

int luaGameGetCurrencyItems(lua_State* L)
{
	// Game.getCurrencyItems()
	const auto& currencyItems = Item::items.currencyItems;
	size_t size = currencyItems.size();
	lua_createtable(L, size, 0);

	for (auto&& itemId : currencyItems | std::views::values | std::views::as_const) {
		const ItemType& itemType = Item::items[itemId];
		tfs::lua::pushUserdata(L, &itemType);
		tfs::lua::setMetatable(L, -1, "ItemType");
		lua_rawseti(L, -2, size--);
	}
	return 1;
}

int luaGameGetItemTypeByClientId(lua_State* L)
{
	// Game.getItemTypeByClientId(clientId)
	uint16_t spriteId = tfs::lua::getNumber<uint16_t>(L, 1);
	const ItemType& itemType = Item::items.getItemIdByClientId(spriteId);
	if (itemType.id != 0) {
		tfs::lua::pushUserdata(L, &itemType);
		tfs::lua::setMetatable(L, -1, "ItemType");
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int luaGameGetParties(lua_State* L)
{
	// Game.getParties()
	const auto& parties = g_game.getParties();
	lua_createtable(L, parties.size(), 0);

	int index = 0;
	for (const auto& party : parties) {
		tfs::lua::pushSharedPtr(L, party);
		tfs::lua::setMetatable(L, -1, "Party");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetTowns(lua_State* L)
{
	// Game.getTowns()
	const auto& towns = g_game.map.towns.getTowns();
	lua_createtable(L, towns.size(), 0);

	int index = 0;
	for (const auto& town : towns | std::views::values) {
		tfs::lua::pushTown(L, *town);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetHouses(lua_State* L)
{
	// Game.getHouses()
	const auto& houses = g_game.getHouses() | std::ranges::to<std::vector>();
	lua_createtable(L, houses.size(), 0);

	int index = 0;
	for (auto&& house : houses | std::views::as_const) {
		tfs::lua::pushSharedPtr(L, house);
		tfs::lua::setMetatable(L, -1, "House");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaGameGetVocations(lua_State* L)
{
	// Game.getVocations()
	const auto& vocations = g_vocations.getVocations();
	lua_createtable(L, vocations.size(), 0);

	int index = 0;
	for (const auto& [id, vocation] : vocations) {
		tfs::lua::pushUserdata(L, &vocation);
		tfs::lua::setMetatable(L, -1, "Vocation");
		lua_rawseti(L, -2, ++index);
	}

	return 1;
}

int luaGameGetRuneSpells(lua_State* L)
{
	// Game.getRuneSpells()
	const auto& runeSpells = g_spells->getRuneSpells();

	lua_createtable(L, runeSpells.size(), 0);

	int index = 0;
	for (const auto& spell : runeSpells | std::views::values) {
		tfs::lua::pushUserdata<const Spell>(L, &spell);
		tfs::lua::setMetatable(L, -1, "Spell");
		lua_rawseti(L, -2, ++index);
	}

	return 1;
}

int luaGameGetInstantSpells(lua_State* L)
{
	// Game.getInstantSpells()
	const auto& instantSpells = g_spells->getInstantSpells();

	lua_createtable(L, instantSpells.size(), 0);

	int index = 0;
	for (const auto& spell : instantSpells | std::views::values) {
		tfs::lua::pushUserdata<const Spell>(L, &spell);
		tfs::lua::setMetatable(L, -1, "Spell");
		lua_rawseti(L, -2, ++index);
	}

	return 1;
}

int luaGameGetGameState(lua_State* L)
{
	// Game.getGameState()
	tfs::lua::pushNumber(L, g_game.getGameState());
	return 1;
}

int luaGameSetGameState(lua_State* L)
{
	// Game.setGameState(state)
	GameState_t state = tfs::lua::getNumber<GameState_t>(L, 1);
	g_game.setGameState(state);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameGetWorldType(lua_State* L)
{
	// Game.getWorldType()
	tfs::lua::pushNumber(L, g_game.getWorldType());
	return 1;
}

int luaGameSetWorldType(lua_State* L)
{
	// Game.setWorldType(type)
	WorldType_t type = tfs::lua::getNumber<WorldType_t>(L, 1);
	g_game.setWorldType(type);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameGetReturnMessage(lua_State* L)
{
	// Game.getReturnMessage(value)
	ReturnValue value = tfs::lua::getNumber<ReturnValue>(L, 1);
	tfs::lua::pushString(L, getReturnMessage(value));
	return 1;
}

int luaGameGetItemAttributeByName(lua_State* L)
{
	// Game.getItemAttributeByName(name)
	tfs::lua::pushNumber(L, stringToItemAttribute(tfs::lua::getString(L, 1)));
	return 1;
}

int luaGameCreateItem(lua_State* L)
{
	// Game.createItem(itemId[, count[, position]])
	uint16_t count = tfs::lua::getNumber<uint16_t>(L, 2, 1);
	uint16_t id;
	if (tfs::lua::isNumber(L, 1)) {
		id = tfs::lua::getNumber<uint16_t>(L, 1);
	} else {
		id = Item::items.getItemIdByName(tfs::lua::getString(L, 1));
		if (id == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	const ItemType& it = Item::items[id];
	if (it.stackable) {
		count = std::min<uint16_t>(count, ITEM_STACK_SIZE);
	}

	const auto& item = Item::CreateItem(id, count);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) >= 3) {
		const Position& position = tfs::lua::getPosition(L, 3);
		const auto& tile = g_game.map.getTile(position);
		if (!tile) {
			lua_pushnil(L);
			return 1;
		}

		g_game.internalAddItem(tile, item, INDEX_WHEREEVER, FLAG_NOLIMIT);
	}

	tfs::lua::pushSharedPtr(L, item);
	tfs::lua::setItemMetatable(L, -1, item);
	return 1;
}

int luaGameCreateContainer(lua_State* L)
{
	// Game.createContainer(itemId, size[, position])
	uint16_t size = tfs::lua::getNumber<uint16_t>(L, 2);
	uint16_t id;
	if (tfs::lua::isNumber(L, 1)) {
		id = tfs::lua::getNumber<uint16_t>(L, 1);
	} else {
		id = Item::items.getItemIdByName(tfs::lua::getString(L, 1));
		if (id == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	const auto& container = Item::CreateItemAsContainer(id, size);
	if (!container) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) >= 3) {
		const Position& position = tfs::lua::getPosition(L, 3);
		const auto& tile = g_game.map.getTile(position);
		if (!tile) {
			lua_pushnil(L);
			return 1;
		}

		g_game.internalAddItem(tile, container, INDEX_WHEREEVER, FLAG_NOLIMIT);
	}

	tfs::lua::pushSharedPtr(L, container);
	tfs::lua::setMetatable(L, -1, "Container");
	return 1;
}

int luaGameCreateMonster(lua_State* L)
{
	// Game.createMonster(monsterName, position[, extended = false[, force = false[, magicEffect = CONST_ME_TELEPORT]]])
	const auto& monster = Monster::createMonster(tfs::lua::getString(L, 1));
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	const Position& position = tfs::lua::getPosition(L, 2);
	bool extended = tfs::lua::getBoolean(L, 3, false);
	bool force = tfs::lua::getBoolean(L, 4, false);
	MagicEffectClasses magicEffect = tfs::lua::getNumber<MagicEffectClasses>(L, 5, CONST_ME_TELEPORT);
	if (tfs::events::monster::onSpawn(monster, position, false, true) || force) {
		if (g_game.placeCreature(monster, position, extended, force, magicEffect)) {
			tfs::lua::pushSharedPtr(L, monster);
			tfs::lua::setMetatable(L, -1, "Monster");
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaGameCreateNpc(lua_State* L)
{
	// Game.createNpc(npcName, position[, extended = false[, force = false[, magicEffect = CONST_ME_TELEPORT]]])
	const auto& npc = Npc::createNpc(tfs::lua::getString(L, 1));
	if (!npc) {
		lua_pushnil(L);
		return 1;
	}

	const Position& position = tfs::lua::getPosition(L, 2);
	bool extended = tfs::lua::getBoolean(L, 3, false);
	bool force = tfs::lua::getBoolean(L, 4, false);
	MagicEffectClasses magicEffect = tfs::lua::getNumber<MagicEffectClasses>(L, 5, CONST_ME_TELEPORT);
	if (g_game.placeCreature(npc, position, extended, force, magicEffect)) {
		tfs::lua::pushSharedPtr(L, npc);
		tfs::lua::setMetatable(L, -1, "Npc");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaGameCreateTile(lua_State* L)
{
	// Game.createTile(x, y, z[, isDynamic = false])
	// Game.createTile(position[, isDynamic = false])
	Position position;
	bool isDynamic;
	if (lua_istable(L, 1)) {
		position = tfs::lua::getPosition(L, 1);
		isDynamic = tfs::lua::getBoolean(L, 2, false);
	} else {
		position.x = tfs::lua::getNumber<uint16_t>(L, 1);
		position.y = tfs::lua::getNumber<uint16_t>(L, 2);
		position.z = tfs::lua::getNumber<uint16_t>(L, 3);
		isDynamic = tfs::lua::getBoolean(L, 4, false);
	}

	auto tile = g_game.map.getTile(position);
	if (!tile) {
		if (isDynamic) {
			tile = std::make_shared<DynamicTile>(position.x, position.y, position.z);
		} else {
			tile = std::make_shared<StaticTile>(position.x, position.y, position.z);
		}

		g_game.map.setTile(position, tile);
	}

	tfs::lua::pushSharedPtr(L, tile);
	tfs::lua::setMetatable(L, -1, "Tile");
	return 1;
}

int luaGameCreateMonsterType(lua_State* L)
{
	// Game.createMonsterType(name)
	if (tfs::lua::getScriptEnv()->getScriptInterface() != &g_scripts->getScriptInterface()) {
		tfs::lua::reportError(L, "MonsterTypes can only be registered in the Scripts interface.");
		lua_pushnil(L);
		return 1;
	}

	const std::string& name = tfs::lua::getString(L, 1);
	if (name.length() == 0) {
		lua_pushnil(L);
		return 1;
	}

	MonsterType* monsterType = g_monsters.getMonsterType(name);
	if (!monsterType) {
		monsterType = &g_monsters.monsters[boost::algorithm::to_lower_copy(name)];
		monsterType->name = name;
		monsterType->nameDescription = "a " + name;
		monsterType->monsterName = boost::algorithm::to_lower_copy(name);
	} else {
		monsterType->info.lootItems.clear();
		monsterType->info.attackSpells.clear();
		monsterType->info.defenseSpells.clear();
		monsterType->info.thinkEvent = -1;
		monsterType->info.creatureAppearEvent = -1;
		monsterType->info.creatureDisappearEvent = -1;
		monsterType->info.creatureMoveEvent = -1;
		monsterType->info.creatureSayEvent = -1;
	}

	tfs::lua::pushUserdata(L, monsterType);
	tfs::lua::setMetatable(L, -1, "MonsterType");
	return 1;
}

int luaGameGetClientVersion(lua_State* L)
{
	// Game.getClientVersion()
	lua_createtable(L, 0, 3);
	tfs::lua::setField(L, "min", CLIENT_VERSION_MIN);
	tfs::lua::setField(L, "max", CLIENT_VERSION_MAX);
	tfs::lua::setField(L, "string", CLIENT_VERSION_STR);
	return 1;
}

// Mirrors the trailing `lua_gc(g_luaEnvironment.getLuaState(), LUA_GCCOLLECT, 0)`
// that the previous luaGameReload(type) ran after every reload type. Reloading
// XML/Lua data on subsystems can leave many short-lived objects on the main
// interface; the original collected them eagerly and we preserve that exactly.
void gcMainInterfaceAfterReload() { lua_gc(g_luaEnvironment.getLuaState(), LUA_GCCOLLECT, 0); }

int luaGameSaveMap(lua_State* L)
{
	// Game.saveMap()
	tfs::lua::pushBoolean(L, Map::save());
	return 1;
}

int luaGameReloadActions(lua_State* L)
{
	// Game.reloadActions()
	const bool ok = g_actions->reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadChat(lua_State* L)
{
	// Game.reloadChat()
	const bool ok = g_chat.load();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadConfig(lua_State* L)
{
	// Game.reloadConfig()
	const bool ok = ConfigManager::load();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadEvents(lua_State* L)
{
	// Game.reloadEvents()
	tfs::events::reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameReloadItems(lua_State* L)
{
	// Game.reloadItems()
	const bool ok = Item::items.reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadMonsters(lua_State* L)
{
	// Game.reloadMonsters()
	const bool ok = g_monsters.reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadMovements(lua_State* L)
{
	// Game.reloadMovements()
	const bool ok = g_moveEvents->reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadNpcs(lua_State* L)
{
	// Game.reloadNpcs()
	Npcs::reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameReloadSpells(lua_State* L)
{
	// Game.reloadSpells()
	// Matches the previous RELOAD_TYPE_SPELLS path: also reloads monsters, and
	// terminates on failure because partial spell/monster data would leave the
	// game in an inconsistent state.
	if (!g_spells->reload()) {
		std::cout << "[Error - Game.reloadSpells] Failed to reload spells." << std::endl;
		std::terminate();
	}
	if (!g_monsters.reload()) {
		std::cout << "[Error - Game.reloadSpells] Failed to reload monsters." << std::endl;
		std::terminate();
	}
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameReloadTalkActions(lua_State* L)
{
	// Game.reloadTalkActions()
	const bool ok = g_talkActions->reload();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadWeapons(lua_State* L)
{
	// Game.reloadWeapons()
	// Matches the previous RELOAD_TYPE_WEAPONS path: loadDefaults() only. Repeat
	// calls are idempotent because Weapons::loadDefaults skips entries already
	// present in `weapons`. The full clear+rebuild happens in reloadScripts()
	// and reloadAll(), as in the original switch.
	g_weapons->loadDefaults();
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameReloadScripts(lua_State* L)
{
	// Game.reloadScripts()
	g_actions->clear(true);
	g_moveEvents->clear(true);
	g_talkActions->clear(true);
	g_weapons->clear(true);
	g_weapons->loadDefaults();
	g_spells->clear(true);
	const bool ok = g_scripts->loadScripts("scripts", false, true);
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, ok);
	return 1;
}

int luaGameReloadGlobal(lua_State* L)
{
	// Game.reloadGlobal()
	// Returns two booleans (global.lua loaded, scripts/lib loaded) to preserve
	// the multi-return contract of the previous luaGameReload(RELOAD_TYPE_GLOBAL)
	// path. The Lua dispatcher forwards both values, so Game.reload(GLOBAL)
	// keeps the same return shape it had before this migration.
	const bool globalLoaded = g_luaEnvironment.loadFile("data/global.lua") == 0;
	const bool libsLoaded = g_scripts->loadScripts("scripts/lib", true, true);
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, globalLoaded);
	tfs::lua::pushBoolean(L, libsLoaded);
	return 2;
}

int luaGameReloadAll(lua_State* L)
{
	// Game.reloadAll()
	// Matches the previous default branch of Game::reload(): mirrors the exact
	// order of subsystem reloads, including the duplicate spell/monster passes
	// that the original sweep performed.
	if (!g_spells->reload()) {
		std::cout << "[Error - Game.reloadAll] Failed to reload spells." << std::endl;
		std::terminate();
	}
	if (!g_monsters.reload()) {
		std::cout << "[Error - Game.reloadAll] Failed to reload monsters." << std::endl;
		std::terminate();
	}

	g_actions->reload();
	ConfigManager::load();
	g_monsters.reload();
	g_moveEvents->reload();
	Npcs::reload();
	g_talkActions->reload();
	Item::items.reload();
	g_weapons->clear(true);
	g_weapons->loadDefaults();
	tfs::events::reload();
	g_chat.load();
	g_actions->clear(true);
	g_moveEvents->clear(true);
	g_talkActions->clear(true);
	g_spells->clear(true);
	g_scripts->loadScripts("scripts", false, true);
	gcMainInterfaceAfterReload();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaGameGetPlayerRecord(lua_State* L)
{
	// Game.getPlayerRecord()
	tfs::lua::pushNumber(L, g_game.getPlayerRecord());
	return 1;
}

int luaGameSetPlayerRecord(lua_State* L)
{
	// Game.setPlayerRecord(record)
	g_game.setPlayerRecord(tfs::lua::getNumber<uint32_t>(L, 1));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

} // namespace

void tfs::lua::registerGame(LuaScriptInterface& lsi)
{
	registerEnum(lsi, ITEM_STACK_SIZE);
	registerEnum(lsi, MAX_STACKPOS);

	registerEnum(lsi, GAME_STATE_STARTUP);
	registerEnum(lsi, GAME_STATE_INIT);
	registerEnum(lsi, GAME_STATE_NORMAL);
	registerEnum(lsi, GAME_STATE_CLOSED);
	registerEnum(lsi, GAME_STATE_SHUTDOWN);
	registerEnum(lsi, GAME_STATE_CLOSING);
	registerEnum(lsi, GAME_STATE_MAINTAIN);

	lsi.registerGlobalVariable("SCHEDULER_MINTICKS", SCHEDULER_MINTICKS.count());

	registerEnum(lsi, WORLD_TYPE_NO_PVP);
	registerEnum(lsi, WORLD_TYPE_PVP);
	registerEnum(lsi, WORLD_TYPE_PVP_ENFORCED);

	lsi.registerTable("Game");

	lsi.registerMethod("Game", "getSpectators", luaGameGetSpectators);
	lsi.registerMethod("Game", "getPlayers", luaGameGetPlayers);
	lsi.registerMethod("Game", "getNpcs", luaGameGetNpcs);
	lsi.registerMethod("Game", "getMonsters", luaGameGetMonsters);
	lsi.registerMethod("Game", "loadMap", luaGameLoadMap);

	lsi.registerMethod("Game", "getExperienceStage", luaGameGetExperienceStage);
	lsi.registerMethod("Game", "getExperienceForLevel", luaGameGetExperienceForLevel);
	lsi.registerMethod("Game", "getMonsterCount", luaGameGetMonsterCount);
	lsi.registerMethod("Game", "getPlayerCount", luaGameGetPlayerCount);
	lsi.registerMethod("Game", "getNpcCount", luaGameGetNpcCount);
	lsi.registerMethod("Game", "getMonsterTypes", luaGameGetMonsterTypes);
	lsi.registerMethod("Game", "getBestiary", luaGameGetBestiary);
	lsi.registerMethod("Game", "getCurrencyItems", luaGameGetCurrencyItems);
	lsi.registerMethod("Game", "getItemTypeByClientId", luaGameGetItemTypeByClientId);

	lsi.registerMethod("Game", "getParties", luaGameGetParties);
	lsi.registerMethod("Game", "getTowns", luaGameGetTowns);
	lsi.registerMethod("Game", "getHouses", luaGameGetHouses);
	lsi.registerMethod("Game", "getVocations", luaGameGetVocations);
	lsi.registerMethod("Game", "getRuneSpells", luaGameGetRuneSpells);
	lsi.registerMethod("Game", "getInstantSpells", luaGameGetInstantSpells);

	lsi.registerMethod("Game", "getGameState", luaGameGetGameState);
	lsi.registerMethod("Game", "setGameState", luaGameSetGameState);

	lsi.registerMethod("Game", "getWorldType", luaGameGetWorldType);
	lsi.registerMethod("Game", "setWorldType", luaGameSetWorldType);

	lsi.registerMethod("Game", "getItemAttributeByName", luaGameGetItemAttributeByName);
	lsi.registerMethod("Game", "getReturnMessage", luaGameGetReturnMessage);

	lsi.registerMethod("Game", "createItem", luaGameCreateItem);
	lsi.registerMethod("Game", "createContainer", luaGameCreateContainer);
	lsi.registerMethod("Game", "createMonster", luaGameCreateMonster);
	lsi.registerMethod("Game", "createNpc", luaGameCreateNpc);
	lsi.registerMethod("Game", "createTile", luaGameCreateTile);
	lsi.registerMethod("Game", "createMonsterType", luaGameCreateMonsterType);

	lsi.registerMethod("Game", "getClientVersion", luaGameGetClientVersion);

	lsi.registerMethod("Game", "saveMap", luaGameSaveMap);

	// Per-subsystem reload bindings. Game.reload(type) lives in Lua
	// (data/scripts/systems/game_state.lua) and dispatches through these.
	lsi.registerMethod("Game", "reloadActions", luaGameReloadActions);
	lsi.registerMethod("Game", "reloadChat", luaGameReloadChat);
	lsi.registerMethod("Game", "reloadConfig", luaGameReloadConfig);
	lsi.registerMethod("Game", "reloadEvents", luaGameReloadEvents);
	lsi.registerMethod("Game", "reloadGlobal", luaGameReloadGlobal);
	lsi.registerMethod("Game", "reloadItems", luaGameReloadItems);
	lsi.registerMethod("Game", "reloadMonsters", luaGameReloadMonsters);
	lsi.registerMethod("Game", "reloadMovements", luaGameReloadMovements);
	lsi.registerMethod("Game", "reloadNpcs", luaGameReloadNpcs);
	lsi.registerMethod("Game", "reloadScripts", luaGameReloadScripts);
	lsi.registerMethod("Game", "reloadSpells", luaGameReloadSpells);
	lsi.registerMethod("Game", "reloadTalkActions", luaGameReloadTalkActions);
	lsi.registerMethod("Game", "reloadWeapons", luaGameReloadWeapons);
	lsi.registerMethod("Game", "reloadAll", luaGameReloadAll);

	lsi.registerMethod("Game", "getPlayerRecord", luaGameGetPlayerRecord);
	lsi.registerMethod("Game", "setPlayerRecord", luaGameSetPlayerRecord);
}
