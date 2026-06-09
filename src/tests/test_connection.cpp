#define BOOST_TEST_MODULE connection

#include "../otpch.h"

#include "../configmanager.h"
#include "../connection.h"
#include "../outputmessage.h"
#include "../protocol.h"
#include "../server.h"
#include "../tasks.h"

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

class MockProtocol : public Protocol
{
public:
	bool recvFirstCalled = false;
	bool recvCalled = false;
	bool sendCalled = false;
	bool connectCalled = false;

	explicit MockProtocol(std::shared_ptr<Connection> connection) : Protocol(std::move(connection)) {}

	void onRecvFirstMessage(NetworkMessage&) override { recvFirstCalled = true; }
	void onRecvMessage(NetworkMessage&) override { recvCalled = true; }
	void onSendMessage(const std::shared_ptr<OutputMessage>& msg) override
	{
		sendCalled = true;
		Protocol::onSendMessage(msg);
	}
	void onConnect() override { connectCalled = true; }
};

// Establishes a real loopback TCP connection between clientSocket
// and the Connection's internal socket, allowing async I/O tests.
struct ConnectionFixture
{
	boost::asio::io_context ioContext;
	std::shared_ptr<ConnectionManager> manager = std::make_shared<ConnectionManager>();
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket clientSocket;
	std::shared_ptr<Connection> connection;
	std::shared_ptr<MockProtocol> mockProtocol;

	ConnectionFixture() :
	    acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 0)),
	    clientSocket(ioContext)
	{
		ConfigManager::setNumber(ConfigManager::MAX_PACKETS_PER_SECOND, 1000);

		auto endpoint = acceptor.local_endpoint();
		clientSocket.async_connect(endpoint, [](auto) {});

		boost::asio::ip::tcp::socket serverSocket(ioContext);
		acceptor.async_accept(serverSocket, [](auto) {});

		ioContext.run();
		ioContext.restart();

		connection = std::make_shared<Connection>(ioContext, nullptr, manager, std::move(serverSocket));
		mockProtocol = std::make_shared<MockProtocol>(connection);
	}

	~ConnectionFixture()
	{
		if (clientSocket.is_open()) {
			clientSocket.close();
		}
		ioContext.restart();
	}

	void writeToConnection(const std::vector<uint8_t>& data)
	{
		boost::asio::write(clientSocket, boost::asio::buffer(data));
	}

	std::vector<uint8_t> readFromConnection(size_t bytes)
	{
		std::vector<uint8_t> buffer(bytes);
		boost::asio::read(clientSocket, boost::asio::buffer(buffer));
		return buffer;
	}

	void runIO() { ioContext.poll(); }

	// Wire format sent to the Connection: [blockCount:u16] [checksum:u32] [body...]
	// blockCount = ceil((bodySize + 4) / 8) per the XTEA framing protocol.
	void writeFramedPacket(const std::vector<uint8_t>& body)
	{
		uint16_t blockCount = static_cast<uint16_t>((body.size() + 4) / 8);
		uint32_t checksum = 0;
		std::vector<uint8_t> packet(2 + 4 + body.size());
		std::memcpy(packet.data(), &blockCount, 2);
		std::memcpy(packet.data() + 2, &checksum, 4);
		std::memcpy(packet.data() + 6, body.data(), body.size());
		writeToConnection(packet);
	}
};

BOOST_AUTO_TEST_CASE(manager_release_untracks)
{
	auto manager = std::make_shared<ConnectionManager>();
	boost::asio::io_context context;
	auto firstConnection = manager->createConnection(context, nullptr);
	auto secondConnection = manager->createConnection(context, nullptr);
	BOOST_TEST(manager->size() == 2);
	manager->releaseConnection(firstConnection);
	BOOST_TEST(manager->size() == 1);
	manager->releaseConnection(secondConnection);
	BOOST_TEST(manager->size() == 0);
}

BOOST_AUTO_TEST_CASE(manager_close_all_empties)
{
	auto manager = std::make_shared<ConnectionManager>();
	boost::asio::io_context context;
	manager->createConnection(context, nullptr);
	manager->createConnection(context, nullptr);
	manager->createConnection(context, nullptr);
	manager->closeAll();
	BOOST_TEST(manager->size() == 0);
}

