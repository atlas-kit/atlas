# Movement: Walkthrough, Stairhop & Zone Transitions

Migrate movement rules to Lua: walkthrough logic, stairhop delay, zone transitions.

## Files
- `src/player.cpp` (lines 675-726, 1112-1124, 1274-1281)
- `src/creature.cpp` (lines 342-489)
- `src/game.cpp` (lines 602-880)

## What to Move

### 1. `canWalkthrough()` (player.cpp:675-710)
Multi-condition walkthrough rule:
- Access player bypass
- Ghost mode
- `ALLOW_WALKTHROUGH` config
- Protection zone check
- Level check (`PROTECTION_LEVEL`)
- 2-second cooldown on walkthrough attempt
- Position tracking

### 2. `canWalkthroughEx()` (player.cpp:712-726)
Simpler version without time/position checks

### 3. Stairhop Delay (player.cpp:1274-1281)
Anti-stairhop mechanic:
- Teleport or floor change applies `CONDITION_PACIFIED` for `STAIRHOP_DELAY`

### 4. Player Zone Change (player.cpp:1112-1124)
`onChangeZone()`:
- Protection zone entry: cancel attack, "Target lost.", update walkthrough, update icons

### 5. Creature Zone Change (creature.cpp:342-371)
- Protection zone: drop target/follow
- Zone event dispatch

### 6. Creature Move (creature.cpp:373-489)
Full creature move:
- Step cost (diagonal=3, floor change=2)
- Summon despawn check (>2 floors or >30 tiles)
- Zone-change events
- Follow/attack zone validation

### 7. Player Move Creature (game.cpp:677-764)
- Pushability check
- Ghost mode interaction
- Zone transition rules
- NPC master position check
- Throw range check
- Already has `onPlayerMoveCreature` event

### 8. Player Move Item (game.cpp:890-1064)
- Pickupable check
- Throw range / line-of-sight
- Hangable item special logic
- Walk-to-item for distant items
- Action cooldown system
- Cross-floor movement restrictions
- Already has `onPlayerMoveItem` event

## Required Bindings
- `Player:canWalkthrough(creature)` / `canWalkthroughEx(creature)` - needs binding
- `Tile:hasFlag(TILESTATE_PROTECTIONZONE)` - already exists
- `Player:hasCondition(CONDITION_PACIFIED)` - already exists
- `Creature:isPushable()` - already exists
- `Creature:isMovementBlocked()` - already exists
- `Item:isHangable()` - needs binding
- `Item:isPickupable()` - already exists
- `Item:getThrowRange()` - needs binding

## Migration Strategy
1. Create `data/scripts/systems/movement_rules.lua`
2. Move walkthrough logic to Lua `onCreatureMove` event
3. Move stairhop to Lua condition handling
4. Move zone transition logic to `onCreatureChangeZone` event (already exists)
5. Move pushability/zone rules to `onPlayerMoveCreature` event (already exists)
