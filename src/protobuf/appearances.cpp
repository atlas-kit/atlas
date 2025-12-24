// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../otpch.h"

#include "appearances.h"

#include "appearances.pb.h"

#include <fstream>
#include <iostream>

Appearances g_appearances;

bool Appearances::loadFromFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "[Error - Appearances::loadFromFile] Cannot open file: " << filename << std::endl;
		return false;
	}

	// Read file content
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	// Parse protobuf
	atlas::protobuf::appearances::Appearances proto;
	if (!proto.ParseFromString(content)) {
		std::cout << "[Error - Appearances::loadFromFile] Failed to parse protobuf from: " << filename << std::endl;
		return false;
	}

	// Clear existing data
	objects.clear();
	outfits.clear();
	effects.clear();
	missiles.clear();

	// Load objects (items)
	for (const auto& appearance : proto.object()) {
		if (appearance.has_id()) {
			AppearanceInfo info;
			parseAppearance(appearance, info);
			objects[info.id] = std::move(info);
		}
	}

	// Load outfits
	for (const auto& appearance : proto.outfit()) {
		if (appearance.has_id()) {
			AppearanceInfo info;
			parseAppearance(appearance, info);
			outfits[info.id] = std::move(info);
		}
	}

	// Load effects
	for (const auto& appearance : proto.effect()) {
		if (appearance.has_id()) {
			AppearanceInfo info;
			parseAppearance(appearance, info);
			effects[info.id] = std::move(info);
		}
	}

	// Load missiles
	for (const auto& appearance : proto.missile()) {
		if (appearance.has_id()) {
			AppearanceInfo info;
			parseAppearance(appearance, info);
			missiles[info.id] = std::move(info);
		}
	}

	// Load special meaning IDs
	if (proto.has_special_meaning_appearance_ids()) {
		const auto& special = proto.special_meaning_appearance_ids();
		if (special.has_gold_coin_id()) {
			goldCoinId = static_cast<uint16_t>(special.gold_coin_id());
		}
		if (special.has_platinum_coin_id()) {
			platinumCoinId = static_cast<uint16_t>(special.platinum_coin_id());
		}
		if (special.has_crystal_coin_id()) {
			crystalCoinId = static_cast<uint16_t>(special.crystal_coin_id());
		}
		if (special.has_tibia_coin_id()) {
			tibiaCoinId = static_cast<uint16_t>(special.tibia_coin_id());
		}
		if (special.has_stamped_letter_id()) {
			stampedLetterId = static_cast<uint16_t>(special.stamped_letter_id());
		}
		if (special.has_supply_stash_id()) {
			supplyStashId = static_cast<uint16_t>(special.supply_stash_id());
		}
		if (special.has_reward_chest_id()) {
			rewardChestId = static_cast<uint16_t>(special.reward_chest_id());
		}
	}

	std::cout << ">> Loaded " << objects.size() << " object appearances" << std::endl;
	std::cout << ">> Loaded " << outfits.size() << " outfit appearances" << std::endl;
	std::cout << ">> Loaded " << effects.size() << " effect appearances" << std::endl;
	std::cout << ">> Loaded " << missiles.size() << " missile appearances" << std::endl;

	return true;
}

void Appearances::parseAppearance(const atlas::protobuf::appearances::Appearance& proto, AppearanceInfo& info)
{
	info.id = static_cast<uint16_t>(proto.id());

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
			info.marketTradeAs = static_cast<uint16_t>(market.trade_as_object_id());
		}
		if (market.has_show_as_object_id()) {
			info.marketShowAs = static_cast<uint16_t>(market.show_as_object_id());
		}
		if (market.has_minimum_level()) {
			info.marketMinLevel = static_cast<uint16_t>(market.minimum_level());
		}
		// Profession mask
		for (int i = 0; i < market.restrict_to_profession_size(); ++i) {
			int prof = market.restrict_to_profession(i);
			if (prof > 0) {
				info.marketProfessionMask |= (1 << prof);
			}
		}
	}

	// Upgrade classification
	if (flags.has_upgradeclassification()) {
		if (flags.upgradeclassification().has_upgrade_classification()) {
			info.classification = static_cast<uint8_t>(flags.upgradeclassification().upgrade_classification());
		}
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
