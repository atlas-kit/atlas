// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "appearances.h"

#include "appearances.pb.h"

#include <fstream>
#include <iostream>
#include <limits>

namespace {

// Garante que um id vindo do protobuf (uint32) cabe no espaco uint16 usado
// como indice de item; ids fora do intervalo viram 0 (invalido)
uint16_t toItemId(uint32_t value)
{
	return value <= std::numeric_limits<uint16_t>::max() ? static_cast<uint16_t>(value) : 0;
}

} // namespace

Appearances g_appearances;

bool Appearances::loadFromFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "[Error - Appearances::loadFromFile] Cannot open file: " << filename << std::endl;
		return false;
	}

	// Validate file is not empty
	file.seekg(0, std::ios::end);
	auto fileSize = file.tellg();
	if (fileSize <= 0) {
		std::cout << "[Error - Appearances::loadFromFile] File is empty: " << filename << std::endl;
		return false;
	}
	file.seekg(0, std::ios::beg);

	atlas::protobuf::appearances::Appearances proto;
	if (!proto.ParseFromIstream(&file)) {
		std::cout << "[Error - Appearances::loadFromFile] Failed to parse protobuf from: " << filename << std::endl;
		return false;
	}

	// A truncated or wrong file can still parse as a valid (near-empty)
	// protobuf; refuse to continue without object appearances
	if (proto.object_size() == 0) {
		std::cout << "[Error - Appearances::loadFromFile] No object appearances in: " << filename
		          << " (wrong or truncated file?)" << std::endl;
		return false;
	}

	// Clear existing data
	objects.clear();
	outfits.clear();
	effects.clear();
	missiles.clear();

	// Reset so a reload cannot keep stale values
	goldCoinId = 0;
	platinumCoinId = 0;
	crystalCoinId = 0;
	tibiaCoinId = 0;
	stampedLetterId = 0;
	supplyStashId = 0;
	standardRewardChestId = 0;
	blankImbuementScrollId = 0;

	size_t skippedObjects = 0;

	// Load objects (items)
	for (const auto& appearance : proto.object()) {
		if (!appearance.has_id() || appearance.id() > std::numeric_limits<uint16_t>::max()) {
			++skippedObjects;
			continue;
		}
		AppearanceInfo info;
		parseAppearance(appearance, info);
		objects[info.id] = std::move(info);
	}

	// Load outfits
	for (const auto& appearance : proto.outfit()) {
		if (!appearance.has_id() || appearance.id() > std::numeric_limits<uint16_t>::max()) {
			++skippedObjects;
			continue;
		}
		AppearanceInfo info;
		parseAppearance(appearance, info);
		outfits[info.id] = std::move(info);
	}

	// Load effects
	for (const auto& appearance : proto.effect()) {
		if (!appearance.has_id() || appearance.id() > std::numeric_limits<uint16_t>::max()) {
			++skippedObjects;
			continue;
		}
		AppearanceInfo info;
		parseAppearance(appearance, info);
		effects[info.id] = std::move(info);
	}

	// Load missiles
	for (const auto& appearance : proto.missile()) {
		if (!appearance.has_id() || appearance.id() > std::numeric_limits<uint16_t>::max()) {
			++skippedObjects;
			continue;
		}
		AppearanceInfo info;
		parseAppearance(appearance, info);
		missiles[info.id] = std::move(info);
	}

	// Load special meaning IDs
	if (proto.has_special_meaning_appearance_ids()) {
		const auto& special = proto.special_meaning_appearance_ids();
		if (special.has_gold_coin_id()) {
			goldCoinId = toItemId(special.gold_coin_id());
		}
		if (special.has_platinum_coin_id()) {
			platinumCoinId = toItemId(special.platinum_coin_id());
		}
		if (special.has_crystal_coin_id()) {
			crystalCoinId = toItemId(special.crystal_coin_id());
		}
		if (special.has_tibia_coin_id()) {
			tibiaCoinId = toItemId(special.tibia_coin_id());
		}
		if (special.has_stamped_letter_id()) {
			stampedLetterId = toItemId(special.stamped_letter_id());
		}
		if (special.has_supply_stash_id()) {
			supplyStashId = toItemId(special.supply_stash_id());
		}
		if (special.has_standard_reward_chest_id()) {
			standardRewardChestId = toItemId(special.standard_reward_chest_id());
		}
		if (special.has_blank_imbuement_scroll_id()) {
			blankImbuementScrollId = toItemId(special.blank_imbuement_scroll_id());
		}
	}

	// Log loading statistics
	std::cout << ">> Loaded appearances from: " << filename << " (" << fileSize << " bytes)" << std::endl;
	std::cout << ">> Loaded " << objects.size() << " object appearances" << std::endl;
	std::cout << ">> Loaded " << outfits.size() << " outfit appearances" << std::endl;
	std::cout << ">> Loaded " << effects.size() << " effect appearances" << std::endl;
	std::cout << ">> Loaded " << missiles.size() << " missile appearances" << std::endl;

	if (skippedObjects > 0) {
		std::cout << ">> Warning: skipped " << skippedObjects << " appearances without ID" << std::endl;
	}

	// Log extended field statistics
	size_t weaponCount = 0, levelReqCount = 0, vocationReqCount = 0;
	size_t imbueableCount = 0, expireCount = 0, npcSaleCount = 0;
	for (const auto& [id, info] : objects) {
		if (info.weaponType > 0) ++weaponCount;
		if (info.minimumLevel > 0) ++levelReqCount;
		if (!info.restrictedVocations.empty()) ++vocationReqCount;
		if (info.imbueableSlotCount > 0) ++imbueableCount;
		if (info.wearout || info.clockExpire || info.expire) ++expireCount;
		if (!info.npcSaleData.empty()) ++npcSaleCount;
	}
	std::cout << ">> Appearance flags: " << weaponCount << " weapons, " << levelReqCount << " level-restricted, "
	          << vocationReqCount << " vocation-restricted, " << imbueableCount << " imbueable, " << expireCount
	          << " expirable, " << npcSaleCount << " with NPC data" << std::endl;

	return true;
}

