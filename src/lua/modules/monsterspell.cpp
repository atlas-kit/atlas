#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaCreateMonsterSpell(lua_State* L)
{
	// MonsterSpell() will create a new Monster Spell
	tfs::lua::pushUserdata(L, new MonsterSpell);
	tfs::lua::setMetatable(L, -1, "MonsterSpell");
	return 1;
}

int luaDeleteMonsterSpell(lua_State* L)
{
	// monsterSpell:delete() monsterSpell:__gc()
	MonsterSpell** monsterSpellPtr = tfs::lua::getRawUserdata<MonsterSpell>(L, 1);
	if (monsterSpellPtr && *monsterSpellPtr) {
		delete *monsterSpellPtr;
		*monsterSpellPtr = nullptr;
	}
	return 0;
}

int luaMonsterSpellSetType(lua_State* L)
{
	// monsterSpell:setType(type)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->name = tfs::lua::getString(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetScriptName(lua_State* L)
{
	// monsterSpell:setScriptName(name)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->scriptName = tfs::lua::getString(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetChance(lua_State* L)
{
	// monsterSpell:setChance(chance)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->chance = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetInterval(lua_State* L)
{
	// monsterSpell:setInterval(interval)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->interval = tfs::lua::getNumber<uint16_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetRange(lua_State* L)
{
	// monsterSpell:setRange(range)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->range = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatValue(lua_State* L)
{
	// monsterSpell:setCombatValue(min, max)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->minCombatValue = tfs::lua::getNumber<int32_t>(L, 2);
		spell->maxCombatValue = tfs::lua::getNumber<int32_t>(L, 3);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatType(lua_State* L)
{
	// monsterSpell:setCombatType(combatType_t)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->combatType = tfs::lua::getNumber<CombatType_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetAttackValue(lua_State* L)
{
	// monsterSpell:setAttackValue(attack, skill)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->attack = tfs::lua::getNumber<int32_t>(L, 2);
		spell->skill = tfs::lua::getNumber<int32_t>(L, 3);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetNeedTarget(lua_State* L)
{
	// monsterSpell:setNeedTarget(bool)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->needTarget = tfs::lua::getBoolean(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetNeedDirection(lua_State* L)
{
	// monsterSpell:setNeedDirection(bool)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->needDirection = tfs::lua::getBoolean(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatLength(lua_State* L)
{
	// monsterSpell:setCombatLength(length)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->length = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatSpread(lua_State* L)
{
	// monsterSpell:setCombatSpread(spread)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->spread = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatRadius(lua_State* L)
{
	// monsterSpell:setCombatRadius(radius)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->radius = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatRing(lua_State* L)
{
	// monsterSpell:setCombatRing(ring)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->ring = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionType(lua_State* L)
{
	// monsterSpell:setConditionType(type)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->conditionType = tfs::lua::getNumber<ConditionType_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionDamage(lua_State* L)
{
	// monsterSpell:setConditionDamage(min, max, start)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->conditionMinDamage = tfs::lua::getNumber<int32_t>(L, 2);
		spell->conditionMaxDamage = tfs::lua::getNumber<int32_t>(L, 3);
		spell->conditionStartDamage = tfs::lua::getNumber<int32_t>(L, 4);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionSpeedChange(lua_State* L)
{
	// monsterSpell:setConditionSpeedChange(minSpeed[, maxSpeed])
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->minSpeedChange = tfs::lua::getNumber<int32_t>(L, 2);
		spell->maxSpeedChange = tfs::lua::getNumber<int32_t>(L, 3, 0);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionDuration(lua_State* L)
{
	// monsterSpell:setConditionDuration(duration)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->duration = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionDrunkenness(lua_State* L)
{
	// monsterSpell:setConditionDrunkenness(drunkenness)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->drunkenness = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetConditionTickInterval(lua_State* L)
{
	// monsterSpell:setConditionTickInterval(interval)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->tickInterval = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatShootEffect(lua_State* L)
{
	// monsterSpell:setCombatShootEffect(effect)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->shoot = tfs::lua::getNumber<ShootType_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetCombatEffect(lua_State* L)
{
	// monsterSpell:setCombatEffect(effect)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		spell->effect = tfs::lua::getNumber<MagicEffectClasses>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaMonsterSpellSetOutfit(lua_State* L)
{
	// monsterSpell:setOutfit(outfit)
	MonsterSpell* spell = tfs::lua::getUserdata<MonsterSpell>(L, 1);
	if (spell) {
		if (lua_istable(L, 2)) {
			spell->outfit = tfs::lua::getOutfit(L, 2);
		} else if (tfs::lua::isNumber(L, 2)) {
			spell->outfit.lookTypeEx = tfs::lua::getNumber<uint16_t>(L, 2);
		} else if (lua_isstring(L, 2)) {
			MonsterType* mType = g_monsters.getMonsterType(tfs::lua::getString(L, 2));
			if (mType) {
				spell->outfit = mType->info.outfit;
			}
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerMonsterSpell(LuaScriptInterface& i)
{
	i.registerClass("MonsterSpell", "", luaCreateMonsterSpell);
	i.registerMetaMethod("MonsterSpell", "__gc", luaDeleteMonsterSpell);
	i.registerMethod("MonsterSpell", "delete", luaDeleteMonsterSpell);

	i.registerMethod("MonsterSpell", "setType", luaMonsterSpellSetType);
	i.registerMethod("MonsterSpell", "setScriptName", luaMonsterSpellSetScriptName);
	i.registerMethod("MonsterSpell", "setChance", luaMonsterSpellSetChance);
	i.registerMethod("MonsterSpell", "setInterval", luaMonsterSpellSetInterval);
	i.registerMethod("MonsterSpell", "setRange", luaMonsterSpellSetRange);
	i.registerMethod("MonsterSpell", "setCombatValue", luaMonsterSpellSetCombatValue);
	i.registerMethod("MonsterSpell", "setCombatType", luaMonsterSpellSetCombatType);
	i.registerMethod("MonsterSpell", "setAttackValue", luaMonsterSpellSetAttackValue);
	i.registerMethod("MonsterSpell", "setNeedTarget", luaMonsterSpellSetNeedTarget);
	i.registerMethod("MonsterSpell", "setNeedDirection", luaMonsterSpellSetNeedDirection);
	i.registerMethod("MonsterSpell", "setCombatLength", luaMonsterSpellSetCombatLength);
	i.registerMethod("MonsterSpell", "setCombatSpread", luaMonsterSpellSetCombatSpread);
	i.registerMethod("MonsterSpell", "setCombatRadius", luaMonsterSpellSetCombatRadius);
	i.registerMethod("MonsterSpell", "setCombatRing", luaMonsterSpellSetCombatRing);
	i.registerMethod("MonsterSpell", "setConditionType", luaMonsterSpellSetConditionType);
	i.registerMethod("MonsterSpell", "setConditionDamage", luaMonsterSpellSetConditionDamage);
	i.registerMethod("MonsterSpell", "setConditionSpeedChange", luaMonsterSpellSetConditionSpeedChange);
	i.registerMethod("MonsterSpell", "setConditionDuration", luaMonsterSpellSetConditionDuration);
	i.registerMethod("MonsterSpell", "setConditionDrunkenness", luaMonsterSpellSetConditionDrunkenness);
	i.registerMethod("MonsterSpell", "setConditionTickInterval", luaMonsterSpellSetConditionTickInterval);
	i.registerMethod("MonsterSpell", "setCombatShootEffect", luaMonsterSpellSetCombatShootEffect);
	i.registerMethod("MonsterSpell", "setCombatEffect", luaMonsterSpellSetCombatEffect);
	i.registerMethod("MonsterSpell", "setOutfit", luaMonsterSpellSetOutfit);
}
