// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_CONNECTION_H
#define FS_CONNECTION_H

#include "networkmessage.h"

class Protocol;
class OutputMessage;
class Connection;
class ServicePort;

/**
 * @brief Write timeout in seconds. Configurable for tests.
 */
static inline int32_t CONNECTION_WRITE_TIMEOUT = 30;

/**
 * @brief Read timeout in seconds. Configurable for tests.
 */
static inline int32_t CONNECTION_READ_TIMEOUT = 30;

/**
 * @class ConnectionManager
 * @brief Tracks all active Connection instances.
 *
 * Connections register on creation (createConnection) and unregister
 * on close (releaseConnection). closeAll() provides a clean shutdown
 * of every tracked connection. Thread-safe.
 */
class ConnectionManager : public std::enable_shared_from_this<ConnectionManager>
{
public:
	ConnectionManager() = default;

	ConnectionManager(const ConnectionManager&) = delete;
	ConnectionManager& operator=(const ConnectionManager&) = delete;

	/**
	 * @brief Creates a new Connection and begins tracking it.
	 *
	 * @param {io_context} The ASIO context for async operations.
	 * @param {servicePort} The owning service port.
	 * @return A shared pointer to the new Connection.
	 */
	std::shared_ptr<Connection> createConnection(boost::asio::io_context& io_context,
	                                             std::shared_ptr<ServicePort> servicePort);

	/**
	 * @brief Stops tracking a connection.
	 *
	 * Called automatically by Connection::close().
	 */
	void releaseConnection(const std::shared_ptr<Connection>& connection);

	/**
	 * @brief Closes and releases all tracked connections.
	 */
	void closeAll();

	/**
	 * @brief Returns the number of tracked connections.
	 */
	size_t size() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return activeConnections.size();
	}

private:
	std::unordered_set<std::shared_ptr<Connection>> activeConnections;
	mutable std::mutex mutex;
};

/**
 * @class Connection
 * @brief Manages a single TCP connection with async read/write and framing.
 *
 * Supports two handshake modes:
 *   - Server-sends-first (accept(Protocol*)): reads the client name first,
 *     then enters the sequence read loop.
 *   - Client-sends-first (accept()): enters the sequence read loop
 *     immediately.
 *
 * Steady-state read loop: startReadSequence() -> parseHeader()
 * -> parsePacket() -> startReadSequence().
 */
class Connection : public std::enable_shared_from_this<Connection>
{
public:
	// non-copyable
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;

	/**
	 * @brief Pass true to close() to skip pending writes.
	 */
	static constexpr bool FORCE_CLOSE = true;

	/**
	 * @brief Constructs a Connection with a socket and manager.
	 *
	 * @param {io_context} The ASIO context for async operations.
	 * @param {servicePort} The owning service port.
	 * @param {manager} The ConnectionManager to register with.
	 * @param {socket} A pre-connected socket.
	 */
	Connection(boost::asio::io_context& io_context, std::shared_ptr<ServicePort> servicePort,
	           std::shared_ptr<ConnectionManager> manager, boost::asio::ip::tcp::socket socket);

	/**
	 * @brief Closes the socket if still open.
	 */
	~Connection();

	friend class ConnectionManager;

	/** @name Public API */
	///@{

	/**
	 * @brief Gracefully or forcefully closes the connection.
	 *
	 * Marks the connection as closed, releases it from the manager,
	 * releases the protocol, and either closes the socket immediately
	 * (force) or defers until pending writes complete.
	 *
	 * @param {force} If true, close the socket even with pending writes.
	 */
	void close(bool force = false);

	/**
	 * @brief Initiates the server-sends-first handshake.
	 *
	 * Stores the protocol, dispatches onConnect(), captures the
	 * remote address, and begins reading the client name.
	 *
	 * @param {protocol} The pre-created protocol instance.
	 */
	void accept(std::shared_ptr<Protocol> protocol);

	/**
	 * @brief Initiates the client-sends-first handshake.
	 *
	 * Captures the remote address and enters the sequence read loop
	 * immediately (no name detection).
	 */
	void accept();

	/**
	 * @brief Queues a message for sending.
	 *
	 * If no write is in progress, posts internalSend() to the
	 * socket executor immediately.
	 *
	 * @param {msg} The message to send.
	 */
	void send(const std::shared_ptr<OutputMessage>& msg);

	/**
	 * @brief Returns the remote IP address.
	 */
	const auto& getRemoteAddress() const { return remoteAddress; };

	///@}

	/** @name Testing helpers */
	///@{

	/**
	 * @brief Checks whether the client exceeded the per-second packet limit.
	 *
	 * @return false if the rate limit was exceeded (connection was closed).
	 */
	bool checkRateLimit();

	/**
	 * @brief Handles the first message on a new connection.
	 *
	 * For the status protocol (no protocol set): detects and skips
	 * deprecated checksum bytes, then creates the protocol instance.
	 * For the game protocol: skips the enter-game opcode.
	 *
	 * @return false if protocol creation failed (connection was closed).
	 */
	bool onFirstMessage();

	/**
	 * @brief Closes the socket and cancels all timers.
	 *
	 * Safe to call multiple times — returns early if not open.
	 */
	void closeSocket();

	/**
	 * @brief Exposes the underlying TCP socket for testing.
	 */
	boost::asio::ip::tcp::socket& getSocket() { return socket; }

