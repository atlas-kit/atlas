# Party System

Migrate party game logic to Lua.

## Files
- `src/game.cpp` (lines 4767-4890)
- `src/party.cpp` (lines 20-480)
- `src/player.cpp` (lines 3855-3981, 3465-3521)

## What to Move

### 1. Shared Experience Formulas (party.cpp:402-480)
- `getMemberSharedExperienceStatus()`: level range check (`highestLevel * 2/3`), inactivity check
- `getSharedExperienceStatus()`: aggregate status
- `shareExperience()`: distribute to members
- `updateSharedExperience()`: enable/disable
- Constants: `EXPERIENCE_SHARE_RANGE = 30`, `EXPERIENCE_SHARE_FLOORS = 1`

### 2. Party Lifecycle (party.cpp:20-218)
- `joinParty()`: script check, broadcast, icon updates, shared EXP check
- `leaveParty()`: leader transfer, member removal, broadcast
- `passPartyLeadership()`: leadership transition
- `disband()`: cleanup
- `invitePlayer()` / `revokeInvitation()`

### 3. Party Display Logic (player.cpp:3855-3981)
- `getPartyShield()`: leader/shared-exp/invitation shields
- `isInviting()`: check if player has invited specific player
- `isPartner()`: same party check
- Party invitation list management

### 4. Party Experience Tracking (player.cpp:3465-3521)
- `onIdleStatus()`: clears party damage points on idle
- `onAttackedCreatureDrainHealth()`: party shared-exp tick tracking
- `onTargetCreatureGainHealth()`: party heal-based tick tracking

### 5. Player-Side Party Logic (game.cpp:4767-4890)
- `playerInviteToParty()`: self-invite prevention, auto-creation, leader-only
- `playerJoinParty()`: valid leader + invitation check
- `playerRevokePartyInvitation()`: leader-only
- `playerPassPartyLeadership()`: leader-only, partnership check
- `playerLeaveParty()`: in-fight protection
- `playerEnableSharedPartyExperience()`: in-fight restriction outside PZ

## Required Bindings
- `Party` class already mostly exposed: `getLeader()`, `getMembers()`, `invitePlayer()`, `joinParty()`, `leaveParty()`, `disband()`, `setSharedExperience()`, `shareExperience()`
- `Player:getParty()` - already exists
- `Player:getZone()` - already exists
- `Player:hasCondition(CONDITION_INFIGHT)` - already exists
- `Party:getMemberSharedExperienceStatus()` - needs binding

## Migration Strategy
1. Create `data/scripts/systems/party_logic.lua`
2. Move shared exp formula to Lua (pure math, no engine dependency)
3. Move party lifecycle logic to event handlers (already have `onPartyJoin`, `onPartyLeave`, `onPartyDisband`)
4. Move invite/join/leave validation to Lua
5. Remove C++ party logic, keep Party class as data container + engine operations
