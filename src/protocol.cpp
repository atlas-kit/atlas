// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "protocol.h"

#include "outputmessage.h"
#include "rsa.h"
#include "xtea.h"

Protocol::~Protocol()
{
	const auto zlibEndResult = deflateEnd(&zlibStream);
	if (zlibEndResult == Z_DATA_ERROR) {
		std::cout << "ZLIB discarded pending output or unprocessed input while cleaning up stream state" << std::endl;
	} else if (zlibEndResult == Z_STREAM_ERROR) {
		std::cout << "ZLIB encountered an error while cleaning up stream state" << std::endl;
	}
}

void Protocol::onSendMessage(const std::shared_ptr<OutputMessage>& msg)
{
	// Raw messages: no framing at all.
	if (rawMode) {
		return;
	}

	// Unencrypted: only need the block count header.
	if (!encrypted) {
		const auto blockCount =
		    static_cast<uint16_t>((msg->getLength() - NetworkMessage::CHECKSUM_LENGTH) / NetworkMessage::XTEA_MULTIPLE);
		msg->addHeader(blockCount);
		return;
	}

	// 1. Optional zlib compression (MSB of sequenceId = 1 when compressed).
	auto compressionFlag = 0u;
	if (msg->getLength() >= 128 && deflateMessage(*msg)) {
		compressionFlag = 0x80000000u;
	}

	// 2. Pad to XTEA block boundary, stash padding count as first encrypted byte.
	const auto paddingCount =
	    static_cast<uint8_t>(NetworkMessage::XTEA_MULTIPLE - (msg->getLength() % NetworkMessage::XTEA_MULTIPLE) - 1);
	msg->addPaddingBytes(paddingCount);
	msg->addHeader(paddingCount);

	// 3. XTEA-encrypt in-place.
	xtea::encrypt(msg->getOutputBuffer(), msg->getLength(), xteaKey);

	// 4. Prepend sequence/checksum (u32, plaintext).
	const auto sequenceId = compressionFlag | nextSequenceId();
	msg->addHeader(sequenceId);

	// 5. Prepend block count (u16, plaintext).
	const auto blockCount =
	    static_cast<uint16_t>((msg->getLength() - NetworkMessage::CHECKSUM_LENGTH) / NetworkMessage::XTEA_MULTIPLE);
	msg->addHeader(blockCount);
}

void Protocol::onRecvMessage(NetworkMessage& msg)
{
	// Unencrypted: no framing to strip.
	if (!encrypted) {
		parsePacket(msg);
		return;
	}

	// 1. Verify remainder after 6-byte header is whole XTEA blocks.
	const auto encryptedLen = msg.getLength() - NetworkMessage::CRYPTO_HEADER_LENGTH;
	if ((encryptedLen & (NetworkMessage::XTEA_MULTIPLE - 1)) != 0) {
		return;
	}

	// 2. Decrypt in-place.
	xtea::decrypt(msg.getRemainingBuffer(), encryptedLen, xteaKey);

	// 3. Strip padding (first decrypted byte = padding count).
	const auto paddingLength = msg.getByte();
	msg.setLength(msg.getLength() - paddingLength);

	parsePacket(msg);
}

std::shared_ptr<OutputMessage> Protocol::getOutputBuffer(int32_t size)
{
	// dispatcher thread
	if (!sendBuffer) {
		sendBuffer = tfs::net::make_output_message();
	} else if ((sendBuffer->getLength() + size) > NetworkMessage::MAX_PROTOCOL_BODY_LENGTH) {
		send(sendBuffer);
		sendBuffer = tfs::net::make_output_message();
	}
	return sendBuffer;
}

bool Protocol::deflateMessage(OutputMessage& msg)
{
	static thread_local std::vector<uint8_t> buffer(NETWORKMESSAGE_MAXSIZE);

	zlibStream.next_in = msg.getOutputBuffer();
	zlibStream.avail_in = msg.getLength();
	zlibStream.next_out = buffer.data();
	zlibStream.avail_out = buffer.size();

	const auto result = deflate(&zlibStream, Z_FINISH);
	if (result != Z_OK && result != Z_STREAM_END) {
		std::cout << "Error while deflating packet data error: " << (zlibStream.msg ? zlibStream.msg : "unknown")
		          << std::endl;
		return false;
	}

	const auto size = zlibStream.total_out;
	deflateReset(&zlibStream);

	if (size <= 0) {
		std::cout << "Deflated packet data had invalid size: " << size
		          << " error: " << (zlibStream.msg ? zlibStream.msg : "unknown") << std::endl;
		return false;
	}

	msg.reset();
	msg.addBytes(reinterpret_cast<const char*>(buffer.data()), size);

	return true;
}

boost::asio::ip::address Protocol::getRemoteAddress() const
{
	if (auto connection = getConnection()) {
		return connection->getRemoteAddress();
	}

	return {};
}
