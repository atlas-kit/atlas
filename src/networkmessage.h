// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_NETWORKMESSAGE_H
#define FS_NETWORKMESSAGE_H

#include "const.h"

class Item;
struct Position;

class NetworkMessage
{
public:
	using MsgSize_t = uint16_t;

	// Wire format: [blockCount:u16] [checksum:u32] [padding:u8] [encrypted data + 0x33 padding]
	static constexpr MsgSize_t HEADER_LENGTH = 2;   // u16 XTEA block count
	static constexpr MsgSize_t CHECKSUM_LENGTH = 4; // u32 sequence number
	static constexpr MsgSize_t PADDING_LENGTH = 1;  // u8 padding count (0x00–0x07)
	static constexpr MsgSize_t CRYPTO_HEADER_LENGTH =
	    HEADER_LENGTH + CHECKSUM_LENGTH; // plaintext before encrypted payload
	static constexpr MsgSize_t INITIAL_BUFFER_POSITION =
	    HEADER_LENGTH + CHECKSUM_LENGTH + PADDING_LENGTH; // header area reserved in OutputMessage
	static constexpr MsgSize_t XTEA_MULTIPLE = 8;         // XTEA block size
	static constexpr MsgSize_t MAX_BODY_LENGTH =
	    NETWORKMESSAGE_MAXSIZE - HEADER_LENGTH - CHECKSUM_LENGTH - XTEA_MULTIPLE;
	static constexpr MsgSize_t MAX_PROTOCOL_BODY_LENGTH = MAX_BODY_LENGTH - 10;

	NetworkMessage() = default;

	void reset()
	{
		info.length = 0;
		info.position = INITIAL_BUFFER_POSITION;
		info.overrun = false;
	}

	// simply read functions for incoming message
	uint8_t getByte()
	{
		if (!canRead(1)) {
			return 0;
		}

		return buffer[info.position++];
	}

	uint8_t getPreviousByte() { return buffer[--info.position]; }

	template <typename T>
	std::enable_if_t<std::is_trivially_copyable_v<T>, T> get() noexcept
	{
		static_assert(std::is_trivially_constructible_v<T>, "Destination type must be trivially constructible");

		if (!canRead(sizeof(T))) {
			return 0;
		}

		T value;
		std::memcpy(&value, buffer.data() + info.position, sizeof(T));
		info.position += sizeof(T);
		return value;
	}

	std::string getString(uint16_t stringLen = 0);
	Position getPosition();
	bool getBool();

	// skips count unknown/unused bytes in an incoming message
	void skipBytes(int16_t count) { info.position += count; }

	// simply write functions for outgoing message
	void addByte(uint8_t value)
	{
		if (!canAdd(1)) {
			return;
		}

		buffer[info.position++] = value;
		info.length++;
	}

	template <typename T>
	void add(T value)
	{
		if (!canAdd(sizeof(T))) {
			return;
		}

		std::memcpy(buffer.data() + info.position, &value, sizeof(T));
		info.position += sizeof(T);
		info.length += sizeof(T);
	}

	void addBytes(const char* bytes, size_t size);
	void addPaddingBytes(size_t n);

	void addString(std::string_view value);

	void addDouble(double value, uint8_t precision = 2);

	// write functions for complex types
	void addPosition(const Position& pos);
	void addItem(uint16_t id, uint8_t count);
	void addItem(const std::shared_ptr<const Item>& item);
	void addItemId(uint16_t itemId);
	void addBool(bool value);

	MsgSize_t getLength() const { return info.length; }

	bool isEmpty() const { return info.length == 0; }

	void setLength(MsgSize_t newLength) { info.length = newLength; }

	MsgSize_t getBufferPosition() const { return info.position; }

	MsgSize_t getRemainingBufferLength() const { return info.length - info.position; }

	uint16_t getLengthHeader() const { return static_cast<uint16_t>(buffer[0] | buffer[1] << 8); }

	bool isOverrun() const { return info.overrun; }

	uint8_t* getBuffer() { return &buffer[0]; }

	const uint8_t* getBuffer() const { return &buffer[0]; }

	uint8_t* getRemainingBuffer() { return &buffer[0] + info.position; }

	uint8_t* getBodyBuffer()
	{
		info.position = 2;
		return &buffer[HEADER_LENGTH];
	}

protected:
	struct NetworkMessageInfo
	{
		MsgSize_t length = 0;
		MsgSize_t position = INITIAL_BUFFER_POSITION;
		bool overrun = false;
	};

	NetworkMessageInfo info;
	std::array<uint8_t, NETWORKMESSAGE_MAXSIZE> buffer;

private:
	bool canAdd(size_t size) const { return (size + info.position) < MAX_BODY_LENGTH; }

	bool canRead(int32_t size)
	{
		if ((info.position + size) > (info.length + 8) || size >= (NETWORKMESSAGE_MAXSIZE - info.position)) {
			info.overrun = true;
			return false;
		}
		return true;
	}
};

#endif // FS_NETWORKMESSAGE_H
