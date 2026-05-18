# House System

Migrate house logic to Lua: rent, access control, ownership transfer.

## Files
- `src/game.cpp` (lines 5578-5694)
- `src/house.cpp` (lines 23-501)

## What to Move

### 1. Rent System (game.cpp:5607-5694)
`payHouses()`:
- Rent period check
- Ownership check
- Zero-rent / prepaid skip
- Town validation
- Offline player loading
- Bank balance payment logic
- Rent period date calculation
- Payment warning system (letter creation with formatted text)
- House forfeiture after 7 warnings
- Player saving

### 2. Ownership & Transfer (house.cpp:23-93, 301-357)
- `setOwner()`: DB update, item transfer to depot, kick occupants, access list reset, rent period calculation (yearly/monthly/weekly/daily)
- `executeTransfer()`: transfer execution
- `getTransferItem()`: transfer document creation
- `resetTransferItem()`: cancel cleanup
- `HouseTransferItem::onTradeEvent()`: trade event for real estate

### 3. Access Control (house.cpp:95-185, 254-299, 359-501)
- `getHouseAccessLevel()`: account ownership, admin flag, GUID match, subowner, guest - 30-line decision tree
- `canEditAccessList()`: owner full access, subowner only guest list
- `isInvited()`: access check wrapper
- `Door::canUse()`: subowner+ always use, else check list
- `kickPlayer()`: access level comparison, admin immunity, teleport to entry
- `AccessList::parseList()`: guild @ notation, wildcard `*`, player names, line limits
- `setAccessList()`: apply list and kick unauthorized

## Required Bindings
- `House` class already mostly exposed: `getOwner()`, `setOwner()`, `getRent()`, `setRent()`, `getPaidUntil()`, `setPaidUntil()`, `getPayRentWarnings()`, `setPayRentWarnings()`, `canEditAccessList()`, `getAccessList()`, `setAccessList()`, `kickPlayer()`
- `IOLoginData.loadPlayerById(guid)` - needs binding
- `IOLoginData.savePlayer(player)` - needs binding
- `House:getTownId()` / `Map:getTown(id)` - needs binding
- `Item:CreateItem(ITEM_LETTER_STAMPED)` - already exists

## Migration Strategy
1. Create `data/scripts/systems/house_management.lua`
2. Move `payHouses()` to Lua `onGameSave` event
3. Move access control to Lua (rules already well-defined)
4. Move rent period calculation to Lua
5. Keep house data storage in C++, move all business logic to Lua
