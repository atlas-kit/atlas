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
                                                                std::shared_ptr<ServicePort> servicePort)
{
	std::lock_guard<std::mutex> lock(mutex);

	auto connection = std::make_shared<Connection>(io_context, servicePort, shared_from_this(),
	                                               boost::asio::ip::tcp::socket(io_context));
	activeConnections.insert(connection);
	return connection;
}

void ConnectionManager::releaseConnection(const std::shared_ptr<Connection>& connection)
{
	std::lock_guard<std::mutex> lock(mutex);

	activeConnections.erase(connection);
}

void ConnectionManager::closeAll()
{
	std::lock_guard<std::mutex> lock(mutex);

	for (const auto& connection : activeConnections) {
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
	activeConnections.clear();
}

Connection::Connection(boost::asio::io_context& io_context, std::shared_ptr<ServicePort> servicePort,
                       std::shared_ptr<ConnectionManager> manager, boost::asio::ip::tcp::socket socket) :
    socket(std::move(socket)),
    readTimer(io_context),
    writeTimer(io_context),
    servicePort(std::move(servicePort)),
    manager(manager),
    rateWindowStart(std::chrono::steady_clock::now())
{}

Connection::~Connection() { closeSocket(); }

void Connection::close(bool force)
{
	if (auto mgr = manager.lock()) {
		mgr->releaseConnection(shared_from_this());
	}

	std::lock_guard<std::recursive_mutex> lock(mutex);
	closed = true;

	if (protocol) {
		g_dispatcher.addTask([protocol = protocol]() { protocol->release(); });
	}

	if (writeQueue.empty() || force) {
		closeSocket();
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

void Connection::accept(std::shared_ptr<Protocol> protocol)
{
	this->protocol = protocol;

	g_dispatcher.addTask([=]() { protocol->onConnect(); });

	std::lock_guard<std::recursive_mutex> lock(mutex);

	boost::system::error_code error;
	if (auto endpoint = socket.remote_endpoint(error); !error) {
		remoteAddress = endpoint.address();
	}

	startReadServerName();
}

void Connection::accept()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	boost::system::error_code error;
	if (auto endpoint = socket.remote_endpoint(error); !error) {
		remoteAddress = endpoint.address();
	}

	startReadSequence();
}

void Connection::startReadTimer()
{
	readTimer.expires_after(std::chrono::seconds(CONNECTION_READ_TIMEOUT));
	if (!readTimerArmed) {
		readTimer.async_wait([thisPtr = std::weak_ptr<Connection>(shared_from_this())](
		                         const boost::system::error_code& error) {
			if (auto conn = thisPtr.lock()) {
				conn->readTimerArmed = false;
			}
			Connection::onTimeout(thisPtr, error);
		});
		readTimerArmed = true;
	}
}

void Connection::startWriteTimer()
{
	writeTimer.expires_after(std::chrono::seconds(CONNECTION_WRITE_TIMEOUT));
	if (!writeTimerArmed) {
		writeTimer.async_wait([thisPtr = std::weak_ptr<Connection>(shared_from_this())](
		                         const boost::system::error_code& error) {
			if (auto conn = thisPtr.lock()) {
				conn->writeTimerArmed = false;
			}
			Connection::onTimeout(thisPtr, error);
		});
		writeTimerArmed = true;
	}
}

void Connection::onTimeout(std::weak_ptr<Connection> connectionWeak, const boost::system::error_code& error)
{
	if (error == boost::asio::error::operation_aborted) {
		return;
	}

	if (auto connection = connectionWeak.lock()) {
		connection->close(FORCE_CLOSE);
	}
}

void Connection::startReadServerName()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	try {
		startReadTimer();

		const auto bufferLength = nameState != NameState::Waiting ? 1 : NetworkMessage::HEADER_LENGTH;

		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), bufferLength),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->onServerNameRead(error);
		    });
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		close(FORCE_CLOSE);
	}
}

void Connection::startReadSequence()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	try {
		startReadTimer();

		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), NetworkMessage::HEADER_LENGTH),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		close(FORCE_CLOSE);
	}
}