void Appearances::parseAppearance(const atlas::protobuf::appearances::Appearance& proto, AppearanceInfo& info)
{
	info.id = toItemId(proto.id());

	if (proto.has_name()) {
		info.name = proto.name();
	}

	if (proto.has_description()) {
		info.description = proto.description();
	}

	if (proto.has_flags()) {
		parseFlags(proto.flags(), info);
	}
}

void Appearances::parseFlags(const atlas::protobuf::appearances::AppearanceFlags& flags, AppearanceInfo& info)
{
	// Ground/Bank (waypoints = ground speed)
	if (flags.has_bank()) {
		info.isGround = true;
		if (flags.bank().has_waypoints()) {
			info.groundSpeed = flags.bank().waypoints();
		}
	}

	// Basic boolean flags
	info.isGroundBorder = flags.has_clip() && flags.clip();
	info.isOnBottom = flags.has_bottom() && flags.bottom();
	info.isOnTop = flags.has_top() && flags.top();
	info.isContainer = flags.has_container() && flags.container();
	info.isStackable = flags.has_cumulative() && flags.cumulative();
	info.isUsable = flags.has_usable() && flags.usable();
	info.isForceUse = flags.has_forceuse() && flags.forceuse();
	info.isMultiUse = flags.has_multiuse() && flags.multiuse();
	info.isFluidPool = flags.has_liquidpool() && flags.liquidpool();
	info.isUnpassable = flags.has_unpass() && flags.unpass();
	info.isUnmovable = flags.has_unmove() && flags.unmove();
	info.isBlockMissile = flags.has_unsight() && flags.unsight();
	info.isBlockPath = flags.has_avoid() && flags.avoid();
	info.isPickupable = flags.has_take() && flags.take();
	info.isFluidContainer = flags.has_liquidcontainer() && flags.liquidcontainer();
	info.isHangable = flags.has_hang() && flags.hang();
	if (flags.has_hook() && flags.hook().has_direction()) {
		info.hookDirection = static_cast<uint8_t>(flags.hook().direction());
	}
	info.isRotatable = flags.has_rotate() && flags.rotate();
	info.isDontHide = flags.has_dont_hide() && flags.dont_hide();
	info.isTranslucent = flags.has_translucent() && flags.translucent();
	info.isLyingObject = flags.has_lying_object() && flags.lying_object();
	info.isAnimateAlways = flags.has_animate_always() && flags.animate_always();
	info.isFullGround = flags.has_fullbank() && flags.fullbank();
	info.isIgnoreLook = flags.has_ignore_look() && flags.ignore_look();
	info.isWrap = flags.has_wrap() && flags.wrap();
	info.isUnwrap = flags.has_unwrap() && flags.unwrap();
	info.isTopEffect = flags.has_topeffect() && flags.topeffect();
	info.isCorpse = flags.has_corpse() && flags.corpse();
	info.isPlayerCorpse = flags.has_player_corpse() && flags.player_corpse();
	info.isAmmo = flags.has_ammo() && flags.ammo();
	info.isShowOffSocket = flags.has_show_off_socket() && flags.show_off_socket();
	info.isReportable = flags.has_reportable() && flags.reportable();

	// Write
	if (flags.has_write()) {
		info.isWritable = true;
		if (flags.write().has_max_text_length()) {
			info.maxTextLength = static_cast<uint16_t>(flags.write().max_text_length());
		}
	}

	// Write once
	if (flags.has_write_once()) {
		info.isWritableOnce = true;
		if (flags.write_once().has_max_text_length_once()) {
			info.maxTextLength = static_cast<uint16_t>(flags.write_once().max_text_length_once());
		}
	}

	// Light
	if (flags.has_light()) {
		info.hasLight = true;
		if (flags.light().has_brightness()) {
			info.lightLevel = static_cast<uint8_t>(flags.light().brightness());
		}
		if (flags.light().has_color()) {
			info.lightColor = static_cast<uint8_t>(flags.light().color());
		}
	}

	// Shift (offset)
	if (flags.has_shift()) {
		info.hasOffset = true;
		if (flags.shift().has_x()) {
			info.shiftX = static_cast<int16_t>(flags.shift().x());
		}
		if (flags.shift().has_y()) {
			info.shiftY = static_cast<int16_t>(flags.shift().y());
		}
	}

	// Height (elevation)
	if (flags.has_height()) {
		info.hasElevation = true;
		if (flags.height().has_elevation()) {
			info.elevation = static_cast<uint16_t>(flags.height().elevation());
		}
	}

	// Automap
	if (flags.has_automap()) {
		info.hasAutomapColor = true;
		if (flags.automap().has_color()) {
			info.automapColor = static_cast<uint16_t>(flags.automap().color());
		}
	}

	// Lenshelp
	if (flags.has_lenshelp()) {
		info.isLensHelp = true;
		if (flags.lenshelp().has_id()) {
			info.lensHelp = static_cast<uint8_t>(flags.lenshelp().id());
		}
	}

	// Clothes (slot)
	if (flags.has_clothes()) {
		info.isCloth = true;
		if (flags.clothes().has_slot()) {
			info.clothSlot = static_cast<uint8_t>(flags.clothes().slot());
		}
	}

	// Default action
	if (flags.has_default_action()) {
		info.hasDefaultAction = true;
		if (flags.default_action().has_action()) {
			info.defaultAction = static_cast<uint8_t>(flags.default_action().action());
		}
	}

	// Market
	if (flags.has_market()) {
		const auto& market = flags.market();
		if (market.has_category()) {
			info.marketCategory = static_cast<uint16_t>(market.category());
		}
		if (market.has_trade_as_object_id()) {
			info.marketTradeAs = toItemId(market.trade_as_object_id());
		}
		if (market.has_show_as_object_id()) {
			info.marketShowAs = toItemId(market.show_as_object_id());
		}
	}

	// Upgrade classification
	if (flags.has_upgradeclassification()) {
		if (flags.upgradeclassification().has_upgrade_classification()) {
			info.classification = static_cast<uint8_t>(flags.upgradeclassification().upgrade_classification());
		}
	}

	// Visual flags
	info.noMovementAnimation = flags.has_no_movement_animation() && flags.no_movement_animation();
	info.reverseAddonsEast = flags.has_reverse_addons_east() && flags.reverse_addons_east();
	info.reverseAddonsWest = flags.has_reverse_addons_west() && flags.reverse_addons_west();
	info.reverseAddonsSouth = flags.has_reverse_addons_south() && flags.reverse_addons_south();
	info.reverseAddonsNorth = flags.has_reverse_addons_north() && flags.reverse_addons_north();

	// Expiration flags
	info.wearout = flags.has_wearout() && flags.wearout();
	info.clockExpire = flags.has_clockexpire() && flags.clockexpire();
	info.expire = flags.has_expire() && flags.expire();
	info.expireStop = flags.has_expirestop() && flags.expirestop();

	// Decoration kit
	info.decoItemKit = flags.has_deco_item_kit() && flags.deco_item_kit();

	// Dual wielding
	info.dualWielding = flags.has_dual_wielding() && flags.dual_wielding();

	// Weapon type
	if (flags.has_weapon_type()) {
		info.weaponType = static_cast<uint8_t>(flags.weapon_type());
	}

	// Minimum level
	if (flags.has_minimum_level()) {
		info.minimumLevel = flags.minimum_level();
	}

	// Vocation restrictions
	for (int i = 0; i < flags.restrict_to_vocation_size(); ++i) {
		info.restrictedVocations.push_back(static_cast<int32_t>(flags.restrict_to_vocation(i)));
	}

	// Imbueable
	if (flags.has_imbueable()) {
		if (flags.imbueable().has_slot_count()) {
			info.imbueableSlotCount = flags.imbueable().slot_count();
		}
	}

	// Skill wheel gem
	if (flags.has_skillwheel_gem()) {
		if (flags.skillwheel_gem().has_gem_quality_id()) {
			info.gemQualityId = flags.skillwheel_gem().gem_quality_id();
		}
		if (flags.skillwheel_gem().has_vocation_id()) {
			info.gemVocationId = flags.skillwheel_gem().vocation_id();
		}
	}

	// Proficiency
	if (flags.has_proficiency()) {
		if (flags.proficiency().has_proficiency_id()) {
			info.proficiencyId = flags.proficiency().proficiency_id();
		}
	}

	// Changed to expire
	if (flags.has_changedtoexpire()) {
		if (flags.changedtoexpire().has_former_object_typeid()) {
			info.formerObjectTypeId = flags.changedtoexpire().former_object_typeid();
		}
	}

	// Cyclopedia
	if (flags.has_cyclopediaitem()) {
		if (flags.cyclopediaitem().has_cyclopedia_type()) {
			info.cyclopediaType = flags.cyclopediaitem().cyclopedia_type();
		}
	}

	// NPC sale data
	for (int i = 0; i < flags.npcsaledata_size(); ++i) {
		const auto& npc = flags.npcsaledata(i);
		NpcSaleInfo sale;
		if (npc.has_name()) {
			sale.name = npc.name();
		}
		if (npc.has_location()) {
			sale.location = npc.location();
		}
		if (npc.has_sale_price()) {
			sale.salePrice = npc.sale_price();
		}
		if (npc.has_buy_price()) {
			sale.buyPrice = npc.buy_price();
		}
		if (npc.has_currency_object_type_id()) {
			sale.currencyObjectTypeId = npc.currency_object_type_id();
		}
		if (npc.has_currency_quest_flag_display_name()) {
			sale.currencyQuestFlagDisplayName = npc.currency_quest_flag_display_name();
		}
		info.npcSaleData.push_back(std::move(sale));
	}
}

const AppearanceInfo* Appearances::getObjectAppearance(uint16_t id) const
{
	auto it = objects.find(id);
	if (it != objects.end()) {
		return &it->second;
	}
	return nullptr;
}

const AppearanceInfo* Appearances::getOutfitAppearance(uint16_t id) const
{
	auto it = outfits.find(id);
	if (it != outfits.end()) {
		return &it->second;
	}
	return nullptr;
}

const AppearanceInfo* Appearances::getEffectAppearance(uint16_t id) const
{
	auto it = effects.find(id);
	if (it != effects.end()) {
		return &it->second;
	}
	return nullptr;
}

const AppearanceInfo* Appearances::getMissileAppearance(uint16_t id) const
{
	auto it = missiles.find(id);
	if (it != missiles.end()) {
		return &it->second;
	}
	return nullptr;
}