	/**
	 * @brief Starts the read timeout timer.
	 *
	 * On expiration, onTimeout() force-closes the connection.
	 */
	void startReadTimer();

	/**
	 * @brief Starts the write timeout timer.
	 *
	 * On expiration, onTimeout() force-closes the connection.
	 */
	void startWriteTimer();

	/**
	 * @brief Callback for read/write timer expiration.
	 *
	 * Closes the connection if the timer fires and was not
	 * cancelled by the corresponding completion handler.
	 *
	 * @param {connectionWeak} Prevents keeping the connection alive.
	 * @param {error} operation_aborted if the timer was cancelled.
	 */
	static void onTimeout(std::weak_ptr<Connection> connectionWeak, const boost::system::error_code& error);

	/**
	 * @brief Reads the next packet sequence header (u16 block count).
	 *
	 * This is the steady-state read loop for all protocols.
	 * The result is processed by parseHeader().
	 */
	void startReadSequence();

	/**
	 * @brief Processes a completed write operation.
	 *
	 * Dequeues the sent message, sends the next queued message, or
	 * shuts down the socket if the connection was closed and the
	 * queue is empty.
	 *
	 * @param {error} Boost.Asio error code.
	 */
	void onWriteComplete(const boost::system::error_code& error);

	///@}

	/** @name State queries */
	///@{

	/**
	 * @brief Returns true after close() is called.
	 */
	bool isClosed() const { return closed; }

	/**
	 * @brief Returns true after onFirstMessage() succeeds.
	 */
	bool isFirstMessageProcessed() const { return firstMessageProcessed; }

	/**
	 * @brief Returns the number of packets received in the current rate window.
	 */
	uint32_t getWindowPacketCount() const { return windowPacketCount; }

	/**
	 * @brief Returns the number of messages queued for sending.
	 */
	size_t getWriteQueueSize() const { return writeQueue.size(); }

	///@}

private:
	/**
	 * @enum NameState
	 * @brief Tracks the server-name detection phase.
	 */
	enum class NameState
	{
		Waiting, ///< No name bytes received yet.
		Reading, ///< Accumulating multi-byte name.
		Complete ///< Name reception finished.
	};

	/** @name Async initiators */
	///@{

	/**
	 * @brief Reads the server name from the client byte-by-byte.
	 *
	 * Used by server-sends-first protocols (e.g., game) to detect
	 * which server the client intends to connect to.
	 */
	void startReadServerName();

	/**
	 * @brief Processes a server name read result.
	 *
	 * Accumulates name bytes until 0x0A (end marker), then transitions
	 * to the sequence read loop via startReadSequence().
	 *
	 * @param {error} Boost.Asio error code.
	 */
	void onServerNameRead(const boost::system::error_code& error);

	/**
	 * @brief Processes a sequence header (block count).
	 *
	 * Performs rate limiting, validates size, then reads the full
	 * packet body. The body is processed by parsePacket().
	 *
	 * @param {error} Boost.Asio error code.
	 */
	void parseHeader(const boost::system::error_code& error);

	/**
	 * @brief Processes a complete packet body.
	 *
	 * Reads the checksum, dispatches the message to the protocol
	 * (onRecvFirstMessage or onRecvMessage), then loops back to
	 * startReadSequence().
	 *
	 * @param {error} Boost.Asio error code.
	 */
	void parsePacket(const boost::system::error_code& error);

	///@}

	/** @name Helpers */
	///@{

	/**
	 * @brief Sends a message immediately (no queue check).
	 *
	 * Called by send() for the first pending message and by
	 * onWriteComplete() for subsequent queued messages.
	 *
	 * @param {msg} The message to send.
	 */
	void internalSend(const std::shared_ptr<OutputMessage>& msg);

	///@}

public:
	/** @name Members */
	///@{

	/** @brief Buffer for incoming data reads. */
	NetworkMessage msg;

	/** @brief Remote peer IP address. */
	boost::asio::ip::address remoteAddress;

	/** @brief The TCP socket. */
	boost::asio::ip::tcp::socket socket;

	/** @brief Read timeout timer. */
	boost::asio::steady_timer readTimer;

	/** @brief Write timeout timer. */
	boost::asio::steady_timer writeTimer;

	/** @brief Guards all mutable state. */
	std::recursive_mutex mutex;

	/** @brief Outbound messages awaiting transmission. */
	std::list<std::shared_ptr<OutputMessage>> writeQueue;

	/** @brief The owning service port. */
	std::shared_ptr<ServicePort> servicePort;

	/** @brief The active protocol handler. */
	std::shared_ptr<Protocol> protocol;

	/** @brief The connection manager for lifecycle. */
	std::weak_ptr<ConnectionManager> manager;

	/** @brief Start of the rate-limiting time window. */
	std::chrono::steady_clock::time_point rateWindowStart;

	/** @brief Packets received in the current rate window. */
	uint32_t windowPacketCount = 0;

	/** @brief true after close() is called. */
	bool closed = false;

	/** @brief true after onFirstMessage() succeeds. */
	bool firstMessageProcessed = false;

	/** @brief Server-name detection phase. */
	NameState nameState = NameState::Waiting;

	/** @brief true after readTimer async_wait is registered. */
	bool readTimerArmed = false;

	/** @brief true after writeTimer async_wait is registered. */
	bool writeTimerArmed = false;

	///@}
};

#endif // FS_CONNECTION_H