BOOST_AUTO_TEST_CASE(rate_limit_under_limit_passes)
{
	ConnectionFixture fixture;
	BOOST_TEST(fixture.connection->checkRateLimit());
}

BOOST_AUTO_TEST_CASE(rate_limit_over_limit_closes)
{
	ConnectionFixture fixture;
	ConfigManager::setNumber(ConfigManager::MAX_PACKETS_PER_SECOND, 1);
	for (int i = 0; i < 5; ++i) {
		fixture.connection->checkRateLimit();
	}
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(rate_limit_zero_limit_closes_immediately)
{
	ConnectionFixture fixture;
	// Zero packets per second means even the first packet exceeds the limit.
	ConfigManager::setNumber(ConfigManager::MAX_PACKETS_PER_SECOND, 0);
	fixture.connection->checkRateLimit();
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(close_socket_idempotent)
{
	ConnectionFixture fixture;
	BOOST_TEST(fixture.connection->getSocket().is_open());
	fixture.connection->closeSocket();
	BOOST_TEST(!fixture.connection->getSocket().is_open());
	fixture.connection->closeSocket();
	BOOST_TEST(!fixture.connection->getSocket().is_open());
}

BOOST_AUTO_TEST_CASE(destructor_closes_socket)
{
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket socket(context);
	socket.open(boost::asio::ip::tcp::v4());
	bool wasOpen = socket.is_open();

	{
		auto manager = std::make_shared<ConnectionManager>();
		auto connection = std::make_shared<Connection>(context, nullptr, manager, std::move(socket));
	}
	BOOST_TEST(wasOpen);
	BOOST_TEST(!socket.is_open());
}

BOOST_AUTO_TEST_CASE(send_when_closed_ignores_message)
{
	ConnectionFixture fixture;
	fixture.connection->close();
	auto message = tfs::net::make_output_message();
	message->addBytes("\x01", 1);
	fixture.connection->send(message);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 0);
}

BOOST_AUTO_TEST_CASE(send_queues_message)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto message = tfs::net::make_output_message();
	message->addBytes("\x01", 1);
	fixture.connection->send(message);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 1);
}

BOOST_AUTO_TEST_CASE(double_close_is_safe)
{
	ConnectionFixture fixture;
	fixture.connection->close();
	BOOST_TEST(fixture.connection->isClosed());
	fixture.connection->close();
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(force_close_skips_pending_writes)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto message = tfs::net::make_output_message();
	message->addBytes("\x01", 1);
	fixture.connection->send(message);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 1);

	fixture.connection->close(Connection::FORCE_CLOSE);
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(on_first_message_without_protocol_closes)
{
	ConnectionFixture fixture;

	// Manually create a connection with a ServicePort that has no registered services.
	// When onFirstMessage runs without a protocol, it calls servicePort->make_protocol,
	// which returns null for unregistered services, triggering close.
	boost::asio::ip::tcp::socket serverSocket(fixture.ioContext);
	auto servicePort = std::make_shared<ServicePort>(fixture.ioContext);
	auto connection =
	    std::make_shared<Connection>(fixture.ioContext, servicePort, fixture.manager, std::move(serverSocket));

	connection->msg.addPaddingBytes(10);
	connection->msg.setLength(10);

	BOOST_TEST(!connection->onFirstMessage());
	BOOST_TEST(connection->isClosed());
}

BOOST_AUTO_TEST_CASE(close_graceful_without_pending_writes)
{
	ConnectionFixture fixture;

	// close(false) with an empty queue should close the socket immediately.
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 0);
	fixture.connection->close(false);
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(is_first_message_processed_after_on_first_message)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	BOOST_TEST(!fixture.connection->isFirstMessageProcessed());
	fixture.connection->msg.addBytes("\x01\x02payload", 9);
	fixture.connection->msg.setLength(9);
	fixture.connection->onFirstMessage();
	BOOST_TEST(fixture.connection->isFirstMessageProcessed());
}

BOOST_AUTO_TEST_CASE(get_window_packet_count_after_check_rate_limit)
{
	ConnectionFixture fixture;
	BOOST_TEST(fixture.connection->getWindowPacketCount() == 0);
	fixture.connection->checkRateLimit();
	BOOST_TEST(fixture.connection->getWindowPacketCount() == 1);
	fixture.connection->checkRateLimit();
	BOOST_TEST(fixture.connection->getWindowPacketCount() == 2);
}

