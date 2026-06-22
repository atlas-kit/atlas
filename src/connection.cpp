// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "connection.h"

#include "configmanager.h"
#include "outputmessage.h"
#include "protocol.h"
#include "server.h"
#include "tasks.h"

#include <print>

extern Dispatcher g_dispatcher;

std::shared_ptr<Connection> ConnectionManager::createConnection(boost::asio::io_context& io_context,
                                                                std::shared_ptr<const ServicePort> servicePort)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	auto connection = std::make_shared<Connection>(io_context, servicePort);
	connections.insert(connection);
	return connection;
}

void ConnectionManager::releaseConnection(const std::shared_ptr<Connection>& connection)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	connections.erase(connection);
}

void ConnectionManager::closeAll()
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	for (const auto& connection : connections) {
		if (!connection->socket.is_open()) {
			continue;
		}

		try {
			connection->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			connection->socket.close();
		} catch (boost::system::system_error& e) {
			std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		}
	}
	connections.clear();
}

// Connection

Connection::Connection(boost::asio::io_context& io_context, std::shared_ptr<const ServicePort> service_port) :
    readTimer(io_context),
    writeTimer(io_context),
    service_port(std::move(service_port)),
    socket(io_context),
    timeConnected(std::chrono::steady_clock::now())
{}

void Connection::close(bool force)
{
	// any thread
	ConnectionManager::getInstance().releaseConnection(shared_from_this());

	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	connectionState = CONNECTION_STATE_DISCONNECTED;

	if (protocol) {
		g_dispatcher.addTask([protocol = protocol]() { protocol->release(); });
	}

	if (messageQueue.empty() || force) {
		closeSocket();
	} else {
		// will be closed by the destructor or onWriteOperation
	}
}

void Connection::closeSocket()
{
	if (!socket.is_open()) {
		return;
	}

	try {
		readTimer.cancel();
		writeTimer.cancel();
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
	}
}

Connection::~Connection() { closeSocket(); }

void Connection::accept(std::shared_ptr<Protocol> protocol)
{
	this->protocol = protocol;
	g_dispatcher.addTask([=]() { protocol->onConnect(); });
	connectionState = CONNECTION_STATE_GAMEWORLD_AUTH;
	accept();
}

