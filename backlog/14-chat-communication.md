# Chat & Communication Rules

Migrate speech validation, muting, channel management, and message routing to Lua.

## Files
- `src/game.cpp` (lines 1862-1997, 3334-3525)
- `src/player.cpp` (lines 1494-1540)
- `src/chat.cpp` (lines 332-584)

## What to Move

### 1. `playerSay()` (game.cpp:3334-3394)
Message routing:
- Idle time reset
- Spell casting intercept
- NPC private chat routing
- Mute check
- Message buffer consumption
- Speech type routing (whisper/yell/private/channel/broadcast)
- Access player admin command detection

### 2. `playerYell()` (game.cpp:3443-3475)
**Perfect candidate** - Distinct game rules:
- Yell exhaustion condition
- Minimum level requirement
- Premium-only yell
- Yell cooldown condition creation
- Uppercase transformation

### 3. `playerWhisper()` (game.cpp:3420-3440)
Spectator filtering with distance-based obfuscation

### 4. `playerSpeakTo()` - Private Message (game.cpp:3477-3521)
- Online check
- Red private message permission
- Minimum level to send private
- Premium requirement
- Ghost mode handling

### 5. `removeMessageBuffer()` - Mute System (player.cpp:1509-1540)
- Message buffer overflow -> muting
- Escalating mute duration: `5s * muteCount^2`
- 1500ms rate limiting

### 6. `playerBroadcastMessage()` (game.cpp:1862-1875)
Permission check: `PlayerFlag_CanBroadcast`

### 7. `playerCreatePrivateChannel()` (game.cpp:1877-1890)
Premium check for channel creation

### 8. Channel Management (chat.cpp:332-584)
- `createChannel()`: guild channel per guild, party channel per party, private channel (premium only, free slot search)
- `getChannelList()`: channel visibility/permissions
- `talkToChannel()`: speak type mapping based on channel and rank

## Required Bindings
- `Player:isMuted()` / `getMuteConditions()` - already exists
- `Player:removeMessageBuffer()` - needs binding
- `Player:hasCondition(CONDITION_YELLTICKS)` - already exists
- `Player:getLevel()` - already exists
- `Player:isPremium()` - already exists
- `Condition:createCondition(type, ticks)` - already exists
- `Player:addCondition(condition)` - already exists
- `Player:getAccountType()` - already exists
- `Player:canSeeGhostMode()` - needs binding
- `Chat:createChannel(name)` / `Chat:getChannelList()` - needs binding

## Migration Strategy
1. Create `data/scripts/systems/chat_rules.lua`
2. Move yell logic to Lua `onPlayerSay` handler
3. Move private message rules to Lua
4. Move mute system to Lua (message buffer tracking)
5. Channel management already has Lua event hooks (`onJoin`, `onLeave`, `onSpeak`)
