#define BOOST_TEST_MODULE market_module

#include "../../../otpch.h"

#include "../../../application/app.h"
#include "../../../modules/market/module.h"
#include "../../../tools.h"

#include <boost/test/unit_test.hpp>

using namespace tfs::modules::market;

BOOST_AUTO_TEST_CASE(market_module_registers_resources_and_legacy_statistics_pointers)
{
	tfs::application::App app;
	app.add_module<MarketModule>();
	auto cleanup = tfs::scope_exit([] {
		g_marketPurchaseStatistics = nullptr;
		g_marketSaleStatistics = nullptr;
	});

	auto purchaseStatistics = app.read_resource<MarketPurchaseStatistics>();
	auto saleStatistics = app.read_resource<MarketSaleStatistics>();
	auto expiredOffersTimer = app.read_resource<ExpiredMarketOffersTimer>();

	BOOST_TEST(g_marketPurchaseStatistics == purchaseStatistics.get());
	BOOST_TEST(g_marketSaleStatistics == saleStatistics.get());
	BOOST_TEST(purchaseStatistics->values.empty());
	BOOST_TEST(saleStatistics->values.empty());
	BOOST_TEST(expiredOffersTimer->timer == nullptr);
}

BOOST_AUTO_TEST_CASE(market_statistics_lookup_returns_entries_and_missing_values)
{
	MarketPurchaseStatistics purchaseStatistics;
	MarketSaleStatistics saleStatistics;

	BOOST_TEST(getPurchaseStatistics(purchaseStatistics, 100) == nullptr);
	BOOST_TEST(getSaleStatistics(saleStatistics, 100) == nullptr);

	purchaseStatistics.values[100] = MarketStatistics{3, 30, 90, 10};
	saleStatistics.values[200] = MarketStatistics{4, 40, 120, 20};

	auto* purchase = getPurchaseStatistics(purchaseStatistics, 100);
	auto* sale = getSaleStatistics(saleStatistics, 200);

	BOOST_REQUIRE(purchase != nullptr);
	BOOST_REQUIRE(sale != nullptr);
	BOOST_TEST(purchase->numTransactions == 3);
	BOOST_TEST(purchase->lowestPrice == 10);
	BOOST_TEST(purchase->totalPrice == 90);
	BOOST_TEST(purchase->highestPrice == 30);
	BOOST_TEST(sale->numTransactions == 4);
	BOOST_TEST(sale->lowestPrice == 20);
	BOOST_TEST(sale->totalPrice == 120);
	BOOST_TEST(sale->highestPrice == 40);
}