void Connection::accept()
{
	if (connectionState == CONNECTION_STATE_PENDING) {
		connectionState = CONNECTION_STATE_REQUEST_CHARLIST;
	}

	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);

	try {
		readTimer.expires_after(std::chrono::seconds(CONNECTION_READ_TIMEOUT));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Read size of the first packet
		auto bufferLength = !receivedLastChar && receivedName && connectionState == CONNECTION_STATE_GAMEWORLD_AUTH
		                        ? 1
		                        : NetworkMessage::HEADER_LENGTH;
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), bufferLength),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::accept] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::parseHeader(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	readTimer.cancel();

	if (error) {
		close(FORCE_CLOSE);
		return;
	} else if (connectionState == CONNECTION_STATE_DISCONNECTED) {
		return;
	}

	auto timePassed =
	    std::max(1s, duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeConnected) + 1s);
	if ((++packetsSent / timePassed.count()) >
	    static_cast<uint32_t>(getNumber(ConfigManager::MAX_PACKETS_PER_SECOND))) {
		std::cout << getIP() << " disconnected for exceeding packet per second limit." << std::endl;
		close();
		return;
	}

	if (!receivedLastChar && connectionState == CONNECTION_STATE_GAMEWORLD_AUTH) {
		uint8_t* msgBuffer = msg.getBuffer();

		if (!receivedName && msgBuffer[1] == 0x00) {
			receivedLastChar = true;
		} else {
			if (!receivedName) {
				receivedName = true;

				accept();
				return;
			}

			if (msgBuffer[0] == 0x0A) {
				receivedLastChar = true;
			}

			accept();
			return;
		}
	}

	if (receivedLastChar && connectionState == CONNECTION_STATE_GAMEWORLD_AUTH) {
		connectionState = CONNECTION_STATE_GAME;
	}

	if (timePassed > 2s) {
		timeConnected = std::chrono::steady_clock::now();
		packetsSent = 0;
	}

	uint16_t size = (msg.getLengthHeader() * 8) + NetworkMessage::CHECKSUM_LENGTH;
	if (size == 0 || size >= NETWORKMESSAGE_MAXSIZE - 16) {
		close(FORCE_CLOSE);
		return;
	}

	try {
		readTimer.expires_after(std::chrono::seconds(CONNECTION_READ_TIMEOUT));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Read packet content
		msg.setLength(size + NetworkMessage::HEADER_LENGTH);
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBodyBuffer(), size),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parsePacket(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::parseHeader] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::parsePacket(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	readTimer.cancel();

	if (error) {
		close(FORCE_CLOSE);
		return;
	} else if (connectionState == CONNECTION_STATE_DISCONNECTED) {
		return;
	}

	// Read potential checksum bytes
	msg.get<uint32_t>();

	if (!receivedFirst) {
		receivedFirst = true;

		if (!protocol) {
			// Skip deprecated checksum bytes (with clients that aren't using it in mind)
			uint16_t len = msg.getLength();
			if (len < 280 && len != 151) {
				msg.skipBytes(-NetworkMessage::CHECKSUM_LENGTH);
			}

			// Game protocol has already been created at this point
			protocol = service_port->make_protocol(msg, shared_from_this());
			if (!protocol) {
				close(FORCE_CLOSE);
				return;
			}
		} else {
			msg.skipBytes(2); // Skip enter-game opcode (u16 in 15.24, was u8)
		}

		protocol->onRecvFirstMessage(msg);
	} else {
		protocol->onRecvMessage(msg); // Send the packet to the current protocol
	}

	try {
		readTimer.expires_after(std::chrono::seconds(CONNECTION_READ_TIMEOUT));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Wait to the next packet
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), NetworkMessage::HEADER_LENGTH),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::parsePacket] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::send(const std::shared_ptr<OutputMessage>& msg)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	if (connectionState == CONNECTION_STATE_DISCONNECTED) {
		return;
	}

	bool noPendingWrite = messageQueue.empty();
	messageQueue.emplace_back(msg);
	if (noPendingWrite) {
		try {
			boost::asio::post(socket.get_executor(),
			                  [thisPtr = shared_from_this(), msg] { thisPtr->internalSend(msg); });
		} catch (const boost::system::system_error& e) {
			std::cout << "[Network error - Connection::send] " << e.what() << std::endl;
			messageQueue.clear();
			close(FORCE_CLOSE);
		}
	}
}

void Connection::internalSend(const std::shared_ptr<OutputMessage>& msg)
{
	protocol->onSendMessage(msg);
	try {
		writeTimer.expires_after(std::chrono::seconds(CONNECTION_WRITE_TIMEOUT));
		writeTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		boost::asio::async_write(
		    socket, boost::asio::buffer(msg->getOutputBuffer(), msg->getLength()),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->onWriteOperation(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::internalSend] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::onWriteOperation(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	writeTimer.cancel();
	messageQueue.pop_front();

	if (error) {
		messageQueue.clear();
		close(FORCE_CLOSE);
		return;
	}

	if (!messageQueue.empty()) {
		internalSend(messageQueue.front());
	} else if (connectionState == CONNECTION_STATE_DISCONNECTED) {
		closeSocket();
	}
}

void Connection::handleTimeout(std::weak_ptr<Connection> connectionWeak, const boost::system::error_code& error)
{
	if (error == boost::asio::error::operation_aborted) {
		// The timer has been cancelled manually
		return;
	}

	if (auto connection = connectionWeak.lock()) {
		connection->close(FORCE_CLOSE);
	}
}

const Connection::Address& Connection::initializeIP()
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	boost::system::error_code error;
	if (auto endpoint = socket.remote_endpoint(error); !error) {
		remoteAddress = endpoint.address();
	} else {
		std::cout << "[Network error - Connection::initializeIP] " << error.message() << std::endl;
	}
	return remoteAddress;
}