BOOST_AUTO_TEST_CASE(write_complete_success_sends_next_queued)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto firstMessage = tfs::net::make_output_message();
	firstMessage->addBytes("\x01", 1);
	fixture.connection->send(firstMessage);

	auto secondMessage = tfs::net::make_output_message();
	secondMessage->addBytes("\x02", 1);
	fixture.connection->send(secondMessage);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 2);

	fixture.connection->onWriteComplete(boost::system::error_code{});
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 1);
	BOOST_TEST(!fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(write_complete_success_closes_socket_when_closed_flag_set)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto message = tfs::net::make_output_message();
	message->addBytes("\x01", 1);
	fixture.connection->send(message);

	fixture.connection->close(false);
	BOOST_TEST(fixture.connection->isClosed());

	fixture.connection->onWriteComplete(boost::system::error_code{});
	BOOST_TEST(!fixture.connection->getSocket().is_open());
}

BOOST_AUTO_TEST_CASE(accept_server_first_socket_error_during_name)
{
	ConnectionFixture fixture;

	// Start name detection, then close the client socket so the async read fails.
	fixture.connection->accept(fixture.mockProtocol);

	fixture.clientSocket.close();
	fixture.runIO();

	// Connection should detect the socket error and close.
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(parse_header_without_protocol_rejects_zero_size)
{
	ConnectionFixture fixture;

	// Without protocol: size = header value directly. blockCount = 0 → size = 0 → close.
	fixture.connection->startReadSequence();

	uint16_t blockCount = 0;
	uint32_t checksum = 0;
	std::vector<uint8_t> packet(2 + 4);
	std::memcpy(packet.data(), &blockCount, 2);
	std::memcpy(packet.data() + 2, &checksum, 4);

	fixture.writeToConnection(packet);
	fixture.runIO();

	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(remote_address_set_on_accept)
{
	ConnectionFixture fixture;

	// accept() captures the remote endpoint address
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address("127.0.0.1"), 0);
	auto address = fixture.connection->getRemoteAddress();
	BOOST_TEST(address.is_unspecified());
}

BOOST_AUTO_TEST_CASE(write_queue_queues_multiple_messages)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto firstMessage = tfs::net::make_output_message();
	firstMessage->addBytes("\x01", 1);
	fixture.connection->send(firstMessage);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 1);

	auto secondMessage = tfs::net::make_output_message();
	secondMessage->addBytes("\x02", 1);
	fixture.connection->send(secondMessage);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 2);
}

BOOST_AUTO_TEST_CASE(write_complete_error_clears_queue_and_closes)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto message = tfs::net::make_output_message();
	message->addBytes("\x01", 1);
	fixture.connection->send(message);
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 1);

	// Simulate a write error directly — verifies the error handler clears the
	// queue and force-closes the connection without involving the real socket.
	fixture.connection->onWriteComplete(boost::asio::error::connection_reset);

	BOOST_TEST(fixture.connection->isClosed());
	BOOST_TEST(fixture.connection->getWriteQueueSize() == 0);
}

