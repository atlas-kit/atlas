// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_MODULES_MARKET_RESOURCES_H
#define FS_MODULES_MARKET_RESOURCES_H

#include "../../application/resource.h"
#include "../../application/timing/timer.h"
#include "../../enums.h"

#include <chrono>
#include <map>
#include <memory>

class MarketPurchaseStatistics final : public tfs::application::Resource
{
public:
	std::map<uint16_t, MarketStatistics> values;
};

class MarketSaleStatistics final : public tfs::application::Resource
{
public:
	std::map<uint16_t, MarketStatistics> values;
};

class ExpiredMarketOffersTimer final : public tfs::application::Resource
{
public:
	std::unique_ptr<tfs::application::Timer> timer;
};

// Legacy bridge for code paths that do not receive an application Context yet.
extern MarketPurchaseStatistics* g_marketPurchaseStatistics;
extern MarketSaleStatistics* g_marketSaleStatistics;

#endif // FS_MODULES_MARKET_RESOURCES_H
