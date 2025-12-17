#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaMonsterTypeCreate(lua_State* L)
{
	// MonsterType(name or raceId)
	MonsterType* monsterType;
	if (tfs::lua::isNumber(L, 2)) {
		monsterType = g_monsters.getMonsterType(tfs::lua::getNumber<uint32_t>(L, 2));
	} else {
		monsterType = g_monsters.getMonsterType(tfs::lua::getString(L, 2));
	}

	if (monsterType) {
		tfs::lua::pushUserdata(L, monsterType);
		tfs::lua::setMetatable(L, -1, "MonsterType");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsAttackable(lua_State* L)
{
	// get: monsterType:isAttackable() set: monsterType:isAttackable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isAttackable);
		} else {
			monsterType->info.isAttackable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsChallengeable(lua_State* L)
{
	// get: monsterType:isChallengeable() set: monsterType:isChallengeable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isChallengeable);
		} else {
			monsterType->info.isChallengeable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsConvinceable(lua_State* L)
{
	// get: monsterType:isConvinceable() set: monsterType:isConvinceable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isConvinceable);
		} else {
			monsterType->info.isConvinceable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsSummonable(lua_State* L)
{
	// get: monsterType:isSummonable() set: monsterType:isSummonable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isSummonable);
		} else {
			monsterType->info.isSummonable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsIgnoringSpawnBlock(lua_State* L)
{
	// get: monsterType:isIgnoringSpawnBlock() set: monsterType:isIgnoringSpawnBlock(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isIgnoringSpawnBlock);
		} else {
			monsterType->info.isIgnoringSpawnBlock = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsIllusionable(lua_State* L)
{
	// get: monsterType:isIllusionable() set: monsterType:isIllusionable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isIllusionable);
		} else {
			monsterType->info.isIllusionable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsHostile(lua_State* L)
{
	// get: monsterType:isHostile() set: monsterType:isHostile(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isHostile);
		} else {
			monsterType->info.isHostile = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsPushable(lua_State* L)
{
	// get: monsterType:isPushable() set: monsterType:isPushable(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.pushable);
		} else {
			monsterType->info.pushable = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsHealthHidden(lua_State* L)
{
	// get: monsterType:isHealthHidden() set: monsterType:isHealthHidden(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.hiddenHealth);
		} else {
			monsterType->info.hiddenHealth = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeIsBoss(lua_State* L)
{
	// get: monsterType:isBoss() set: monsterType:isBoss(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.isBoss);
		} else {
			monsterType->info.isBoss = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCanPushItems(lua_State* L)
{
	// get: monsterType:canPushItems() set: monsterType:canPushItems(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.canPushItems);
		} else {
			monsterType->info.canPushItems = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCanPushCreatures(lua_State* L)
{
	// get: monsterType:canPushCreatures() set: monsterType:canPushCreatures(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.canPushCreatures);
		} else {
			monsterType->info.canPushCreatures = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCanWalkOnEnergy(lua_State* L)
{
	// get: monsterType:canWalkOnEnergy() set: monsterType:canWalkOnEnergy(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.canWalkOnEnergy);
		} else {
			monsterType->info.canWalkOnEnergy = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCanWalkOnFire(lua_State* L)
{
	// get: monsterType:canWalkOnFire() set: monsterType:canWalkOnFire(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.canWalkOnFire);
		} else {
			monsterType->info.canWalkOnFire = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCanWalkOnPoison(lua_State* L)
{
	// get: monsterType:canWalkOnPoison() set: monsterType:canWalkOnPoison(bool)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushBoolean(L, monsterType->info.canWalkOnPoison);
		} else {
			monsterType->info.canWalkOnPoison = tfs::lua::getBoolean(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int32_t luaMonsterTypeName(lua_State* L)
{
	// get: monsterType:name() set: monsterType:name(name)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushString(L, monsterType->name);
		} else {
			monsterType->name = tfs::lua::getString(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeNameDescription(lua_State* L)
{
	// get: monsterType:nameDescription() set: monsterType:nameDescription(desc)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushString(L, monsterType->nameDescription);
		} else {
			monsterType->nameDescription = tfs::lua::getString(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeHealth(lua_State* L)
{
	// get: monsterType:health() set: monsterType:health(health)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.health);
		} else {
			monsterType->info.health = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeMaxHealth(lua_State* L)
{
	// get: monsterType:maxHealth() set: monsterType:maxHealth(health)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.healthMax);
		} else {
			monsterType->info.healthMax = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeRunHealth(lua_State* L)
{
	// get: monsterType:runHealth() set: monsterType:runHealth(health)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.runAwayHealth);
		} else {
			monsterType->info.runAwayHealth = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeExperience(lua_State* L)
{
	// get: monsterType:experience() set: monsterType:experience(exp)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.experience);
		} else {
			monsterType->info.experience = tfs::lua::getNumber<uint64_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeSkull(lua_State* L)
{
	// get: monsterType:skull() set: monsterType:skull(str/constant)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.skull);
		} else {
			if (tfs::lua::isNumber(L, 2)) {
				monsterType->info.skull = tfs::lua::getNumber<Skulls_t>(L, 2);
			} else {
				monsterType->info.skull = getSkullType(tfs::lua::getString(L, 2));
			}
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCombatImmunities(lua_State* L)
{
	// get: monsterType:combatImmunities() set: monsterType:combatImmunities(immunity)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.damageImmunities);
		} else {
			std::string immunity = tfs::lua::getString(L, 2);
			if (immunity == "physical") {
				monsterType->info.damageImmunities |= COMBAT_PHYSICALDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "energy") {
				monsterType->info.damageImmunities |= COMBAT_ENERGYDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "fire") {
				monsterType->info.damageImmunities |= COMBAT_FIREDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "poison" || immunity == "earth") {
				monsterType->info.damageImmunities |= COMBAT_EARTHDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "drown") {
				monsterType->info.damageImmunities |= COMBAT_DROWNDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "ice") {
				monsterType->info.damageImmunities |= COMBAT_ICEDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "holy") {
				monsterType->info.damageImmunities |= COMBAT_HOLYDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "death") {
				monsterType->info.damageImmunities |= COMBAT_DEATHDAMAGE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "lifedrain") {
				monsterType->info.damageImmunities |= COMBAT_LIFEDRAIN;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "manadrain") {
				monsterType->info.damageImmunities |= COMBAT_MANADRAIN;
				tfs::lua::pushBoolean(L, true);
			} else {
				std::cout << "[Warning - Monsters::loadMonster] Unknown immunity name " << immunity
				          << " for monster: " << monsterType->name << '\n';
				lua_pushnil(L);
			}
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeConditionImmunities(lua_State* L)
{
	// get: monsterType:conditionImmunities() set: monsterType:conditionImmunities(immunity)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.conditionImmunities);
		} else {
			std::string immunity = tfs::lua::getString(L, 2);
			if (immunity == "physical") {
				monsterType->info.conditionImmunities |= CONDITION_BLEEDING;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "energy") {
				monsterType->info.conditionImmunities |= CONDITION_ENERGY;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "fire") {
				monsterType->info.conditionImmunities |= CONDITION_FIRE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "poison" || immunity == "earth") {
				monsterType->info.conditionImmunities |= CONDITION_POISON;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "drown") {
				monsterType->info.conditionImmunities |= CONDITION_DROWN;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "ice") {
				monsterType->info.conditionImmunities |= CONDITION_FREEZING;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "holy") {
				monsterType->info.conditionImmunities |= CONDITION_DAZZLED;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "death") {
				monsterType->info.conditionImmunities |= CONDITION_CURSED;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "paralyze") {
				monsterType->info.conditionImmunities |= CONDITION_PARALYZE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "outfit") {
				monsterType->info.conditionImmunities |= CONDITION_OUTFIT;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "drunk") {
				monsterType->info.conditionImmunities |= CONDITION_DRUNK;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "invisible" || immunity == "invisibility") {
				monsterType->info.conditionImmunities |= CONDITION_INVISIBLE;
				tfs::lua::pushBoolean(L, true);
			} else if (immunity == "bleed") {
				monsterType->info.conditionImmunities |= CONDITION_BLEEDING;
				tfs::lua::pushBoolean(L, true);
			} else {
				std::cout << "[Warning - Monsters::loadMonster] Unknown immunity name " << immunity
				          << " for monster: " << monsterType->name << '\n';
				lua_pushnil(L);
			}
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetAttackList(lua_State* L)
{
	// monsterType:getAttackList()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	lua_createtable(L, monsterType->info.attackSpells.size(), 0);

	int index = 0;
	for (const auto& spellBlock : monsterType->info.attackSpells) {
		lua_createtable(L, 0, 8);

		tfs::lua::setField(L, "chance", spellBlock.chance);
		tfs::lua::setField(L, "isCombatSpell", spellBlock.combatSpell ? 1 : 0);
		tfs::lua::setField(L, "isMelee", spellBlock.isMelee ? 1 : 0);
		tfs::lua::setField(L, "minCombatValue", spellBlock.minCombatValue);
		tfs::lua::setField(L, "maxCombatValue", spellBlock.maxCombatValue);
		tfs::lua::setField(L, "range", spellBlock.range);
		tfs::lua::setField(L, "speed", spellBlock.speed);
		tfs::lua::pushUserdata(L, static_cast<CombatSpell*>(spellBlock.spell));
		lua_setfield(L, -2, "spell");

		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterTypeAddAttack(lua_State* L)
{
	// monsterType:addAttack(monsterspell)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 2);
		if (spell) {
			spellBlock_t sb;
			if (g_monsters.deserializeSpell(spell, sb, monsterType->name)) {
				monsterType->info.attackSpells.push_back(std::move(sb));
			} else {
				std::cout << monsterType->name << '\n';
				std::cout << "[Warning - Monsters::loadMonster] Cant load spell. " << spell->name << '\n';
			}
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetDefenseList(lua_State* L)
{
	// monsterType:getDefenseList()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	lua_createtable(L, monsterType->info.defenseSpells.size(), 0);

	int index = 0;
	for (const auto& spellBlock : monsterType->info.defenseSpells) {
		lua_createtable(L, 0, 8);

		tfs::lua::setField(L, "chance", spellBlock.chance);
		tfs::lua::setField(L, "isCombatSpell", spellBlock.combatSpell ? 1 : 0);
		tfs::lua::setField(L, "isMelee", spellBlock.isMelee ? 1 : 0);
		tfs::lua::setField(L, "minCombatValue", spellBlock.minCombatValue);
		tfs::lua::setField(L, "maxCombatValue", spellBlock.maxCombatValue);
		tfs::lua::setField(L, "range", spellBlock.range);
		tfs::lua::setField(L, "speed", spellBlock.speed);
		tfs::lua::pushUserdata(L, static_cast<CombatSpell*>(spellBlock.spell));
		lua_setfield(L, -2, "spell");

		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterTypeAddDefense(lua_State* L)
{
	// monsterType:addDefense(monsterspell)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 2);
		if (spell) {
			spellBlock_t sb;
			if (g_monsters.deserializeSpell(spell, sb, monsterType->name)) {
				monsterType->info.defenseSpells.push_back(std::move(sb));
			} else {
				std::cout << monsterType->name << '\n';
				std::cout << "[Warning - Monsters::loadMonster] Cant load spell. " << spell->name << '\n';
			}
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetElementList(lua_State* L)
{
	// monsterType:getElementList()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	lua_createtable(L, monsterType->info.elementMap.size(), 0);
	for (auto&& [combatType, percent] : monsterType->info.elementMap | std::views::as_const) {
		tfs::lua::pushNumber(L, percent);
		lua_rawseti(L, -2, combatType);
	}
	return 1;
}

int luaMonsterTypeAddElement(lua_State* L)
{
	// monsterType:addElement(type, percent)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		CombatType_t element = tfs::lua::getNumber<CombatType_t>(L, 2);
		monsterType->info.elementMap[element] = tfs::lua::getNumber<int32_t>(L, 3);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetVoices(lua_State* L)
{
	// monsterType:getVoices()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	int index = 0;
	lua_createtable(L, monsterType->info.voiceVector.size(), 0);
	for (const auto& voiceBlock : monsterType->info.voiceVector) {
		lua_createtable(L, 0, 2);
		tfs::lua::setField(L, "text", voiceBlock.text);
		tfs::lua::setField(L, "yellText", voiceBlock.yellText);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterTypeAddVoice(lua_State* L)
{
	// monsterType:addVoice(sentence, interval, chance, yell)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		voiceBlock_t voice;
		voice.text = tfs::lua::getString(L, 2);
		monsterType->info.yellSpeedTicks = tfs::lua::getNumber<uint32_t>(L, 3);
		monsterType->info.yellChance = tfs::lua::getNumber<uint32_t>(L, 4);
		voice.yellText = tfs::lua::getBoolean(L, 5);
		monsterType->info.voiceVector.push_back(voice);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetLoot(lua_State* L)
{
	// monsterType:getLoot()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushLoot(L, monsterType->info.lootItems);
	return 1;
}

int luaMonsterTypeAddLoot(lua_State* L)
{
	// monsterType:addLoot(loot)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		Loot* loot = tfs::lua::getUserdata<Loot>(L, 2);
		if (loot) {
			monsterType->loadLoot(monsterType, loot->lootBlock);
			tfs::lua::pushBoolean(L, true);
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetCreatureEvents(lua_State* L)
{
	// monsterType:getCreatureEvents()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	int index = 0;
	lua_createtable(L, monsterType->info.scripts.size(), 0);
	for (const std::string& creatureEvent : monsterType->info.scripts) {
		tfs::lua::pushString(L, creatureEvent);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterTypeRegisterEvent(lua_State* L)
{
	// monsterType:registerEvent(name)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		monsterType->info.scripts.push_back(tfs::lua::getString(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeEventOnCallback(lua_State* L)
{
	// monsterType:onThink(callback)
	// monsterType:onAppear(callback)
	// monsterType:onDisappear(callback)
	// monsterType:onMove(callback)
	// monsterType:onSay(callback)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (monsterType->loadCallback(&g_scripts->getScriptInterface())) {
			tfs::lua::pushBoolean(L, true);
			return 1;
		}
		tfs::lua::pushBoolean(L, false);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeEventType(lua_State* L)
{
	// monstertype:eventType(event)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		monsterType->info.eventType = tfs::lua::getNumber<MonstersEvent_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeGetSummonList(lua_State* L)
{
	// monsterType:getSummonList()
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	int index = 0;
	lua_createtable(L, monsterType->info.summons.size(), 0);
	for (const auto& summonBlock : monsterType->info.summons) {
		lua_createtable(L, 0, 6);
		tfs::lua::setField(L, "name", summonBlock.name);
		tfs::lua::setField(L, "speed", summonBlock.speed);
		tfs::lua::setField(L, "chance", summonBlock.chance);
		tfs::lua::setField(L, "max", summonBlock.max);
		tfs::lua::setField(L, "effect", summonBlock.effect);
		tfs::lua::setField(L, "masterEffect", summonBlock.masterEffect);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int luaMonsterTypeAddSummon(lua_State* L)
{
	// monsterType:addSummon(name, interval, chance[, max = -1[, effect = CONST_ME_TELEPORT[, masterEffect =
	// CONST_ME_NONE]]])
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		summonBlock_t summon;
		summon.name = tfs::lua::getString(L, 2);
		summon.speed = tfs::lua::getNumber<int32_t>(L, 3);
		summon.chance = tfs::lua::getNumber<int32_t>(L, 4);
		summon.max = tfs::lua::getNumber<int32_t>(L, 5, -1);
		summon.effect = tfs::lua::getNumber<MagicEffectClasses>(L, 6, CONST_ME_TELEPORT);
		summon.masterEffect = tfs::lua::getNumber<MagicEffectClasses>(L, 7, CONST_ME_NONE);
		monsterType->info.summons.push_back(summon);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeMaxSummons(lua_State* L)
{
	// get: monsterType:maxSummons() set: monsterType:maxSummons(ammount)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.maxSummons);
		} else {
			monsterType->info.maxSummons = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeArmor(lua_State* L)
{
	// get: monsterType:armor() set: monsterType:armor(armor)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.armor);
		} else {
			monsterType->info.armor = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeDefense(lua_State* L)
{
	// get: monsterType:defense() set: monsterType:defense(defense)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.defense);
		} else {
			monsterType->info.defense = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeOutfit(lua_State* L)
{
	// get: monsterType:outfit() set: monsterType:outfit(outfit)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushOutfit(L, monsterType->info.outfit);
		} else {
			monsterType->info.outfit = tfs::lua::getOutfit(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeRace(lua_State* L)
{
	// get: monsterType:race() set: monsterType:race(race)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	std::string race = tfs::lua::getString(L, 2);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.race);
		} else {
			if (race == "venom") {
				monsterType->info.race = RACE_VENOM;
			} else if (race == "blood") {
				monsterType->info.race = RACE_BLOOD;
			} else if (race == "undead") {
				monsterType->info.race = RACE_UNDEAD;
			} else if (race == "fire") {
				monsterType->info.race = RACE_FIRE;
			} else if (race == "energy") {
				monsterType->info.race = RACE_ENERGY;
			} else {
				std::cout << "[Warning - Monsters::loadMonster] Unknown race type " << race << ".\n";
				lua_pushnil(L);
				return 1;
			}
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeCorpseId(lua_State* L)
{
	// get: monsterType:corpseId() set: monsterType:corpseId(id)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.lookcorpse);
		} else {
			monsterType->info.lookcorpse = tfs::lua::getNumber<uint16_t>(L, 2);
			lua_pushboolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeManaCost(lua_State* L)
{
	// get: monsterType:manaCost() set: monsterType:manaCost(mana)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.manaCost);
		} else {
			monsterType->info.manaCost = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeBaseSpeed(lua_State* L)
{
	// get: monsterType:baseSpeed() set: monsterType:baseSpeed(speed)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.baseSpeed);
		} else {
			monsterType->info.baseSpeed = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeLight(lua_State* L)
{
	// get: monsterType:light() set: monsterType:light(color, level)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}
	if (lua_gettop(L) == 1) {
		tfs::lua::pushNumber(L, monsterType->info.light.level);
		tfs::lua::pushNumber(L, monsterType->info.light.color);
		return 2;
	} else {
		monsterType->info.light.color = tfs::lua::getNumber<uint8_t>(L, 2);
		monsterType->info.light.level = tfs::lua::getNumber<uint8_t>(L, 3);
		tfs::lua::pushBoolean(L, true);
	}
	return 1;
}

int luaMonsterTypeStaticAttackChance(lua_State* L)
{
	// get: monsterType:staticAttackChance() set: monsterType:staticAttackChance(chance)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.staticAttackChance);
		} else {
			monsterType->info.staticAttackChance = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeTargetDistance(lua_State* L)
{
	// get: monsterType:targetDistance() set: monsterType:targetDistance(distance)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.targetDistance);
		} else {
			monsterType->info.targetDistance = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeYellChance(lua_State* L)
{
	// get: monsterType:yellChance() set: monsterType:yellChance(chance)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.yellChance);
		} else {
			monsterType->info.yellChance = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeYellSpeedTicks(lua_State* L)
{
	// get: monsterType:yellSpeedTicks() set: monsterType:yellSpeedTicks(rate)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.yellSpeedTicks);
		} else {
			monsterType->info.yellSpeedTicks = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeChangeTargetChance(lua_State* L)
{
	// get: monsterType:changeTargetChance() set: monsterType:changeTargetChance(chance)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.changeTargetChance);
		} else {
			monsterType->info.changeTargetChance = tfs::lua::getNumber<int32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeChangeTargetSpeed(lua_State* L)
{
	// get: monsterType:changeTargetSpeed() set: monsterType:changeTargetSpeed(speed)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (monsterType) {
		if (lua_gettop(L) == 1) {
			tfs::lua::pushNumber(L, monsterType->info.changeTargetSpeed);
		} else {
			monsterType->info.changeTargetSpeed = tfs::lua::getNumber<uint32_t>(L, 2);
			tfs::lua::pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterTypeBestiaryInfo(lua_State* L)
{
	// get: monsterType:bestiaryInfo() set: monsterType:bestiaryInfo(info)
	MonsterType* monsterType = tfs::lua::getUserdata<MonsterType>(L, 1);
	if (!monsterType) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) == 1) {
		const auto& info = monsterType->bestiaryInfo;
		lua_createtable(L, 0, 9);

		tfs::lua::setField(L, "class", info.className);
		tfs::lua::setField(L, "raceId", info.raceId);
		tfs::lua::setField(L, "prowess", info.prowess);
		tfs::lua::setField(L, "expertise", info.expertise);
		tfs::lua::setField(L, "mastery", info.mastery);
		tfs::lua::setField(L, "charmPoints", info.charmPoints);
		tfs::lua::setField(L, "difficulty", info.difficulty);
		tfs::lua::setField(L, "occurrence", info.occurrence);
		tfs::lua::setField(L, "locations", info.locations);
		return 1;
	}

	if (lua_istable(L, 2)) {
		BestiaryInfo info{
		    .className = tfs::lua::getFieldString(L, 2, "class"),
		    .raceId = tfs::lua::getField<uint32_t>(L, 2, "raceId"),
		    .prowess = tfs::lua::getField<uint32_t>(L, 2, "prowess"),
		    .expertise = tfs::lua::getField<uint32_t>(L, 2, "expertise"),
		    .mastery = tfs::lua::getField<uint32_t>(L, 2, "mastery"),
		    .charmPoints = tfs::lua::getField<uint32_t>(L, 2, "charmPoints"),
		    .difficulty = tfs::lua::getField<uint32_t>(L, 2, "difficulty"),
		    .occurrence = tfs::lua::getField<uint32_t>(L, 2, "occurrence"),
		    .locations = tfs::lua::getFieldString(L, 2, "locations"),
		};
		lua_pop(L, 9);

		if (g_monsters.isValidBestiaryInfo(info)) {
			monsterType->bestiaryInfo = std::move(info);
			tfs::lua::pushBoolean(L, g_monsters.addBestiaryMonsterType(monsterType));
		} else {
			tfs::lua::pushBoolean(L, false);
		}
		return 1;
	}

	std::cout << "[Warning - luaMonsterTypeBestiaryInfo] bestiaryInfo must be a table.\n";
	lua_pushnil(L);
	return 1;
}

} // namespace

void tfs::lua::registerMonsterType(LuaScriptInterface& i)
{
	i.registerClass("MonsterType", "", luaMonsterTypeCreate);
	i.registerMetaMethod("MonsterType", "__eq", tfs::lua::luaUserdataCompare);

	i.registerMethod("MonsterType", "isAttackable", luaMonsterTypeIsAttackable);
	i.registerMethod("MonsterType", "isChallengeable", luaMonsterTypeIsChallengeable);
	i.registerMethod("MonsterType", "isConvinceable", luaMonsterTypeIsConvinceable);
	i.registerMethod("MonsterType", "isSummonable", luaMonsterTypeIsSummonable);
	i.registerMethod("MonsterType", "isIgnoringSpawnBlock", luaMonsterTypeIsIgnoringSpawnBlock);
	i.registerMethod("MonsterType", "isIllusionable", luaMonsterTypeIsIllusionable);
	i.registerMethod("MonsterType", "isHostile", luaMonsterTypeIsHostile);
	i.registerMethod("MonsterType", "isPushable", luaMonsterTypeIsPushable);
	i.registerMethod("MonsterType", "isHealthHidden", luaMonsterTypeIsHealthHidden);
	i.registerMethod("MonsterType", "isBoss", luaMonsterTypeIsBoss);

	i.registerMethod("MonsterType", "canPushItems", luaMonsterTypeCanPushItems);
	i.registerMethod("MonsterType", "canPushCreatures", luaMonsterTypeCanPushCreatures);

	i.registerMethod("MonsterType", "canWalkOnEnergy", luaMonsterTypeCanWalkOnEnergy);
	i.registerMethod("MonsterType", "canWalkOnFire", luaMonsterTypeCanWalkOnFire);
	i.registerMethod("MonsterType", "canWalkOnPoison", luaMonsterTypeCanWalkOnPoison);

	i.registerMethod("MonsterType", "name", luaMonsterTypeName);
	i.registerMethod("MonsterType", "nameDescription", luaMonsterTypeNameDescription);

	i.registerMethod("MonsterType", "health", luaMonsterTypeHealth);
	i.registerMethod("MonsterType", "maxHealth", luaMonsterTypeMaxHealth);
	i.registerMethod("MonsterType", "runHealth", luaMonsterTypeRunHealth);
	i.registerMethod("MonsterType", "experience", luaMonsterTypeExperience);
	i.registerMethod("MonsterType", "skull", luaMonsterTypeSkull);

	i.registerMethod("MonsterType", "combatImmunities", luaMonsterTypeCombatImmunities);
	i.registerMethod("MonsterType", "conditionImmunities", luaMonsterTypeConditionImmunities);

	i.registerMethod("MonsterType", "getAttackList", luaMonsterTypeGetAttackList);
	i.registerMethod("MonsterType", "addAttack", luaMonsterTypeAddAttack);

	i.registerMethod("MonsterType", "getDefenseList", luaMonsterTypeGetDefenseList);
	i.registerMethod("MonsterType", "addDefense", luaMonsterTypeAddDefense);

	i.registerMethod("MonsterType", "getElementList", luaMonsterTypeGetElementList);
	i.registerMethod("MonsterType", "addElement", luaMonsterTypeAddElement);

	i.registerMethod("MonsterType", "getVoices", luaMonsterTypeGetVoices);
	i.registerMethod("MonsterType", "addVoice", luaMonsterTypeAddVoice);

	i.registerMethod("MonsterType", "getLoot", luaMonsterTypeGetLoot);
	i.registerMethod("MonsterType", "addLoot", luaMonsterTypeAddLoot);

	i.registerMethod("MonsterType", "getCreatureEvents", luaMonsterTypeGetCreatureEvents);
	i.registerMethod("MonsterType", "registerEvent", luaMonsterTypeRegisterEvent);

	i.registerMethod("MonsterType", "eventType", luaMonsterTypeEventType);
	i.registerMethod("MonsterType", "onThink", luaMonsterTypeEventOnCallback);
	i.registerMethod("MonsterType", "onAppear", luaMonsterTypeEventOnCallback);
	i.registerMethod("MonsterType", "onDisappear", luaMonsterTypeEventOnCallback);
	i.registerMethod("MonsterType", "onMove", luaMonsterTypeEventOnCallback);
	i.registerMethod("MonsterType", "onSay", luaMonsterTypeEventOnCallback);

	i.registerMethod("MonsterType", "getSummonList", luaMonsterTypeGetSummonList);
	i.registerMethod("MonsterType", "addSummon", luaMonsterTypeAddSummon);

	i.registerMethod("MonsterType", "maxSummons", luaMonsterTypeMaxSummons);

	i.registerMethod("MonsterType", "armor", luaMonsterTypeArmor);
	i.registerMethod("MonsterType", "defense", luaMonsterTypeDefense);
	i.registerMethod("MonsterType", "outfit", luaMonsterTypeOutfit);
	i.registerMethod("MonsterType", "race", luaMonsterTypeRace);
	i.registerMethod("MonsterType", "corpseId", luaMonsterTypeCorpseId);
	i.registerMethod("MonsterType", "manaCost", luaMonsterTypeManaCost);
	i.registerMethod("MonsterType", "baseSpeed", luaMonsterTypeBaseSpeed);
	i.registerMethod("MonsterType", "light", luaMonsterTypeLight);

	i.registerMethod("MonsterType", "staticAttackChance", luaMonsterTypeStaticAttackChance);
	i.registerMethod("MonsterType", "targetDistance", luaMonsterTypeTargetDistance);
	i.registerMethod("MonsterType", "yellChance", luaMonsterTypeYellChance);
	i.registerMethod("MonsterType", "yellSpeedTicks", luaMonsterTypeYellSpeedTicks);
	i.registerMethod("MonsterType", "changeTargetChance", luaMonsterTypeChangeTargetChance);
	i.registerMethod("MonsterType", "changeTargetSpeed", luaMonsterTypeChangeTargetSpeed);

	i.registerMethod("MonsterType", "bestiaryInfo", luaMonsterTypeBestiaryInfo);
}
