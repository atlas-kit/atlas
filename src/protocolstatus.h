// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_PROTOCOLSTATUS_H
#define FS_PROTOCOLSTATUS_H

#include "protocol.h"

class ProtocolStatus final : public Protocol
{
public:
	// static protocol information
	static constexpr auto server_sends_first = false;
	static constexpr uint8_t protocol_identifier = 0xFF;
	static constexpr auto use_checksum = false;
	static const char* protocol_name() { return "status protocol"; }

	explicit ProtocolStatus(std::shared_ptr<Connection> connection) : Protocol(std::move(connection)) {}

	void onRecvFirstMessage(NetworkMessage& msg) override;

	void sendStatusString();
	void sendInfo(uint16_t requestedInfo, const std::string& characterName);
};

#endif // FS_PROTOCOLSTATUS_H
