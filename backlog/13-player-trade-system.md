# Player-to-Player Trade System

Migrate the complete trade system to Lua.

## Files
- `src/game.cpp` (lines 2572-2958)

## What to Move

### 1. `playerRequestTrade()` (game.cpp:2572-2688)
Initiates trade:
- Self-trade check
- Range/line-of-sight check
- Throw check
- Item pickupable/uniqueid check
- House item trade restriction
- Floor-level check
- Already-trading item check
- Container holding count limit
- Walk-to-item logic
- Already has `onPlayerTradeRequest` event

### 2. `internalStartTrade()` (game.cpp:2691-2721)
Trade initialization:
- Trade state validation
- Notification messages
- Counter-offer logic

### 3. `playerAcceptTrade()` (game.cpp:2723-2845)
Completes trade:
- Trade state validation
- Post-accept line-of-sight recheck
- Item transfer with rollback
- Backpack-slot conflict resolution
- Capacity/room checking
- Success/failure messaging
- Trade state cleanup
- Already has `onPlayerTradeAccept` and `onPlayerTradeCompleted` events

### 4. `internalCloseTrade()` (game.cpp:2920-2958)
Cancels/closes trade:
- Transfer-in-progress protection
- Cancel notification logic
- `tradeItems` map cleanup

### 5. `playerLookInTrade()` (game.cpp:2864-2911)
Examines trade items:
- Container traversal
- Already has `onPlayerLookInTrade` event

### 6. `getTradeErrorDescription()` (game.cpp:2849-2862)
Human-readable trade error messages.

## Required Bindings
- `Player:getTradeState()` / `setTradeState()`
- `Player:getTradeItem()` / `setTradeItem()`
- `Player:getTradePartner()` / `setTradePartner()`
- `Player:sendTradeItemRequest(player, item)`
- These are mostly Player-side, already partially exposed

## Migration Strategy
1. Create `data/scripts/systems/player_trade.lua`
2. Move trade validation to Lua event handlers
3. Move trade execution to Lua using existing item movement bindings
4. Remove C++ trade validation, keep only engine-level item transfer
