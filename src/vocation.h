// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_VOCATION_H
#define FS_VOCATION_H

#include "configmanager.h"
#include "enums.h"

static constexpr uint32_t vocSkillBase[SKILL_LAST + 1] = {50, 50, 50, 50, 30, 100, 20};
static constexpr int16_t VOCATION_MINIMUM_SKILL_LEVEL = 10;

class Vocation
{
public:
	explicit Vocation(uint16_t id) : id(id) {}

	uint64_t getReqSkillTries(uint8_t skill, uint16_t level) const
	{
		if (skill > SKILL_LAST) {
			return 0;
		}
		return vocSkillBase[skill] *
		       std::pow(skillMultipliers[skill], static_cast<int32_t>(level - (VOCATION_MINIMUM_SKILL_LEVEL + 1)));
	}

	uint64_t getReqMana(uint32_t magLevel) const
	{
		if (magLevel == 0) {
			return 0;
		}
		return 1600 * std::pow(manaMultiplier, static_cast<int32_t>(magLevel - 1));
	}

	bool getMagicShield() const
	{
		if (!getBoolean(ConfigManager::MANASHIELD_BREAKABLE)) {
			return false;
		}
		return magicShield;
	}

	std::string name = "none";
	std::string description;

	double skillMultipliers[SKILL_LAST + 1] = {1.5, 2.0, 2.0, 2.0, 2.0, 1.5, 1.1};
	float manaMultiplier = 4.0f;
	float meleeDamageMultiplier = 1.0f;
	float distDamageMultiplier = 1.0f;
	float defenseMultiplier = 1.0f;
	float armorMultiplier = 1.0f;

	uint32_t gainHealthTicks = 6;
	uint32_t gainHealthAmount = 1;
	uint32_t gainManaTicks = 6;
	uint32_t gainManaAmount = 1;
	uint32_t gainCap = 500;
	uint32_t gainMana = 5;
	uint32_t gainHP = 5;
	uint32_t fromVocation = VOCATION_NONE;
	uint32_t attackSpeed = 1500;
	uint32_t baseSpeed = 220;
	uint32_t noPongKickTime = 60000;

	uint16_t id;
	uint16_t gainSoulTicks = 120;

	uint8_t soulMax = 100;
	uint8_t clientId = 0;

	bool allowPvp = true;
	bool magicShield = false;
};

using VocationMap = std::map<uint16_t, Vocation>;

class Vocations
{
public:
	Vocation& registerVocation(uint16_t id)
	{
		auto [it, inserted] = vocationsMap.emplace(std::piecewise_construct,
		                                           std::forward_as_tuple(id),
		                                           std::forward_as_tuple(id));
		return it->second;
	}

	Vocation* getVocation(uint16_t id)
	{
		auto it = vocationsMap.find(id);
		if (it == vocationsMap.end()) {
			std::cout << "[Warning - Vocations::getVocation] Vocation " << id << " not found." << std::endl;
			return nullptr;
		}
		return &it->second;
	}

	int32_t getVocationId(std::string_view name) const
	{
		auto it = std::find_if(vocationsMap.begin(), vocationsMap.end(),
		                       [=](const auto& entry) { return boost::iequals(name, entry.second.name); });
		return it != vocationsMap.end() ? it->first : -1;
	}

	uint16_t getPromotedVocation(uint16_t id) const
	{
		auto it = std::find_if(vocationsMap.begin(), vocationsMap.end(),
		                       [id](const auto& entry) { return entry.second.fromVocation == id && entry.first != id; });
		return it != vocationsMap.end() ? it->first : VOCATION_NONE;
	}

	const VocationMap& getVocations() const { return vocationsMap; }

private:
	VocationMap vocationsMap;
};

extern Vocations g_vocations;

#endif // FS_VOCATION_H
