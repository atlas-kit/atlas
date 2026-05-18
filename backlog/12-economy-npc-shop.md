# Economy: NPC Shop System

Migrate NPC shop interaction logic to Lua.

## Files
- `src/game.cpp` (lines 2961-3069)
- `src/player.cpp` (lines 1188-1216, 3146-3186)

## What to Move

### 1. `playerPurchaseItem()` (game.cpp:2961-2997)
Player buys from NPC shop:
- Amount validation
- Fluid type conversion (`clientFluidToServer`)
- Shop item validation
- Delegates to merchant NPC callback

### 2. `playerSellItem()` (game.cpp:2999-3030)
Player sells to NPC shop:
- Same validation patterns as purchase
- NPC callback delegation

### 3. `playerLookInShop()` (game.cpp:3039-3069)
Examines a shop item with fluid conversion

### 4. `playerCloseShop()` / Shop State (player.cpp:1188-1216)
- `openShopWindow()`: sets shop items, sends to client
- `closeShopWindow()`: NPC callback, clears state

### 5. `updateSaleShopList()` (player.cpp:3146-3177)
Checks if item is in shop list with `sellPrice != 0`, recurses containers, sends update.

### 6. `hasShopItemForSale()` (player.cpp:3179-3186)
Checks if NPC buys/sells the item.

## Required Bindings
- `Player:getShopOwner()` - already exists
- `Player:hasShopItemForSale()` - already exists
- `Player:openShopWindow(items)` / `closeShopWindow()` - already exists
- `Npc:onPlayerTrade(item, buy/sell)` - already exists
- `ItemType:getClientId()` - already exists
- `clientFluidToServer(fluid)` / `serverFluidToClient(fluid)` - needs binding

## Migration Strategy
1. Create `data/scripts/systems/npc_shop.lua`
2. Move purchase/sell validation to Lua
3. Move shop list management to Lua
4. Already has `onPlayerLookInShop` event hook
