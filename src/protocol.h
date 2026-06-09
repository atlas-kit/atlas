// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_PROTOCOL_H
#define FS_PROTOCOL_H

#include "connection.h"
#include "xtea.h"

#include <zlib.h>

/**
 * @class Protocol
 * @brief Base class for all network protocols.
 *
 * Handles message framing (XTEA encrypt/decrypt, block count header, zlib
 * compression), RSA decryption of the first message, and output buffering.
 *
 * Protocol subclasses define traits (server_sends_first, use_checksum,
 * protocol_identifier) consumed by ServicePort.
 */
class Protocol : public std::enable_shared_from_this<Protocol>
{
public:
	/**
	 * @brief Constructs a Protocol bound to a connection.
	 *
	 * Initializes the zlib deflate stream.
	 *
	 * @param {connection} The owning connection.
	 */
	explicit Protocol(std::shared_ptr<Connection> connection) : connection(std::move(connection))
	{
		if (deflateInit2(&zlibStream, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
			std::cout << "ZLIB initialization error: " << (zlibStream.msg ? zlibStream.msg : "unknown") << std::endl;
		}
	}

	/**
	 * @brief Destroys the protocol and cleans up the zlib stream.
	 */
	virtual ~Protocol();

	// non-copyable
	Protocol(const Protocol&) = delete;
	Protocol& operator=(const Protocol&) = delete;

	/**
	 * @brief Dispatches an incoming parsed message.
	 *
	 * Called by the connection after decryption/checksum stripping.
	 * Subclasses override to implement their packet handlers.
	 *
	 * @param {msg} The parsed incoming message.
	 */
	virtual void parsePacket(NetworkMessage& msg) {}

	/**
	 * @brief Applies outbound framing (block count, XTEA, zlib, checksum).
	 *
	 * Called by Connection::internalSend() before writing to the socket.
	 *
	 * @param {msg} The message to frame and send.
	 */
	virtual void onSendMessage(const std::shared_ptr<OutputMessage>& msg);

	/**
	 * @brief Strips inbound framing and dispatches to parsePacket().
	 *
	 * Called by Connection::parsePacket() after the raw body is received.
	 *
	 * @param {msg} The incoming raw message (contains checksum + encrypted payload).
	 */
	virtual void onRecvMessage(NetworkMessage& msg);

	/**
	 * @brief Handles the very first message on a new connection.
	 *
	 * Subclasses implement protocol-specific handshake logic.
	 *
	 * @param {msg} The first message received.
	 */
	virtual void onRecvFirstMessage(NetworkMessage& msg) = 0;

	/**
	 * @brief Called when the connection is established.
	 *
	 * Hook for any connection-time setup (e.g., sending welcome data).
	 */
	virtual void onConnect() {}

	/**
	 * @brief Checks whether the underlying connection has been closed.
	 */
	bool isConnectionExpired() const { return connection.expired(); }

	/**
	 * @brief Returns a shared pointer to the connection, if still alive.
	 */
	std::shared_ptr<Connection> getConnection() const { return connection.lock(); }

	/**
	 * @brief Returns the remote IP address via the connection.
	 */
	boost::asio::ip::address getRemoteAddress() const;

	/**
	 * @brief Gets an output buffer for writing protocol messages.
	 *
	 * Reuses the existing buffer if it fits, otherwise sends it and allocates a new one.
	 * Use only for auto-send patterns (one buffer shared across multiple writes).
	 *
	 * @param {size} The expected message size to reserve.
	 */
	std::shared_ptr<OutputMessage> getOutputBuffer(int32_t size);

	/**
	 * @brief Returns the current send buffer.
	 */
	std::shared_ptr<OutputMessage>& getSendBuffer() { return sendBuffer; }

	/**
	 * @brief Sends a message through the connection.
	 *
	 * @param {msg} The message to send.
	 */
	void send(std::shared_ptr<OutputMessage> msg) const
	{
		if (auto connection = getConnection()) {
			connection->send(msg);
		}
	}

	/**
	 * @brief Advances and returns the next sequence ID.
	 *
	 * Wraps around at int32_t::max to avoid signed overflow in checksum.
	 */
	uint32_t nextSequenceId()
	{
		const auto sequence = ++sequenceNumber;
		if (sequenceNumber >= static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
			sequenceNumber = 0;
		}
		return sequence;
	}

protected:
	/**
	 * @brief Disconnects from the peer.
	 */
	void disconnect() const
	{
		if (auto connection = getConnection()) {
			connection->close();
		}
	}

	/**
	 * @brief Enables XTEA encryption for outbound messages.
	 */
	void enableEncryption() { encrypted = true; }

	/**
	 * @brief Sets and expands the XTEA encryption key.
	 *
	 * @param {xteaKey} The 128-bit XTEA key.
	 */
	void setXTEAKey(const xtea::key& xteaKey) { this->xteaKey = xtea::expand_key(xteaKey); }

	/**
	 * @brief Compresses a message payload with zlib deflate.
	 *
	 * Used to shrink large packets before XTEA encryption.
	 *
	 * @param {msg} The message to compress (replaced with compressed data on success).
	 * @return true if compression was successful.
	 */
	bool deflateMessage(OutputMessage& msg);

	/**
	 * @brief Enables raw mode (no framing applied).
	 *
	 * When enabled, all encryption, compression and header framing are skipped.
	 */
	void enableRawMode() { rawMode = true; }

	/**
	 * @brief Releases resources held by the protocol.
	 *
	 * Overridden by subclasses that need cleanup (e.g., ProtocolGame).
	 */
	virtual void release() {}

private:
	friend class Connection;

	/** @brief Reusable send buffer for auto-send. */
	std::shared_ptr<OutputMessage> sendBuffer;

	/** @brief The owning connection (weak to avoid cycles). */
	const std::weak_ptr<Connection> connection;

	/** @brief Expanded XTEA round keys. */
	xtea::round_keys xteaKey;

	/** @brief Outgoing message sequence counter. */
	uint32_t sequenceNumber = 0;

	/** @brief true after enableEncryption() is called. */
	bool encrypted = false;

	/** @brief true after enableRawMode() is called. */
	bool rawMode = false;

	/** @brief The zlib deflate stream state. */
	z_stream zlibStream{};
};

#endif // FS_PROTOCOL_H
