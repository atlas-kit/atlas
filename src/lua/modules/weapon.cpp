#include "../api.h"
#include "../meta.h"
#include "../register.h"
#include "../script.h"

namespace {

int luaCreateWeapon(lua_State* L)
{
	// Weapon(type)
	if (tfs::lua::getScriptEnv()->getScriptInterface() != &g_scripts->getScriptInterface()) {
		tfs::lua::reportError(L, "Weapons can only be registered in the Scripts interface.");
		lua_pushnil(L);
		return 1;
	}

	WeaponType_t type = tfs::lua::getNumber<WeaponType_t>(L, 2);
	switch (type) {
		case WEAPON_SWORD:
		case WEAPON_AXE:
		case WEAPON_CLUB: {
			WeaponMelee* weapon = new WeaponMelee(tfs::lua::getScriptEnv()->getScriptInterface());
			tfs::lua::pushUserdata(L, weapon);
			tfs::lua::setMetatable(L, -1, "Weapon");
			weapon->weaponType = type;
			weapon->fromLua = true;
			break;
		}
		case WEAPON_DISTANCE:
		case WEAPON_AMMO: {
			WeaponDistance* weapon = new WeaponDistance(tfs::lua::getScriptEnv()->getScriptInterface());
			tfs::lua::pushUserdata(L, weapon);
			tfs::lua::setMetatable(L, -1, "Weapon");
			weapon->weaponType = type;
			weapon->fromLua = true;
			break;
		}
		case WEAPON_WAND: {
			WeaponWand* weapon = new WeaponWand(tfs::lua::getScriptEnv()->getScriptInterface());
			tfs::lua::pushUserdata(L, weapon);
			tfs::lua::setMetatable(L, -1, "Weapon");
			weapon->weaponType = type;
			weapon->fromLua = true;
			break;
		}
		default: {
			lua_pushnil(L);
			break;
		}
	}
	return 1;
}

int luaWeaponAction(lua_State* L)
{
	// weapon:action(callback)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		std::string typeName = tfs::lua::getString(L, 2);
		std::string tmpStr = boost::algorithm::to_lower_copy(typeName);
		if (tmpStr == "removecount") {
			weapon->action = WEAPONACTION_REMOVECOUNT;
		} else if (tmpStr == "removecharge") {
			weapon->action = WEAPONACTION_REMOVECHARGE;
		} else if (tmpStr == "move") {
			weapon->action = WEAPONACTION_MOVE;
		} else {
			std::cout << "Error: [Weapon::action] No valid action " << typeName << '\n';
			tfs::lua::pushBoolean(L, false);
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponRegister(lua_State* L)
{
	// weapon:register()
	Weapon** weaponPtr = tfs::lua::getRawUserdata<Weapon>(L, 1);
	if (!weaponPtr) {
		lua_pushnil(L);
		return 1;
	}

	if (auto* weapon = *weaponPtr) {
		if (weapon->weaponType == WEAPON_DISTANCE || weapon->weaponType == WEAPON_AMMO) {
			weapon = tfs::lua::getUserdata<WeaponDistance>(L, 1);
		} else if (weapon->weaponType == WEAPON_WAND) {
			weapon = tfs::lua::getUserdata<WeaponWand>(L, 1);
		} else {
			weapon = tfs::lua::getUserdata<WeaponMelee>(L, 1);
		}

		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.weaponType = weapon->weaponType;

		if (weapon->getWieldInfo() != 0) {
			it.wieldInfo = weapon->getWieldInfo();
			it.vocationString = weapon->getVocationString();
			it.minReqLevel = weapon->getReqLevel();
			it.minReqMagicLevel = weapon->getReqMagLv();
		}

		weapon->configureWeapon(it);
		tfs::lua::pushBoolean(L, g_weapons->registerLuaEvent(weapon));
		*weaponPtr = nullptr; // Remove luascript reference
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponOnUseWeapon(lua_State* L)
{
	// weapon:onUseWeapon(callback)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		if (!weapon->loadCallback()) {
			tfs::lua::pushBoolean(L, false);
			return 1;
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponUnproperly(lua_State* L)
{
	// weapon:wieldUnproperly(bool)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setWieldUnproperly(tfs::lua::getBoolean(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponLevel(lua_State* L)
{
	// weapon:level(lvl)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setRequiredLevel(tfs::lua::getNumber<uint32_t>(L, 2));
		weapon->setWieldInfo(WIELDINFO_LEVEL);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponMagicLevel(lua_State* L)
{
	// weapon:magicLevel(lvl)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setRequiredMagLevel(tfs::lua::getNumber<uint32_t>(L, 2));
		weapon->setWieldInfo(WIELDINFO_MAGLV);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponMana(lua_State* L)
{
	// weapon:mana(mana)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setMana(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponManaPercent(lua_State* L)
{
	// weapon:manaPercent(percent)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setManaPercent(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponHealth(lua_State* L)
{
	// weapon:health(health)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setHealth(tfs::lua::getNumber<int32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponHealthPercent(lua_State* L)
{
	// weapon:healthPercent(percent)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setHealthPercent(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponSoul(lua_State* L)
{
	// weapon:soul(soul)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setSoul(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponBreakChance(lua_State* L)
{
	// weapon:breakChance(percent)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setBreakChance(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponWandDamage(lua_State* L)
{
	// weapon:damage(damage[min, max]) only use this if the weapon is a wand!
	WeaponWand* weapon = tfs::lua::getUserdata<WeaponWand>(L, 1);
	if (weapon) {
		weapon->setMinChange(tfs::lua::getNumber<uint32_t>(L, 2));
		if (lua_gettop(L) > 2) {
			weapon->setMaxChange(tfs::lua::getNumber<uint32_t>(L, 3));
		} else {
			weapon->setMaxChange(tfs::lua::getNumber<uint32_t>(L, 2));
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponElement(lua_State* L)
{
	// weapon:element(combatType)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		if (!tfs::lua::getNumber<CombatType_t>(L, 2)) {
			std::string element = tfs::lua::getString(L, 2);
			std::string tmpStrValue = boost::algorithm::to_lower_copy(element);
			if (tmpStrValue == "earth") {
				weapon->params.combatType = COMBAT_EARTHDAMAGE;
			} else if (tmpStrValue == "ice") {
				weapon->params.combatType = COMBAT_ICEDAMAGE;
			} else if (tmpStrValue == "energy") {
				weapon->params.combatType = COMBAT_ENERGYDAMAGE;
			} else if (tmpStrValue == "fire") {
				weapon->params.combatType = COMBAT_FIREDAMAGE;
			} else if (tmpStrValue == "death") {
				weapon->params.combatType = COMBAT_DEATHDAMAGE;
			} else if (tmpStrValue == "holy") {
				weapon->params.combatType = COMBAT_HOLYDAMAGE;
			} else {
				std::cout << "[Warning - weapon:element] Type \"" << element << "\" does not exist.\n";
			}
		} else {
			weapon->params.combatType = tfs::lua::getNumber<CombatType_t>(L, 2);
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponPremium(lua_State* L)
{
	// weapon:premium(bool)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setNeedPremium(tfs::lua::getBoolean(L, 2));
		weapon->setWieldInfo(WIELDINFO_PREMIUM);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponVocation(lua_State* L)
{
	// weapon:vocation(vocName[, showInDescription = false, lastVoc = false])
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->addVocationWeaponSet(tfs::lua::getString(L, 2));
		weapon->setWieldInfo(WIELDINFO_VOCREQ);
		std::string tmp;
		bool showInDescription = tfs::lua::getBoolean(L, 3, false);
		bool lastVoc = tfs::lua::getBoolean(L, 4, false);

		if (showInDescription) {
			if (weapon->getVocationString().empty()) {
				tmp = boost::algorithm::to_lower_copy(tfs::lua::getString(L, 2));
				tmp += "s";
				weapon->setVocationString(tmp);
			} else {
				tmp = weapon->getVocationString();
				if (lastVoc) {
					tmp += " and ";
				} else {
					tmp += ", ";
				}
				tmp += boost::algorithm::to_lower_copy(tfs::lua::getString(L, 2));
				tmp += "s";
				weapon->setVocationString(tmp);
			}
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponId(lua_State* L)
{
	// weapon:id(id)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		weapon->setID(tfs::lua::getNumber<uint32_t>(L, 2));
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponAttack(lua_State* L)
{
	// weapon:attack(atk)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.attack = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponDefense(lua_State* L)
{
	// weapon:defense(defense[, extraDefense])
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.defense = tfs::lua::getNumber<int32_t>(L, 2);
		if (lua_gettop(L) > 2) {
			it.extraDefense = tfs::lua::getNumber<int32_t>(L, 3);
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponRange(lua_State* L)
{
	// weapon:range(range)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.shootRange = tfs::lua::getNumber<uint8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponCharges(lua_State* L)
{
	// weapon:charges(charges[, showCharges = true])
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		bool showCharges = tfs::lua::getBoolean(L, 3, true);
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);

		it.charges = tfs::lua::getNumber<uint32_t>(L, 2);
		it.showCharges = showCharges;
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponDuration(lua_State* L)
{
	// weapon:duration(duration[, showDuration = true])
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		bool showDuration = tfs::lua::getBoolean(L, 3, true);
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);

		if (lua_istable(L, 2)) {
			it.decayTimeMin = tfs::lua::getField<uint32_t>(L, 2, "min");
			it.decayTimeMax = tfs::lua::getField<uint32_t>(L, 2, "max");
		} else {
			it.decayTimeMin = tfs::lua::getNumber<uint32_t>(L, 2);
		}

		it.showDuration = showDuration;
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponDecayTo(lua_State* L)
{
	// weapon:decayTo([itemid = 0])
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t itemid = tfs::lua::getNumber<uint16_t>(L, 2, 0);
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);

		it.decayTo = itemid;
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponTransformEquipTo(lua_State* L)
{
	// weapon:transformEquipTo(itemid)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.transformEquipTo = tfs::lua::getNumber<uint16_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponTransformDeEquipTo(lua_State* L)
{
	// weapon:transformDeEquipTo(itemid)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.transformDeEquipTo = tfs::lua::getNumber<uint16_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponShootType(lua_State* L)
{
	// weapon:shootType(type)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.shootType = tfs::lua::getNumber<ShootType_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponSlotType(lua_State* L)
{
	// weapon:slotType(slot)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		std::string slot = tfs::lua::getString(L, 2);

		if (slot == "two-handed") {
			it.slotPosition |= SLOTP_TWO_HAND;
		} else {
			it.slotPosition |= SLOTP_HAND;
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponAmmoType(lua_State* L)
{
	// weapon:ammoType(type)
	WeaponDistance* weapon = tfs::lua::getUserdata<WeaponDistance>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		std::string type = tfs::lua::getString(L, 2);

		if (type == "arrow") {
			it.ammoType = AMMO_ARROW;
		} else if (type == "bolt") {
			it.ammoType = AMMO_BOLT;
		} else {
			std::cout << "[Warning - weapon:ammoType] Type \"" << type << "\" does not exist.\n";
			lua_pushnil(L);
			return 1;
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponHitChance(lua_State* L)
{
	// weapon:hitChance(chance)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.hitChance = tfs::lua::getNumber<int8_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponMaxHitChance(lua_State* L)
{
	// weapon:maxHitChance(max)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.maxHitChance = tfs::lua::getNumber<int32_t>(L, 2);
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int luaWeaponExtraElement(lua_State* L)
{
	// weapon:extraElement(atk, combatType)
	Weapon* weapon = tfs::lua::getUserdata<Weapon>(L, 1);
	if (weapon) {
		uint16_t id = weapon->getID();
		ItemType& it = Item::items.getItemType(id);
		it.abilities.get()->elementDamage = tfs::lua::getNumber<uint16_t>(L, 2);

		if (!tfs::lua::getNumber<CombatType_t>(L, 3)) {
			std::string element = tfs::lua::getString(L, 3);
			std::string tmpStrValue = boost::algorithm::to_lower_copy(element);
			if (tmpStrValue == "earth") {
				it.abilities.get()->elementType = COMBAT_EARTHDAMAGE;
			} else if (tmpStrValue == "ice") {
				it.abilities.get()->elementType = COMBAT_ICEDAMAGE;
			} else if (tmpStrValue == "energy") {
				it.abilities.get()->elementType = COMBAT_ENERGYDAMAGE;
			} else if (tmpStrValue == "fire") {
				it.abilities.get()->elementType = COMBAT_FIREDAMAGE;
			} else if (tmpStrValue == "death") {
				it.abilities.get()->elementType = COMBAT_DEATHDAMAGE;
			} else if (tmpStrValue == "holy") {
				it.abilities.get()->elementType = COMBAT_HOLYDAMAGE;
			} else {
				std::cout << "[Warning - weapon:extraElement] Type \"" << element << "\" does not exist.\n";
			}
		} else {
			it.abilities.get()->elementType = tfs::lua::getNumber<CombatType_t>(L, 3);
		}
		tfs::lua::pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

} // namespace

void tfs::lua::registerWeapon(LuaScriptInterface& i)
{
	i.registerClass("Weapon", "", luaCreateWeapon);
	i.registerMethod("Weapon", "action", luaWeaponAction);
	i.registerMethod("Weapon", "register", luaWeaponRegister);
	i.registerMethod("Weapon", "id", luaWeaponId);
	i.registerMethod("Weapon", "level", luaWeaponLevel);
	i.registerMethod("Weapon", "magicLevel", luaWeaponMagicLevel);
	i.registerMethod("Weapon", "mana", luaWeaponMana);
	i.registerMethod("Weapon", "manaPercent", luaWeaponManaPercent);
	i.registerMethod("Weapon", "health", luaWeaponHealth);
	i.registerMethod("Weapon", "healthPercent", luaWeaponHealthPercent);
	i.registerMethod("Weapon", "soul", luaWeaponSoul);
	i.registerMethod("Weapon", "breakChance", luaWeaponBreakChance);
	i.registerMethod("Weapon", "premium", luaWeaponPremium);
	i.registerMethod("Weapon", "wieldUnproperly", luaWeaponUnproperly);
	i.registerMethod("Weapon", "vocation", luaWeaponVocation);
	i.registerMethod("Weapon", "onUseWeapon", luaWeaponOnUseWeapon);
	i.registerMethod("Weapon", "element", luaWeaponElement);
	i.registerMethod("Weapon", "attack", luaWeaponAttack);
	i.registerMethod("Weapon", "defense", luaWeaponDefense);
	i.registerMethod("Weapon", "range", luaWeaponRange);
	i.registerMethod("Weapon", "charges", luaWeaponCharges);
	i.registerMethod("Weapon", "duration", luaWeaponDuration);
	i.registerMethod("Weapon", "decayTo", luaWeaponDecayTo);
	i.registerMethod("Weapon", "transformEquipTo", luaWeaponTransformEquipTo);
	i.registerMethod("Weapon", "transformDeEquipTo", luaWeaponTransformDeEquipTo);
	i.registerMethod("Weapon", "slotType", luaWeaponSlotType);
	i.registerMethod("Weapon", "hitChance", luaWeaponHitChance);
	i.registerMethod("Weapon", "extraElement", luaWeaponExtraElement);

	// exclusively for distance weapons
	i.registerMethod("Weapon", "ammoType", luaWeaponAmmoType);
	i.registerMethod("Weapon", "maxHitChance", luaWeaponMaxHitChance);

	// exclusively for wands
	i.registerMethod("Weapon", "damage", luaWeaponWandDamage);

	// exclusively for wands & distance weapons
	i.registerMethod("Weapon", "shootType", luaWeaponShootType);
}
