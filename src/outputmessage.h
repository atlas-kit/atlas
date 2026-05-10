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
	OutputMessage() = default;

	// non-copyable
	OutputMessage(const OutputMessage&) = delete;
	OutputMessage& operator=(const OutputMessage&) = delete;

	uint8_t* getOutputBuffer() { return &buffer[outputBufferStart]; }

	void writeMessageLength() { add_header(static_cast<uint16_t>((info.length - 4) / 8)); }

	void writePaddingLength()
	{
		uint8_t paddingAmount = static_cast<uint8_t>(8 - (info.length % 8) - 1);
		add_header(paddingAmount);
	}

	void addCryptoHeader() { add_header(getSequenceId()); }

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

	void setSequenceId(uint32_t sequence) { sequenceId = sequence; }
	uint32_t getSequenceId() const { return sequenceId; }

private:
	template <typename T>
	void add_header(T add)
	{
		assert(outputBufferStart >= sizeof(T));
		outputBufferStart -= sizeof(T);
		std::memcpy(buffer.data() + outputBufferStart, &add, sizeof(T));
		// added header size to the message size
		info.length += sizeof(T);
	}

	MsgSize_t outputBufferStart = INITIAL_BUFFER_POSITION;
	uint32_t sequenceId;
};

namespace tfs::net {

std::shared_ptr<OutputMessage> make_output_message();
void insert_protocol_to_autosend(const std::shared_ptr<Protocol>& protocol);
void remove_protocol_from_autosend(const std::shared_ptr<Protocol>& protocol);

} // namespace tfs::net

#endif // FS_OUTPUTMESSAGE_H
