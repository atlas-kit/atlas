# Equipment Slot Rules & Weapon Usage

Migrate equipment slot constraints, weapon eligibility, ammo system, and quiver logic to Lua.

## Files
- `src/player.cpp` (lines 178-283, 299-330, 2344-2485, 2636-2658, 2913-2985)
- `src/weapons.cpp` (lines 127-220, 234-262, 306-429, 464-494, 573-801)

## What to Move

### 1. Equipment Slot `queryAdd()` (player.cpp:2344-2485)
~140 lines of equipment constraint rules:
- Two-hand weapon check
- Shield-only slot
- Weapon-only slot
- Quiver + distance weapon compatibility
- Ammo slot restrictions
- Classic equipment slots mode
- Store item restrictions

### 2. Weapon Resolution (player.cpp:178-283)
- `getWeapon()`: left/right hand search, ammo slot, quiver content, ammo type matching
- `getWeaponType()`: maps equipped item to weapon type
- `getWeaponSkill()`: maps weapon type to skill (sword/club/axe/distance)

### 3. Weapon Usage Checks (weapons.cpp:127-220)
- `playerWeaponCheck()`: range, mana, health, soul, premium, vocation, level/magic level, unproper wield damage halving
- `ammoCheck()`: ammo-specific validation

### 4. Post-Use Weapon Effects (weapons.cpp:306-429)
- Skill gain on weapon use
- Mana/health/soul costs
- Break chance
- Charge/count consumption
- `addVocationWeaponSet()`: vocation-to-weapon permissions

### 5. Distance Hit Chance (weapons.cpp:573-705)
~130 lines of distance-based accuracy formulas:
- One-handed: 75% max
- Two-handed: 90% max
- 100% max hit chance weapons
- Skill clamping
- Per-distance formula
- Miss behavior: scatter to adjacent tiles

### 6. Skill Type Mapping (weapons.cpp:464-494, 775-801)
- Melee: weapon type -> SKILL_SWORD/CLUB/AXE
- Distance: block type -> skill points (2 on hit, 1 on block)

## Required Bindings
- `Item:getWeaponType()` - already exists
- `ItemType:getSlotPosition()` - already exists
- `Player:getSlotItem(slot)` - already exists
- `Weapon:getRequiredLevel()` / `getRequiredMagicLevel()` - already exists via ItemType
- `CombatDamage:setHitChance(chance)` - needs binding or formula exposed

## Migration Strategy
1. Create `data/scripts/systems/equipment_rules.lua`
2. Move `queryAdd()` logic to `onPlayerInventoryUpdate` event
3. Move weapon resolution to Lua helper functions
4. Move hit chance formula to Lua, with per-weapon customization
5. Keep weapon execution (item removal, charges) in C++ for now
