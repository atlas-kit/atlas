# Economy: Market System

Migrate the complete market system to Lua (create/cancel/accept offers, item listing, fee calculation).

## Files
- `src/game.cpp` (lines 4929-5342)

## What to Move

### 1. `playerCreateMarketOffer()` (game.cpp:4985-5090)
~105 lines - Major candidate. Extensive game rules:
- Amount/price limits
- Market state check
- Premium-only restriction
- Ware item validation
- Stackable amount cap
- Max offer count limit (`getPlayerOfferCount`)
- Fee calculation
- Money balance validation (inventory + bank)
- Item removal from player inventory
- Cash vs bank debit logic
- Offer creation in DB

### 2. `playerCancelMarketOffer()` (game.cpp:5093-5149)
- Offer ownership validation
- Buy offer: refund to bank
- Sell offer: return items to inbox
- Stackable vs non-stackable item return logic
- History recording

### 3. `playerAcceptMarketOffer()` (game.cpp:5153-5339)
~186 lines - Most complex market function:
- Amount validation
- Self-offer prevention
- Buy-side: item removal from player, payment to seller
- Sell-side: payment from player, item delivery
- Offline player loading for buyer/seller
- History recording
- Offer amount tracking and partial acceptance

### 4. `getMarketItemList()` (game.cpp:5342-)
Searches player's inbox and depot chests for items matching a ware ID.

### 5. `playerBrowseMarket()` (game.cpp:4936-4960)
Market browser:
- Auto-enter market
- Item/ware validation
- Fetches buy/sell offers

## Required Bindings
- `Player:getMoney()` - already exists
- `Player:getBankBalance()` / `setBankBalance()` - already exists
- `Player:getInbox()` - already exists
- `Player:getDepotChests()` - already exists
- `Container:getItemList()` - already exists?
- `Player:getShopOwner()` / `hasShopItemForSale()` - already exists
- `Item:hasMarketAttributes()` - needs binding
- `Item:getWorth()` - already exists
- `Item:countByType(itemId, subType)` - needs binding
- `IOMarket` operations exposed to Lua:
  - `IOMarket.getActiveOffers(itemId)`
  - `IOMarket.getPlayerOfferCount(playerGuid)`
  - `IOMarket.createOffer(...)`
  - `IOMarket.deleteOffer(counter)`
  - `IOMarket.moveOfferToHistory(counter)`
  - `IOMarket.appendHistory(...)`
  - `IOMarket.getOfferByCounter(counter)`
- `IOLoginData.loadPlayerById(guid)` - already exposed?
- `IOLoginData.increaseBankBalance(guid, amount)`

## Migration Strategy
1. Create `data/scripts/systems/market_system.lua`
2. Move market offer creation to Lua with game rule callbacks
3. Move offer acceptance to Lua event handler
4. Expose IOMarket functions as Lua bindings
5. Remove C++ market functions, replace with Lua calls
