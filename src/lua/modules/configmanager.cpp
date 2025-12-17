#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

#define registerEnumIn(i, tableName, value) \
	{ \
		std::string enumName = #value; \
		i.registerVariable(tableName, enumName.substr(enumName.find_last_of(':') + 1), value); \
	}

void tfs::lua::registerConfigManager(LuaScriptInterface& i)
{
	i.registerTable("configKeys");

	registerEnumIn(i, "configKeys", ConfigManager::ALLOW_CHANGEOUTFIT);
	registerEnumIn(i, "configKeys", ConfigManager::ONE_PLAYER_ON_ACCOUNT);
	registerEnumIn(i, "configKeys", ConfigManager::AIMBOT_HOTKEY_ENABLED);
	registerEnumIn(i, "configKeys", ConfigManager::REMOVE_RUNE_CHARGES);
	registerEnumIn(i, "configKeys", ConfigManager::REMOVE_WEAPON_AMMO);
	registerEnumIn(i, "configKeys", ConfigManager::REMOVE_WEAPON_CHARGES);
	registerEnumIn(i, "configKeys", ConfigManager::REMOVE_POTION_CHARGES);
	registerEnumIn(i, "configKeys", ConfigManager::EXPERIENCE_FROM_PLAYERS);
	registerEnumIn(i, "configKeys", ConfigManager::FREE_PREMIUM);
	registerEnumIn(i, "configKeys", ConfigManager::REPLACE_KICK_ON_LOGIN);
	registerEnumIn(i, "configKeys", ConfigManager::ALLOW_CLONES);
	registerEnumIn(i, "configKeys", ConfigManager::BIND_ONLY_GLOBAL_ADDRESS);
	registerEnumIn(i, "configKeys", ConfigManager::OPTIMIZE_DATABASE);
	registerEnumIn(i, "configKeys", ConfigManager::MARKET_PREMIUM);
	registerEnumIn(i, "configKeys", ConfigManager::EMOTE_SPELLS);
	registerEnumIn(i, "configKeys", ConfigManager::STAMINA_SYSTEM);
	registerEnumIn(i, "configKeys", ConfigManager::WARN_UNSAFE_SCRIPTS);
	registerEnumIn(i, "configKeys", ConfigManager::CONVERT_UNSAFE_SCRIPTS);
	registerEnumIn(i, "configKeys", ConfigManager::CLASSIC_EQUIPMENT_SLOTS);
	registerEnumIn(i, "configKeys", ConfigManager::CLASSIC_ATTACK_SPEED);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_SAVE_NOTIFY_MESSAGE);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_SAVE_NOTIFY_DURATION);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_SAVE_CLEAN_MAP);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_SAVE_CLOSE);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_SAVE_SHUTDOWN);
	registerEnumIn(i, "configKeys", ConfigManager::ONLINE_OFFLINE_CHARLIST);
	registerEnumIn(i, "configKeys", ConfigManager::CHECK_DUPLICATE_STORAGE_KEYS);

	registerEnumIn(i, "configKeys", ConfigManager::MAP_NAME);
	registerEnumIn(i, "configKeys", ConfigManager::HOUSE_RENT_PERIOD);
	registerEnumIn(i, "configKeys", ConfigManager::SERVER_NAME);
	registerEnumIn(i, "configKeys", ConfigManager::OWNER_NAME);
	registerEnumIn(i, "configKeys", ConfigManager::OWNER_EMAIL);
	registerEnumIn(i, "configKeys", ConfigManager::URL);
	registerEnumIn(i, "configKeys", ConfigManager::LOCATION);
	registerEnumIn(i, "configKeys", ConfigManager::IP);
	registerEnumIn(i, "configKeys", ConfigManager::WORLD_TYPE);
	registerEnumIn(i, "configKeys", ConfigManager::MYSQL_HOST);
	registerEnumIn(i, "configKeys", ConfigManager::MYSQL_USER);
	registerEnumIn(i, "configKeys", ConfigManager::MYSQL_PASS);
	registerEnumIn(i, "configKeys", ConfigManager::MYSQL_DB);
	registerEnumIn(i, "configKeys", ConfigManager::MYSQL_SOCK);
	registerEnumIn(i, "configKeys", ConfigManager::DEFAULT_PRIORITY);
	registerEnumIn(i, "configKeys", ConfigManager::MAP_AUTHOR);

	registerEnumIn(i, "configKeys", ConfigManager::SQL_PORT);
	registerEnumIn(i, "configKeys", ConfigManager::MAX_PLAYERS);
	registerEnumIn(i, "configKeys", ConfigManager::PZ_LOCKED);
	registerEnumIn(i, "configKeys", ConfigManager::DEFAULT_DESPAWNRANGE);
	registerEnumIn(i, "configKeys", ConfigManager::DEFAULT_DESPAWNRADIUS);
	registerEnumIn(i, "configKeys", ConfigManager::DEFAULT_WALKTOSPAWNRADIUS);
	registerEnumIn(i, "configKeys", ConfigManager::REMOVE_ON_DESPAWN);
	registerEnumIn(i, "configKeys", ConfigManager::RATE_EXPERIENCE);
	registerEnumIn(i, "configKeys", ConfigManager::RATE_SKILL);
	registerEnumIn(i, "configKeys", ConfigManager::RATE_LOOT);
	registerEnumIn(i, "configKeys", ConfigManager::RATE_MAGIC);
	registerEnumIn(i, "configKeys", ConfigManager::RATE_SPAWN);
	registerEnumIn(i, "configKeys", ConfigManager::HOUSE_PRICE);
	registerEnumIn(i, "configKeys", ConfigManager::KILLS_TO_RED);
	registerEnumIn(i, "configKeys", ConfigManager::KILLS_TO_BLACK);
	registerEnumIn(i, "configKeys", ConfigManager::MAX_MESSAGEBUFFER);
	registerEnumIn(i, "configKeys", ConfigManager::ACTIONS_DELAY_INTERVAL);
	registerEnumIn(i, "configKeys", ConfigManager::EX_ACTIONS_DELAY_INTERVAL);
	registerEnumIn(i, "configKeys", ConfigManager::PROTECTION_LEVEL);
	registerEnumIn(i, "configKeys", ConfigManager::DEATH_LOSE_PERCENT);
	registerEnumIn(i, "configKeys", ConfigManager::STATUSQUERY_TIMEOUT);
	registerEnumIn(i, "configKeys", ConfigManager::FRAG_TIME);
	registerEnumIn(i, "configKeys", ConfigManager::WHITE_SKULL_TIME);
	registerEnumIn(i, "configKeys", ConfigManager::GAME_PORT);
	registerEnumIn(i, "configKeys", ConfigManager::STATUS_PORT);
	registerEnumIn(i, "configKeys", ConfigManager::STAIRHOP_DELAY);
	registerEnumIn(i, "configKeys", ConfigManager::MARKET_OFFER_DURATION);
	registerEnumIn(i, "configKeys", ConfigManager::CHECK_EXPIRED_MARKET_OFFERS_EACH_MINUTES);
	registerEnumIn(i, "configKeys", ConfigManager::MAX_MARKET_OFFERS_AT_A_TIME_PER_PLAYER);
	registerEnumIn(i, "configKeys", ConfigManager::EXP_FROM_PLAYERS_LEVEL_RANGE);
	registerEnumIn(i, "configKeys", ConfigManager::MAX_PACKETS_PER_SECOND);
	registerEnumIn(i, "configKeys", ConfigManager::TWO_FACTOR_AUTH);
	registerEnumIn(i, "configKeys", ConfigManager::MANASHIELD_BREAKABLE);
	registerEnumIn(i, "configKeys", ConfigManager::STAMINA_REGEN_MINUTE);
	registerEnumIn(i, "configKeys", ConfigManager::STAMINA_REGEN_PREMIUM);
	registerEnumIn(i, "configKeys", ConfigManager::HOUSE_DOOR_SHOW_PRICE);
	registerEnumIn(i, "configKeys", ConfigManager::MONSTER_OVERSPAWN);

	registerEnumIn(i, "configKeys", ConfigManager::QUEST_TRACKER_FREE_LIMIT);
	registerEnumIn(i, "configKeys", ConfigManager::QUEST_TRACKER_PREMIUM_LIMIT);
}
