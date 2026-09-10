// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "networkmessage.h"

#include "container.h"
#include "podium.h"

#include <simdutf.h>

std::string NetworkMessage::getString(uint16_t stringLen /* = 0*/)
{
	if (stringLen == 0) {
		stringLen = get<uint16_t>();
	}

	if (!canRead(stringLen)) {
		return {};
	}

	auto it = reinterpret_cast<char*>(buffer.data() + info.position);
	info.position += stringLen;
	const auto outLen = simdutf::utf8_length_from_latin1(it, stringLen);
	auto out = std::string();
	out.resize_and_overwrite(outLen, [&](char* data, size_t) {
		std::ignore = simdutf::convert_latin1_to_utf8(it, stringLen, data);
		return outLen;
	});
	return out;
}

Position NetworkMessage::getPosition()
{
	Position pos;
	pos.x = get<uint16_t>();
	pos.y = get<uint16_t>();
	pos.z = getByte();
	return pos;
}

bool NetworkMessage::getBool()
{
	const uint8_t value = getByte();
	if (value > 1) {
		std::cout << "[Warning - NetworkMessage::getBool] Invalid boolean value received: " << static_cast<int>(value)
		          << std::endl;
	}
	return value != 0;
}

void NetworkMessage::addString(std::string_view value)
{
	auto stringLen = simdutf::latin1_length_from_utf8(value.data(), value.size());
	if (!canAdd(stringLen + 2) || stringLen > 8192) {
		return;
	}

	add<uint16_t>(stringLen);
	auto it = reinterpret_cast<char*>(buffer.data() + info.position);
	std::ignore = simdutf::convert_utf8_to_latin1(value.data(), value.size(), it);
	info.position += stringLen;
	info.length += stringLen;
}

void NetworkMessage::addDouble(double value, uint8_t precision /* = 2*/)
{
	addByte(precision);
	add<uint32_t>(static_cast<uint32_t>((value * std::pow(static_cast<float>(10), precision)) +
	                                    std::numeric_limits<int32_t>::max()));
}

void NetworkMessage::addBytes(const char* bytes, size_t size)
{
	if (!canAdd(size) || size > 8192) {
		return;
	}

	std::memcpy(buffer.data() + info.position, bytes, size);
	info.position += size;
	info.length += size;
}

void NetworkMessage::addPaddingBytes(size_t n)
{
	if (!canAdd(n)) {
		return;
	}

	std::fill_n(buffer.data() + info.position, n, 0x33);
	info.length += n;
}

void NetworkMessage::addPosition(const Position& pos)
{
	add<uint16_t>(pos.x);
	add<uint16_t>(pos.y);
	addByte(pos.z);
}

// Wire layout for items in protocol 15.x. Every check below is independent —
// the client reads each optional section in order based on the flags in its
// own appearances.dat, so a single item can carry several sections.
//
// Order must match the client's read sequence:
//   1. clientId               u16
//   2. stackable              u8 count
//   3. fluid/splash           u8 fluidType
//   4. container              u8 containerType + (variant payload)
//   5. podium                 outfit + mount + u8 direction + u8 visible
//   6. upgradeClassification  u8 tier
//   7. clock/expire/expireStop u32 decay + u8 brandNew
//   8. wearOut                u32 charges + u8 brandNew
//   9. isWrapKit              u16 unWrapId

void NetworkMessage::addItem(uint16_t id, uint8_t count)
{
	const ItemType& it = Item::items[id];

	add<uint16_t>(it.clientId);

	if (it.stackable) {
		addByte(count);
	}

	if (it.isSplash() || it.isFluidContainer()) {
		addByte(count);
	}

	if (it.isContainer()) {
		addByte(std::to_underlying(ContainerSpecial_t::None));
	}

	if (it.isPodium()) {
		add<uint16_t>(0); // looktype
		add<uint16_t>(0); // lookTypeEx
		add<uint16_t>(0); // lookmount
		addByte(2);       // direction
		addByte(0x01);    // is visible (bool)
	}

	if (it.classification > 0) {
		addByte(0x00); // item tier (0-10)
	}

	if (it.clockExpire || it.expire || it.expireStop) {
		add<uint32_t>(floor<std::chrono::seconds>(it.decayTimeMin).count());
		addByte(0x01); // brand-new
	}

	if (it.wearOut) {
		add<uint32_t>(it.charges);
		addByte(0x01); // brand-new
	}

	if (it.isWrapKit) {
		add<uint16_t>(0x00); // unWrapId
	}
}

void NetworkMessage::addItem(const std::shared_ptr<const Item>& item)
{
	const ItemType& it = Item::items[item->getID()];

	add<uint16_t>(it.clientId);

	if (it.stackable) {
		addByte(std::min<uint16_t>(0xFF, item->getItemCount()));
	}

	if (it.isSplash() || it.isFluidContainer()) {
		addByte(static_cast<uint8_t>(item->getFluidType()));
	}

	if (it.isContainer()) {
		const auto& container = item->asContainer();
		if (container && it.weaponType == WEAPON_QUIVER) {
			addByte(std::to_underlying(ContainerSpecial_t::ContentCounter));
			add<uint32_t>(container->getAmmoCount());
		} else {
			addByte(std::to_underlying(ContainerSpecial_t::None));
		}
	}

	if (it.isPodium()) {
		const auto& podium = item->asPodium();
		const Outfit_t& outfit = podium->getOutfit();

		// add outfit
		if (podium->hasFlag(PODIUM_SHOW_OUTFIT)) {
			add<uint16_t>(outfit.lookType);
			if (outfit.lookType != 0) {
				addByte(outfit.lookHead);
				addByte(outfit.lookBody);
				addByte(outfit.lookLegs);
				addByte(outfit.lookFeet);
				addByte(outfit.lookAddons);
			} else {
				add<uint16_t>(0);
			}
		} else {
			add<uint16_t>(0);
			add<uint16_t>(0);
		}

		// add mount
		if (podium->hasFlag(PODIUM_SHOW_MOUNT)) {
			add<uint16_t>(outfit.lookMount);
			if (outfit.lookMount != 0) {
				addByte(outfit.lookMountHead);
				addByte(outfit.lookMountBody);
				addByte(outfit.lookMountLegs);
				addByte(outfit.lookMountFeet);
			}
		} else {
			add<uint16_t>(0);
		}

		addByte(podium->getDirection());
		addByte(podium->hasFlag(PODIUM_SHOW_PLATFORM) ? 0x01 : 0x00);
	}

	if (it.classification > 0) {
		addByte(0x00); // item tier (0-10) — not persisted yet
	}

	if (it.clockExpire || it.expire || it.expireStop) {
		add<uint32_t>(floor<std::chrono::seconds>(item->getDuration()).count());
		addByte(0x01); // brand-new
	}

	if (it.wearOut) {
		add<uint32_t>(item->getCharges());
		addByte(0x01); // brand-new
	}

	if (it.isWrapKit) {
		add<uint16_t>(0x00); // unWrapId — not persisted yet
	}
}

void NetworkMessage::addItemId(uint16_t itemId) { add<uint16_t>(Item::items[itemId].clientId); }

void NetworkMessage::addBool(bool value) { addByte(value ? 1 : 0); }
