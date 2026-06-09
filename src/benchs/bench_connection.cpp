#include "../otpch.h"

#include "../configmanager.h"
#include "../connection.h"
#include "../outputmessage.h"
#include "../protocol.h"
#include "../server.h"

#include <benchmark/benchmark.h>

class MockProtocol : public Protocol
{
public:
	explicit MockProtocol(std::shared_ptr<Connection> connection) : Protocol(std::move(connection)) {}

	void onRecvFirstMessage(NetworkMessage&) override {}
};

struct BenchFixture
{
	boost::asio::io_context ioContext;
	std::shared_ptr<ConnectionManager> manager = std::make_shared<ConnectionManager>();
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket clientSocket;
	std::shared_ptr<Connection> connection;
	std::shared_ptr<MockProtocol> protocol;

	BenchFixture() :
	    acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 0)),
	    clientSocket(ioContext)
	{
		ConfigManager::setNumber(ConfigManager::MAX_PACKETS_PER_SECOND, 100000);

		auto endpoint = acceptor.local_endpoint();
		clientSocket.async_connect(endpoint, [](auto) {});

		boost::asio::ip::tcp::socket serverSocket(ioContext);
		acceptor.async_accept(serverSocket, [](auto) {});

		ioContext.run();
		ioContext.restart();

		connection = std::make_shared<Connection>(ioContext, nullptr, manager, std::move(serverSocket));
		connection->protocol = std::make_shared<MockProtocol>(connection);
		protocol = std::static_pointer_cast<MockProtocol>(connection->protocol);
	}

	~BenchFixture()
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

	void runIO() { ioContext.poll(); }
};

static void bench_rate_limit(benchmark::State& state)
{
	BenchFixture fixture;
	for (auto _ : state) {
		fixture.connection->checkRateLimit();
	}
}
BENCHMARK(bench_rate_limit);

static void bench_send_packet(benchmark::State& state)
{
	BenchFixture fixture;
	auto payloadSize = static_cast<size_t>(state.range(0));
	for (auto _ : state) {
		auto message = tfs::net::make_output_message();
		message->addPaddingBytes(payloadSize);
		fixture.connection->send(message);
		fixture.runIO();
	}
}
BENCHMARK(bench_send_packet)->Range(64, 4096);

static void bench_accept_cycle(benchmark::State& state)
{
	BenchFixture fixture;
	fixture.connection->protocol = fixture.protocol;
	fixture.connection->accept();

	size_t bodySize = static_cast<size_t>(state.range(0));
	std::vector<uint8_t> packet(2 + 4 + bodySize, 0);
	uint16_t blockCount = static_cast<uint16_t>((bodySize + 4) / 8);
	uint32_t checksum = 0;
	std::memcpy(packet.data(), &blockCount, 2);
	std::memcpy(packet.data() + 2, &checksum, 4);

	for (auto _ : state) {
		fixture.connection->rateWindowStart = std::chrono::steady_clock::now();
		fixture.connection->windowPacketCount = 0;

		fixture.writeToConnection(packet);
		fixture.runIO();
	}
}
BENCHMARK(bench_accept_cycle)->Range(8, 256);

static void bench_concurrent_connections(benchmark::State& state)
{
	auto manager = std::make_shared<ConnectionManager>();
	boost::asio::io_context context;

	size_t count = static_cast<size_t>(state.range(0));
	for (auto _ : state) {
		std::vector<std::shared_ptr<Connection>> connections;
		connections.reserve(count);
		for (size_t i = 0; i < count; ++i) {
			connections.push_back(manager->createConnection(context, nullptr));
		}
		for (auto& c : connections) {
			c->close();
		}
	}
}
BENCHMARK(bench_concurrent_connections)->RangeMultiplier(4)->Range(1, 1024);

BENCHMARK_MAIN();
