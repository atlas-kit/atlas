// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_MODULES_MARKET_MODULE_H
#define FS_MODULES_MARKET_MODULE_H

#include "../../application/module.h"
#include "resources.h"

#include <ctime>

namespace tfs::modules::market {

class MarketModule final : public application::Module
{
public:
	void build(application::App& app) override;
};

MarketOfferList getActiveOffers(MarketAction_t action, uint16_t itemId);
MarketOfferList getOwnOffers(MarketAction_t action, uint32_t playerId);
HistoryMarketOfferList getOwnHistory(MarketAction_t action, uint32_t playerId);

uint32_t getPlayerOfferCount(uint32_t playerId);
MarketOfferEx getOfferByCounter(uint32_t timestamp, uint16_t counter);

void createOffer(uint32_t playerId, MarketAction_t action, uint32_t itemId, uint16_t amount, uint64_t price,
                 bool anonymous);
void acceptOffer(uint32_t offerId, uint16_t amount);
void deleteOffer(uint32_t offerId);

void appendHistory(uint32_t playerId, MarketAction_t type, uint16_t itemId, uint16_t amount, uint64_t price,
                   time_t timestamp, MarketOfferState_t state);
bool moveOfferToHistory(uint32_t offerId, MarketOfferState_t state);

MarketStatistics* getPurchaseStatistics(MarketPurchaseStatistics& purchaseStatistics, uint16_t itemId);
MarketStatistics* getSaleStatistics(MarketSaleStatistics& saleStatistics, uint16_t itemId);

} // namespace tfs::modules::market

#endif // FS_MODULES_MARKET_MODULE_H
