# Player BlockHit / Damage Absorption

Migrate the complete damage absorption, reflection, and skill gain on block to Lua.

## Files
- `src/player.cpp` (lines 1822-1964)

## What to Move

### 1. `blockHit()` (player.cpp:1881-1964)
Complete damage absorption and reflection:
- Per-slot absorb percent application (helmet, armor, legs, boots, shield, ring, necklace, etc.)
- Field absorb (damage type specific)
- Charge consumption on absorb
- Reflection chance/percent per combat type
- Clamp damage to zero
- Each absorb slot iterates through inventory

### 2. Skill Gain on Block (player.cpp:1822-1861)
- `bloodHitCount` / `shieldBlockCount` decrement
- `onBlockHit()`: shield block counter decrement and skill advancement trigger
- `onAttackedCreatureBlockHit()`: BLOCK_NONE -> 30 blood hits, BLOCK_DEFENSE/ARMOR -> decrement counter
- `addAttackSkillPoint` flag

### 3. Player onAttacking() (player.cpp:1433-1492)
Attack cycle:
- `addInFightTicks`
- Pacified check
- Attack speed timing
- Line-of-sight check
- Weapon resolution
- Classic vs action-based speed
- Scheduler re-arming

## Required Bindings
- `Item:getAbilities()` - already exists via `ItemType:getAbilities()`?
- `Player:getInventoryItem(slot)` - already exists
- `Player:getSlotItem(slot)` - already exists
- Special skill access: `PLAYER:getSpecialSkill(SPECIALSKILL_*)` - already exists
- `CombatDamage` type accessors
- `Player:addAttackSkillPoint()` / `resetAttackSkillPoint()`

## Migration Strategy
1. Create `data/scripts/systems/player_damage_absorption.lua`
2. Move `blockHit()` to `onCreatureChangeHealth` event handler
3. Move skill gain to `onAttackedCreatureBlockHit` event (already exists)
4. Move `onAttacking()` to `onCreatureThink` handler
5. Item abilities data already available via ItemType bindings
