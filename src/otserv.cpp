// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "application/app.h"
#include "configmanager.h"
#include "modules/app_loop/module.h"
#include "modules/server/module.h"
#include "tools.h"

namespace {

bool argumentsHandler(const std::vector<std::string_view>& args)
{
	for (const auto& arg : args) {
		if (arg == "--help") {
			std::println("Usage:");
			std::println("  --config=<path>      Alternate configuration file path.");
			std::println("  --ip=<address>       IP address of the server.");
			std::println("  --http-port=<port>   Port for http to listen on.");
			std::println("  --game-port=<port>   Port for game server to listen on.");
			std::println("  --log-level=<level>  Logging level (trace, debug, info, warn, error, critical).");
			return false;
		} else if (arg == "--version") {
			tfs::modules::server::printServerVersion();
			return false;
		}

		auto tmp = explodeString(arg, "=");
		if (tmp.size() < 2) {
			continue;
		}

		if (tmp[0] == "--config")
			ConfigManager::setString(ConfigManager::CONFIG_FILE, tmp[1]);
		else if (tmp[0] == "--ip")
			ConfigManager::setString(ConfigManager::IP, tmp[1]);
		else if (tmp[0] == "--http-port")
			ConfigManager::setNumber(ConfigManager::HTTP_PORT, std::stoi(tmp[1].data()));
		else if (tmp[0] == "--game-port")
			ConfigManager::setNumber(ConfigManager::GAME_PORT, std::stoi(tmp[1].data()));
		else if (tmp[0] == "--log-level") {
			auto level = spdlog::level::from_str(std::string{tmp[1]});
			if (level == spdlog::level::off && tmp[1] != "off") {
				std::println("Invalid log level: {}", tmp[1]);
				return false;
			}
			spdlog::set_level(level);
		}
	}

	return true;
}

} // namespace

int main(int argc, const char** argv)
{
	std::vector<std::string_view> args(argv, argv + argc);
	if (!argumentsHandler(args)) {
		return 1;
	}

	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
	[[maybe_unused]] auto spdlog_exit = tfs::scope_exit([] { spdlog::shutdown(); });

	auto app = std::make_unique<tfs::application::App>();
	app->add_module<tfs::modules::app_loop::AppLoopModule>();
	app->add_module<tfs::modules::server::ServerModule>();
	return app->run();
}
