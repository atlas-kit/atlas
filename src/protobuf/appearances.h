// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPEARANCES_H
#define FS_APPEARANCES_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct NpcSaleInfo
{
	std::string name;
	std::string location;
	uint32_t salePrice = 0;
	uint32_t buyPrice = 0;
	uint32_t currencyObjectTypeId = 0;
	std::string currencyQuestFlagDisplayName;
};

// Forward declaration of protobuf types
namespace atlas::protobuf::appearances {
class Appearances;
class Appearance;
class AppearanceFlags;
} // namespace atlas::protobuf::appearances

struct AppearanceInfo
{
	uint16_t id = 0;

	// Basic flags from protobuf
	bool isGround = false;
	bool isGroundBorder = false;
	bool isOnBottom = false;
	bool isOnTop = false;
	bool isContainer = false;
	bool isStackable = false;
	bool isUsable = false;
	bool isForceUse = false;
	bool isMultiUse = false;
	bool isWritable = false;
	bool isWritableOnce = false;
	bool isFluidContainer = false;
	bool isFluidPool = false; // liquidpool
	bool isUnpassable = false;
	bool isUnmovable = false;
	bool isBlockMissile = false; // unsight
	bool isBlockPath = false;    // avoid
	bool isPickupable = false;   // take
	bool isHangable = false;
	bool isRotatable = false;
	bool hasLight = false;
	bool isDontHide = false;
	bool isTranslucent = false;
	bool hasOffset = false;
	bool hasElevation = false;
	bool isLyingObject = false;
	bool isAnimateAlways = false;
	bool hasAutomapColor = false;
	bool isLensHelp = false;
	bool isFullGround = false;
	bool isIgnoreLook = false;
	bool isCloth = false;
	bool hasDefaultAction = false;
	bool isWrap = false;
	bool isUnwrap = false;
	bool isTopEffect = false;
	bool isCorpse = false;
	bool isPlayerCorpse = false;
	bool isAmmo = false;
	bool isShowOffSocket = false;
	bool isReportable = false;
	bool noMovementAnimation = false;
	bool reverseAddonsEast = false;
	bool reverseAddonsWest = false;
	bool reverseAddonsSouth = false;
	bool reverseAddonsNorth = false;
	bool wearout = false;
	bool clockExpire = false;
	bool expire = false;
	bool expireStop = false;
	bool decoItemKit = false;
	bool dualWielding = false;

	// Extended attributes
	uint32_t groundSpeed = 0;
	uint16_t maxTextLength = 0;
	uint8_t lightLevel = 0;
	uint8_t lightColor = 0;
	uint16_t elevation = 0;
	int16_t shiftX = 0;
	int16_t shiftY = 0;
	uint16_t automapColor = 0;
	uint8_t clothSlot = 0;
	uint8_t defaultAction = 0;
	uint8_t lensHelp = 0;
	uint8_t classification = 0;
	uint8_t hookDirection = 0;
	uint8_t weaponType = 0;
	uint32_t minimumLevel = 0;
	uint32_t imbueableSlotCount = 0;
	uint32_t gemQualityId = 0;
	uint32_t gemVocationId = 0;
	uint32_t proficiencyId = 0;
	uint32_t cyclopediaType = 0;
	uint32_t formerObjectTypeId = 0;
	std::vector<int32_t> restrictedVocations;
	std::vector<NpcSaleInfo> npcSaleData;

	// Market info
	uint16_t marketCategory = 0;
	uint16_t marketTradeAs = 0;
	uint16_t marketShowAs = 0;

	std::string name;
	std::string description;
};

class Appearances
{
public:
	Appearances() = default;

	// non-copyable
	Appearances(const Appearances&) = delete;
	Appearances& operator=(const Appearances&) = delete;

	bool loadFromFile(const std::string& filename);

	const AppearanceInfo* getObjectAppearance(uint16_t id) const;
	const AppearanceInfo* getOutfitAppearance(uint16_t id) const;
	const AppearanceInfo* getEffectAppearance(uint16_t id) const;
	const AppearanceInfo* getMissileAppearance(uint16_t id) const;

	size_t getObjectCount() const { return objects.size(); }
	size_t getOutfitCount() const { return outfits.size(); }
	size_t getEffectCount() const { return effects.size(); }
	size_t getMissileCount() const { return missiles.size(); }

	// Get all objects for iteration
	const std::unordered_map<uint16_t, AppearanceInfo>& getObjects() const { return objects; }

	// Special meaning IDs (from protobuf)
	uint16_t goldCoinId = 0;
	uint16_t platinumCoinId = 0;
	uint16_t crystalCoinId = 0;
	uint16_t tibiaCoinId = 0;
	uint16_t stampedLetterId = 0;
	uint16_t supplyStashId = 0;
	uint16_t standardRewardChestId = 0;
	uint16_t blankImbuementScrollId = 0;

private:
	void parseAppearance(const atlas::protobuf::appearances::Appearance& proto, AppearanceInfo& info);
	void parseFlags(const atlas::protobuf::appearances::AppearanceFlags& flags, AppearanceInfo& info);

	std::unordered_map<uint16_t, AppearanceInfo> objects;
	std::unordered_map<uint16_t, AppearanceInfo> outfits;
	std::unordered_map<uint16_t, AppearanceInfo> effects;
	std::unordered_map<uint16_t, AppearanceInfo> missiles;
};

extern Appearances g_appearances;

#endif // FS_APPEARANCES_H
