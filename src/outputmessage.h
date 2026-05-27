// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_OUTPUTMESSAGE_H
#define FS_OUTPUTMESSAGE_H

#include "connection.h"
#include "networkmessage.h"
#include "tools.h"

class OutputMessage : public NetworkMessage
{
public:
	// User-provided (non-defaulted) so that std::allocate_shared<OutputMessage> performs
	// default-initialization instead of value-initialization. Value-initialization would
	// zero the entire NetworkMessage::buffer (~64 KB) on every construction even though
	// only buffer[0..length) is ever written/sent. All transmitted bytes are explicitly
	// written by add*/add_header/addPaddingBytes before send, so leaving the buffer
	// uninitialized is safe (this already matches plain `NetworkMessage` usage).
	OutputMessage() noexcept {}

	// non-copyable
	OutputMessage(const OutputMessage&) = delete;
	OutputMessage& operator=(const OutputMessage&) = delete;

	uint8_t* getOutputBuffer() { return &buffer[outputBufferStart]; }

	void append(const NetworkMessage& msg)
	{
		auto msgLen = msg.getLength();
		if (msgLen == 0 || info.position + msgLen > buffer.size()) {
			return;
		}
		std::memcpy(buffer.data() + info.position, msg.getBuffer() + INITIAL_BUFFER_POSITION, msgLen);
		info.length += msgLen;
		info.position += msgLen;
	}

	void append(const std::shared_ptr<OutputMessage>& msg)
	{
		auto msgLen = msg->getLength();
		if (msgLen == 0 || info.position + msgLen > buffer.size()) {
			return;
		}
		std::memcpy(buffer.data() + info.position, msg->getBuffer() + INITIAL_BUFFER_POSITION, msgLen);
		info.length += msgLen;
		info.position += msgLen;
	}

	template <typename T>
	void addHeader(T value)
	{
		assert(outputBufferStart >= sizeof(T));
		outputBufferStart -= sizeof(T);
		std::memcpy(buffer.data() + outputBufferStart, &value, sizeof(T));
		// added header size to the message size
		info.length += sizeof(T);
	}

private:
	MsgSize_t outputBufferStart = INITIAL_BUFFER_POSITION;
	// Was incidentally zeroed by value-initialization; keep an explicit default now that
	// the object is no longer zero-initialized on construction.
	uint32_t sequenceId = 0;
};

namespace tfs::net {

std::shared_ptr<OutputMessage> make_output_message();
void insert_protocol_to_autosend(const std::shared_ptr<Protocol>& protocol);
void remove_protocol_from_autosend(const std::shared_ptr<Protocol>& protocol);

} // namespace tfs::net

#endif // FS_OUTPUTMESSAGE_H
