// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_CONNECTION_H
#define FS_CONNECTION_H

#include "networkmessage.h"

enum ConnectionState_t
{
	CONNECTION_STATE_DISCONNECTED,
	CONNECTION_STATE_REQUEST_CHARLIST,
	CONNECTION_STATE_GAMEWORLD_AUTH,
	CONNECTION_STATE_GAME,
	CONNECTION_STATE_PENDING
};

static constexpr int32_t CONNECTION_WRITE_TIMEOUT = 30;
static constexpr int32_t CONNECTION_READ_TIMEOUT = 30;

class Protocol;
class OutputMessage;
class Connection;
class ServicePort;

class ConnectionManager
{
public:
	static ConnectionManager& getInstance()
	{
		static ConnectionManager instance;
		return instance;
	}

	std::shared_ptr<Connection> createConnection(boost::asio::io_context& io_context,
	                                             std::shared_ptr<const ServicePort> servicePort);
	void releaseConnection(const std::shared_ptr<Connection>& connection);
	void closeAll();

private:
	ConnectionManager() = default;

	std::unordered_set<std::shared_ptr<Connection>> connections;
	std::mutex connectionManagerLock;
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	using Address = boost::asio::ip::address;
	// non-copyable
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;

	enum
	{
		FORCE_CLOSE = true
	};

	Connection(boost::asio::io_context& io_context, std::shared_ptr<const ServicePort> service_port);
	~Connection();

	friend class ConnectionManager;

	void close(bool force = false);
	// Used by protocols that require server to send first
	void accept(std::shared_ptr<Protocol> protocol);
	void accept();

	void send(const std::shared_ptr<OutputMessage>& msg);

	const Address& getIP() const { return remoteAddress; };

private:
	void parseHeader(const boost::system::error_code& error);
	void parsePacket(const boost::system::error_code& error);

	void onWriteOperation(const boost::system::error_code& error);

	static void handleTimeout(std::weak_ptr<Connection> connectionWeak, const boost::system::error_code& error);

	void closeSocket();
	void internalSend(const std::shared_ptr<OutputMessage>& msg);

	boost::asio::ip::tcp::socket& getSocket() { return socket; }
	friend class ServicePort;

	NetworkMessage msg;

	boost::asio::steady_timer readTimer;
	boost::asio::steady_timer writeTimer;

	std::recursive_mutex connectionLock;

	std::list<std::shared_ptr<OutputMessage>> messageQueue;

	std::shared_ptr<const ServicePort> service_port;
	std::shared_ptr<Protocol> protocol;

	boost::asio::ip::tcp::socket socket;
	Address remoteAddress;
	time_t timeConnected;
	uint32_t packetsSent = 0;

	ConnectionState_t connectionState = CONNECTION_STATE_PENDING;
	bool receivedFirst = false;
	bool receivedName = false;
	bool receivedLastChar = false;
};

#endif // FS_CONNECTION_H
