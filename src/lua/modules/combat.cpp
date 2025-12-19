#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaCombatCreate(lua_State* L)
{
	// Combat()
	tfs::lua::pushSharedPtr(L, g_luaEnvironment.createCombatObject(tfs::lua::getScriptEnv()->getScriptInterface()));
	tfs::lua::setMetatable(L, -1, "Combat");
	return 1;
}

int luaCombatDelete(lua_State* L)
{
	Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (combat) {
		combat.reset();
	}
	return 0;
}

int luaCombatSetParameter(lua_State* L)
{
	// combat:setParameter(key, value)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	CombatParam_t key = tfs::lua::getNumber<CombatParam_t>(L, 2);
	uint32_t value;
	if (lua_isboolean(L, 3)) {
		value = tfs::lua::getBoolean(L, 3) ? 1 : 0;
	} else {
		value = tfs::lua::getNumber<uint32_t>(L, 3);
	}
	combat->setParam(key, value);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaCombatGetParameter(lua_State* L)
{
	// combat:getParameter(key)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	int32_t value = combat->getParam(tfs::lua::getNumber<CombatParam_t>(L, 2));
	if (value == std::numeric_limits<int32_t>().max()) {
		lua_pushnil(L);
		return 1;
	}

	tfs::lua::pushNumber(L, value);
	return 1;
}

int luaCombatSetFormula(lua_State* L)
{
	// combat:setFormula(type, mina, minb, maxa, maxb)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	formulaType_t type = tfs::lua::getNumber<formulaType_t>(L, 2);
	double mina = tfs::lua::getNumber<double>(L, 3);
	double minb = tfs::lua::getNumber<double>(L, 4);
	double maxa = tfs::lua::getNumber<double>(L, 5);
	double maxb = tfs::lua::getNumber<double>(L, 6);
	combat->setPlayerCombatValues(type, mina, minb, maxa, maxb);
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaCombatSetArea(lua_State* L)
{
	// combat:setArea(area)
	if (tfs::lua::getScriptEnv()->getScriptId() != EVENT_ID_LOADING) {
		tfs::lua::reportError(L, "This function can only be used while loading the script.");
		lua_pushnil(L);
		return 1;
	}

	const AreaCombat* area = g_luaEnvironment.getAreaObject(tfs::lua::getNumber<uint32_t>(L, 2));
	if (!area) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_AREA_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	combat->setArea(new AreaCombat(*area));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaCombatAddCondition(lua_State* L)
{
	// combat:addCondition(condition)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	Condition* condition = tfs::lua::getUserdata<Condition>(L, 2);
	if (condition) {
		combat->addCondition(condition->clone());
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaCombatClearConditions(lua_State* L)
{
	// combat:clearConditions()
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	combat->clearConditions();
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaCombatSetCallback(lua_State* L)
{
	// combat:setCallback(key, function)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	CallBackParam_t key = tfs::lua::getNumber<CallBackParam_t>(L, 2);
	if (!combat->setCallback(key)) {
		lua_pushnil(L);
		return 1;
	}

	CallBack* callback = combat->getCallback(key);
	if (!callback) {
		lua_pushnil(L);
		return 1;
	}

	const std::string& function = tfs::lua::getString(L, 3);
	tfs::lua::pushBoolean(L, callback->loadCallBack(tfs::lua::getScriptEnv()->getScriptInterface(), function));
	return 1;
}

int luaCombatSetOrigin(lua_State* L)
{
	// combat:setOrigin(origin)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	combat->setOrigin(tfs::lua::getNumber<CombatOrigin>(L, 2));
	tfs::lua::pushBoolean(L, true);
	return 1;
}

int luaCombatExecute(lua_State* L)
{
	// combat:execute(creature, variant)
	const Combat_ptr& combat = tfs::lua::getSharedPtr<Combat>(L, 1);
	if (!combat) {
		tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	if (lua_isuserdata(L, 2)) {
		const auto type = tfs::lua::getUserdataType(L, 2);
		if (type != tfs::lua::LuaData_Player && type != tfs::lua::LuaData_Monster && type != tfs::lua::LuaData_Npc) {
			tfs::lua::pushBoolean(L, false);
			return 1;
		}
	}

	const auto& creature = tfs::lua::getCreature(L, 2);

	const LuaVariant& variant = tfs::lua::getVariant(L, 3);
	switch (variant.type()) {
		case VARIANT_NUMBER: {
			const auto& target = g_game.getCreatureByID(variant.getNumber());
			if (!target) {
				tfs::lua::pushBoolean(L, false);
				return 1;
			}

			if (combat->hasArea()) {
				combat->doCombat(creature, target->getPosition());
			} else {
				combat->doCombat(creature, target);
			}
			break;
		}

		case VARIANT_POSITION: {
			combat->doCombat(creature, variant.getPosition());
			break;
		}

		case VARIANT_TARGETPOSITION: {
			if (combat->hasArea()) {
				combat->doCombat(creature, variant.getTargetPosition());
			} else {
				combat->postCombatEffects(creature, variant.getTargetPosition());
				g_game.addMagicEffect(variant.getTargetPosition(), CONST_ME_POFF);
			}
			break;
		}

		case VARIANT_STRING: {
			const auto& target = g_game.getPlayerByName(variant.getString());
			if (!target) {
				tfs::lua::pushBoolean(L, false);
				return 1;
			}

			combat->doCombat(creature, target);
			break;
		}

		case VARIANT_NONE: {
			tfs::lua::reportError(L, tfs::lua::getErrorDesc(tfs::lua::LUA_ERROR_VARIANT_NOT_FOUND));
			tfs::lua::pushBoolean(L, false);
			return 1;
		}

		default: {
			break;
		}
	}

	tfs::lua::pushBoolean(L, true);
	return 1;
}

} // namespace

void tfs::lua::registerCombat(LuaScriptInterface& lsi)
{
	lsi.registerClass("Combat", "", luaCombatCreate);
	lsi.registerMetaMethod("Combat", "__eq", tfs::lua::luaUserdataCompare);
	lsi.registerMetaMethod("Combat", "__gc", luaCombatDelete);
	lsi.registerMethod("Combat", "delete", luaCombatDelete);

	lsi.registerMethod("Combat", "setParameter", luaCombatSetParameter);
	lsi.registerMethod("Combat", "getParameter", luaCombatGetParameter);

	lsi.registerMethod("Combat", "setFormula", luaCombatSetFormula);

	lsi.registerMethod("Combat", "setArea", luaCombatSetArea);
	lsi.registerMethod("Combat", "addCondition", luaCombatAddCondition);
	lsi.registerMethod("Combat", "clearConditions", luaCombatClearConditions);
	lsi.registerMethod("Combat", "setCallback", luaCombatSetCallback);
	lsi.registerMethod("Combat", "setOrigin", luaCombatSetOrigin);

	lsi.registerMethod("Combat", "execute", luaCombatExecute);
}