void Connection::onServerNameRead(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (error) {
		close(FORCE_CLOSE);
		return;
	}

	if (closed) {
		return;
	}

	auto msgBuffer = msg.getBuffer();

	if (nameState == NameState::Waiting) {
		if (msgBuffer[1] == 0x00) {
			nameState = NameState::Complete;
			startReadSequence();
		} else {
			nameState = NameState::Reading;
			startReadServerName();
		}
		return;
	}

	if (msgBuffer[0] == 0x0A) {
		nameState = NameState::Complete;
		startReadSequence();
		return;
	}

	startReadServerName();
}

void Connection::parseHeader(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (error) {
		close(FORCE_CLOSE);
		return;
	}

	if (closed) {
		return;
	}

	if (!checkRateLimit()) {
		return;
	}

	auto size = msg.getLengthHeader();
	if (protocol) {
		size = (size * NetworkMessage::XTEA_MULTIPLE) + NetworkMessage::CHECKSUM_LENGTH;
	}

	if (size == 0 || size >= NETWORKMESSAGE_MAXSIZE - 16) {
		close(FORCE_CLOSE);
		return;
	}

	try {
		startReadTimer();

		msg.setLength(size + NetworkMessage::HEADER_LENGTH);
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBodyBuffer(), size),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parsePacket(error);
		    });
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		close(FORCE_CLOSE);
	}
}

void Connection::parsePacket(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (error) {
		close(FORCE_CLOSE);
		return;
	}

	if (closed) {
		return;
	}

	msg.get<uint32_t>();

	if (!firstMessageProcessed) {
		if (!onFirstMessage()) {
			return;
		}
	} else {
		protocol->onRecvMessage(msg);
	}

	try {
		startReadTimer();

		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), NetworkMessage::HEADER_LENGTH),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		close(FORCE_CLOSE);
	}
}

void Connection::onWriteComplete(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	writeTimer.cancel();
	writeQueue.pop_front();

	if (error) {
		writeQueue.clear();
		close(FORCE_CLOSE);
		return;
	}

	if (!writeQueue.empty()) {
		internalSend(writeQueue.front());
	}

	if (closed) {
		closeSocket();
	}
}

bool Connection::checkRateLimit()
{
	auto now = std::chrono::steady_clock::now();
	auto elapsed = duration_cast<std::chrono::seconds>(now - rateWindowStart).count() + 1;

	if ((++windowPacketCount / static_cast<uint32_t>(elapsed)) >
	    static_cast<uint32_t>(getNumber(ConfigManager::MAX_PACKETS_PER_SECOND))) {
		std::cout << getRemoteAddress() << " disconnected for exceeding packet per second limit." << std::endl;
		close();
		return false;
	}

	if (elapsed >= 3) {
		rateWindowStart = now;
		windowPacketCount = 0;
	}

	return true;
}

bool Connection::onFirstMessage()
{
	firstMessageProcessed = true;

	if (!protocol) {
		auto len = msg.getLength();
		if (len < 280 && len != 151) {
			msg.skipBytes(-NetworkMessage::CHECKSUM_LENGTH);
		}

		protocol = servicePort->make_protocol(msg, shared_from_this());
		if (!protocol) {
			close(FORCE_CLOSE);
			return false;
		}
	} else {
		msg.skipBytes(2);
	}

	protocol->onRecvFirstMessage(msg);
	return true;
}

void Connection::send(const std::shared_ptr<OutputMessage>& msg)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	if (closed) {
		return;
	}

	bool noPendingWrite = writeQueue.empty();
	writeQueue.emplace_back(msg);
	if (noPendingWrite) {
		try {
			boost::asio::post(socket.get_executor(),
			                  [thisPtr = shared_from_this(), msg] { thisPtr->internalSend(msg); });
		} catch (const boost::system::system_error& e) {
			std::println("[Network error - {}] {}", __FUNCTION__, e.what());
			writeQueue.clear();
			close(FORCE_CLOSE);
		}
	}
}

void Connection::internalSend(const std::shared_ptr<OutputMessage>& msg)
{
	protocol->onSendMessage(msg);
	try {
		startWriteTimer();

		boost::asio::async_write(
		    socket, boost::asio::buffer(msg->getOutputBuffer(), msg->getLength()),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->onWriteComplete(error);
		    });
	} catch (boost::system::system_error& e) {
		std::println("[Network error - {}] {}", __FUNCTION__, e.what());
		close(FORCE_CLOSE);
	}
}