BOOST_AUTO_TEST_CASE(read_timeout_triggers_close)
{
	// A zero-second timeout fires on the next io_context tick.
	CONNECTION_READ_TIMEOUT = 0;
	ConnectionFixture fixture;
	fixture.connection->startReadTimer();
	fixture.ioContext.run_one();
	fixture.ioContext.restart();
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(timeout_cancelled_by_close_socket_returns_early)
{
	CONNECTION_READ_TIMEOUT = 0;
	ConnectionFixture fixture;
	fixture.connection->startReadTimer();
	// closeSocket cancels the timer (operation_aborted), so onTimeout
	// must NOT close the connection again.
	fixture.connection->closeSocket();
	fixture.ioContext.run_one();
	fixture.ioContext.restart();
	BOOST_TEST(!fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(write_timeout_triggers_close)
{
	CONNECTION_WRITE_TIMEOUT = 0;
	ConnectionFixture fixture;
	fixture.connection->startWriteTimer();
	fixture.ioContext.run_one();
	fixture.ioContext.restart();
	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(close_with_expired_manager_does_not_crash)
{
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket socket(context);

	auto manager = std::make_shared<ConnectionManager>();
	auto connection = std::make_shared<Connection>(context, nullptr, manager, std::move(socket));
	// Drop the only shared_ptr to the manager so connection->manager.lock()
	// returns null. close() must handle this gracefully.
	manager.reset();
	connection->close();
	BOOST_TEST(connection->isClosed());
}

BOOST_AUTO_TEST_CASE(accept_client_first_dispatches_first_message)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	fixture.connection->accept();
	fixture.writeFramedPacket({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
	fixture.runIO();

	BOOST_TEST(fixture.mockProtocol->recvFirstCalled);
}

BOOST_AUTO_TEST_CASE(accept_client_first_dispatches_second_message)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	// First packet triggers onRecvFirstMessage.
	fixture.connection->accept();
	fixture.writeFramedPacket({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
	fixture.runIO();
	BOOST_TEST(fixture.mockProtocol->recvFirstCalled);

	fixture.mockProtocol->recvFirstCalled = false;
	fixture.mockProtocol->recvCalled = false;

	// Second packet triggers onRecvMessage via the read loop
	// (parsePacket → startReadSequence loopback).
	fixture.writeFramedPacket({0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10});
	fixture.runIO();

	BOOST_TEST(fixture.mockProtocol->recvCalled);
	BOOST_TEST(!fixture.mockProtocol->recvFirstCalled);
}

BOOST_AUTO_TEST_CASE(accept_client_first_zero_size_packet_without_protocol_closes)
{
	ConnectionFixture fixture;

	fixture.connection->accept();

	// Without protocol: size = header value directly. blockCount = 0 → size = 0 → close.
	uint16_t blockCount = 0;
	uint32_t checksum = 0;
	std::vector<uint8_t> packet(2 + 4);
	std::memcpy(packet.data(), &blockCount, 2);
	std::memcpy(packet.data() + 2, &checksum, 4);

	fixture.writeToConnection(packet);
	fixture.runIO();

	BOOST_TEST(fixture.connection->isClosed());
}

BOOST_AUTO_TEST_CASE(accept_server_first_empty_name_detected)
{
	ConnectionFixture fixture;
	fixture.connection->accept(fixture.mockProtocol);

	// A 2-byte header with second byte == 0x00 means "no name".
	uint8_t header[2] = {0x00, 0x00};
	fixture.writeToConnection({header[0], header[1]});
	fixture.runIO();

	// After name detection completes, the connection reads framed packets.
	fixture.writeFramedPacket({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
	fixture.runIO();

	BOOST_TEST(fixture.mockProtocol->recvFirstCalled);
}

BOOST_AUTO_TEST_CASE(accept_server_first_multi_byte_name_detected)
{
	ConnectionFixture fixture;
	fixture.connection->accept(fixture.mockProtocol);

	// 2-byte header: second byte != 0 starts name accumulation.
	fixture.writeToConnection({0x00, 0x01});
	fixture.runIO();

	// Name characters sent one at a time; each triggers a 1-byte async_read.
	fixture.writeToConnection({'A'});
	fixture.runIO();

	fixture.writeToConnection({'B'});
	fixture.runIO();

	fixture.writeToConnection({'C'});
	fixture.runIO();

	// 0x0A terminates the name.
	fixture.writeToConnection({0x0A});
	fixture.runIO();

	fixture.writeFramedPacket({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
	fixture.runIO();

	BOOST_TEST(fixture.mockProtocol->recvFirstCalled);
}

BOOST_AUTO_TEST_CASE(async_write_invokes_on_send_message)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	auto message = tfs::net::make_output_message();
	message->addBytes("\x01\x02\x03", 3);
	fixture.connection->send(message);
	fixture.runIO();

	BOOST_TEST(fixture.mockProtocol->sendCalled);
}

BOOST_AUTO_TEST_CASE(async_read_oversized_packet_closes)
{
	ConnectionFixture fixture;
	fixture.connection->protocol = fixture.mockProtocol;

	fixture.connection->accept();

	// blockCount = 0xFFFF → computed size exceeds NETWORKMESSAGE_MAXSIZE - 16
	// → parseHeader closes the connection.
	uint16_t blockCount = 0xFFFF;
	uint32_t checksum = 0;
	std::vector<uint8_t> packet(2 + 4);
	std::memcpy(packet.data(), &blockCount, 2);
	std::memcpy(packet.data() + 2, &checksum, 4);

	fixture.writeToConnection(packet);
	fixture.runIO();

	BOOST_TEST(fixture.connection->isClosed());
}
