// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "protocol.h"

#include "outputmessage.h"
#include "rsa.h"
#include "xtea.h"

Protocol::~Protocol()
{
	const auto zlibEndResult = deflateEnd(&zstream);
	if (zlibEndResult == Z_DATA_ERROR) {
		std::cout << "ZLIB discarded pending output or unprocessed input while cleaning up stream state" << std::endl;
	} else if (zlibEndResult == Z_STREAM_ERROR) {
		std::cout << "ZLIB encountered an error while cleaning up stream state" << std::endl;
	}
}

void Protocol::onSendMessage(const std::shared_ptr<OutputMessage>& msg)
{
	if (rawMessages) {
		return;
	}

	if (!encryptionEnabled) {
		const auto blockCount =
		    static_cast<uint16_t>((msg->getLength() - NetworkMessage::CHECKSUM_LENGTH) / NetworkMessage::XTEA_MULTIPLE);
		msg->addHeader(blockCount);
		return;
	}

	// The encrypted wire format (built LIFO via addHeader):
	//
	// ┌───────┬────────┬────────────────┬───────────────────────────────────┐
	// │  pos  │ bytes  │     field      │ built by                          │
	// ├───────┼────────┼────────────────┼───────────────────────────────────┤
	// │  0-1  │ u16    │ blockCount     │ addHeader                         │
	// │  2-5  │ u32    │ sequenceId     │ addHeader                         │
	// │   6   │ u8     │ paddingCount   │ addHeader (after addPaddingBytes) │
	// │  7+   │ N      │ data + 0x33    │ append + XTEA encrypt in-place   │
	// └───────┴────────┴────────────────┴───────────────────────────────────┘
	//
	// Fields 0–6 are plaintext on the wire. PaddingCount becomes the first
	// encrypted byte — the receiver reads it to discard trailing 0x33.

	// 1. Optional zlib compression (MSB of sequenceId = 1 when compressed).
	auto compressionFlag = 0u;
	if (msg->getLength() >= 128 && deflateMessage(*msg)) {
		compressionFlag = 0x80000000u;
	}

	// 2. Append 0x33 padding bytes so that (plaintext + 1) is a multiple of 8.
	//    Then prepend the padding count as a 1-byte header — this becomes
	//    the first encrypted byte.
	const auto paddingCount =
	    static_cast<uint8_t>(NetworkMessage::XTEA_MULTIPLE - (msg->getLength() % NetworkMessage::XTEA_MULTIPLE) - 1);
	msg->addPaddingBytes(paddingCount);
	msg->addHeader(paddingCount);

	// 3. XTEA-encrypt the padded payload in-place.
	xtea::encrypt(msg->getOutputBuffer(), msg->getLength(), key);

	// 4. Prepend the sequence/checksum (u32, plaintext).
	const auto sequenceId = compressionFlag | getNextSequenceId();
	msg->addHeader(sequenceId);

	// 5. Prepend the block count (u16, plaintext) = total encrypted bytes ÷ 8.
	const auto blockCount =
	    static_cast<uint16_t>((msg->getLength() - NetworkMessage::CHECKSUM_LENGTH) / NetworkMessage::XTEA_MULTIPLE);
	msg->addHeader(blockCount);
}

void Protocol::onRecvMessage(NetworkMessage& msg)
{
	if (!encryptionEnabled) {
		parsePacket(msg);
		return;
	}

	// The connection layer already consumed the 6-byte plaintext header
	// (blockCount + checksum) before calling us. The buffer looks like:
	//
	// ┌───────┬────────┬──────────────────┬─────────────────────────┐
	// │  pos  │ bytes  │     field        │       status            │
	// ├───────┼────────┼──────────────────┼─────────────────────────┤
	// │  0-1  │ u16    │ blockCount       │ already read by parse   │
	// │  2-5  │ u32    │ checksum         │ already read by parse   │
	// │   6   │ u8     │ paddingAmount    │ to be decrypted         │
	// │  7+   │ N      │ data + 0x33      │ to be decrypted         │
	// └───────┴────────┴──────────────────┴─────────────────────────┘
	//
	// getRemainingBuffer() points at position 6 (start of encrypted data).
	// After decryption, the first byte is the padding amount.

	// 1. Verify the encrypted payload is a whole number of XTEA blocks.
	const auto encryptedLen = msg.getLength() - NetworkMessage::CRYPTO_HEADER_LENGTH;
	if ((encryptedLen & (NetworkMessage::XTEA_MULTIPLE - 1)) != 0) {
		return;
	}

	// 2. Decrypt in-place. After this, buffer[6] = paddingAmount,
	//    buffer[7+] = real data followed by paddingLength bytes of 0x33.
	xtea::decrypt(msg.getRemainingBuffer(), encryptedLen, key);

	// 3. Read the padding amount (first decrypted byte) and discard the trailing 0x33.
	const auto paddingLength = msg.getByte();
	msg.setLength(msg.getLength() - paddingLength);

	parsePacket(msg);
}

std::shared_ptr<OutputMessage> Protocol::getOutputBuffer(int32_t size)
{
	// dispatcher thread
	/*if (!outputBuffer) {
	    outputBuffer = tfs::net::make_output_message();
	} else if ((outputBuffer->getLength() + size) > NetworkMessage::MAX_PROTOCOL_BODY_LENGTH) {
	    send(outputBuffer);
	    outputBuffer = tfs::net::make_output_message();
	}*/

	if (outputBuffer) {
		send(outputBuffer);
	}
	outputBuffer = tfs::net::make_output_message();
	return outputBuffer;
}

bool Protocol::RSA_decrypt(NetworkMessage& msg)
{
	if (msg.getRemainingBufferLength() < RSA_BUFFER_LENGTH) {
		return false;
	}

	tfs::rsa::decrypt(msg.getRemainingBuffer(), RSA_BUFFER_LENGTH);
	return msg.getByte() == 0;
}

bool Protocol::deflateMessage(OutputMessage& msg)
{
	static thread_local std::vector<uint8_t> buffer(NETWORKMESSAGE_MAXSIZE);

	zstream.next_in = msg.getOutputBuffer();
	zstream.avail_in = msg.getLength();
	zstream.next_out = buffer.data();
	zstream.avail_out = buffer.size();

	const auto result = deflate(&zstream, Z_FINISH);
	if (result != Z_OK && result != Z_STREAM_END) {
		std::cout << "Error while deflating packet data error: " << (zstream.msg ? zstream.msg : "unknown")
		          << std::endl;
		return false;
	}

	const auto size = zstream.total_out;
	deflateReset(&zstream);

	if (size <= 0) {
		std::cout << "Deflated packet data had invalid size: " << size
		          << " error: " << (zstream.msg ? zstream.msg : "unknown") << std::endl;
		return false;
	}

	msg.reset();
	msg.addBytes(reinterpret_cast<const char*>(buffer.data()), size);

	return true;
}

Connection::Address Protocol::getIP() const
{
	if (auto connection = getConnection()) {
		return connection->getIP();
	}

	return {};
}
