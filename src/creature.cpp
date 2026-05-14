// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "creature.h"

#include "combat.h"
#include "configmanager.h"
#include "events.h"
#include "game.h"
#include "party.h"
#include "scheduler.h"

double Creature::speedA = 857.36;
double Creature::speedB = 261.29;
double Creature::speedC = -4795.01;

extern Dispatcher g_dispatcher;
extern Game g_game;
extern Scheduler g_scheduler;

Creature::Creature() { onIdleStatus(); }

Creature::~Creature()
{
	for (const auto& summon : summons | tfs::views::lock_weak_ptrs) {
		summon->setAttackedCreature(nullptr);
		summon->removeMaster();
	}
}

bool Creature::canSee(const Position& myPos, const Position& pos, int32_t viewRangeX, int32_t viewRangeY)
{
	if (myPos.z <= 7) {
		// we are on ground level or above (7 -> 0)
		// view is from 7 -> 0
		if (pos.z > 7) {
			return false;
		}
	} else if (myPos.z >= 8) {
		// we are underground (8 -> 15)
		// we can't see floors above 8
		if (pos.z < 8) {
			return false;
		}

		// view is +/- 2 from the floor we stand on
		if (myPos.getDistanceZ(pos) > 2) {
			return false;
		}
	}

	int32_t offsetz = myPos.getOffsetZ(pos);
	return (pos.getX() >= myPos.getX() - viewRangeX + offsetz) && (pos.getX() <= myPos.getX() + viewRangeX + offsetz) &&
	       (pos.getY() >= myPos.getY() - viewRangeY + offsetz) && (pos.getY() <= myPos.getY() + viewRangeY + offsetz);
}

bool Creature::canSee(const Position& pos) const
{
	return canSee(getPosition(), pos, Map::maxViewportX, Map::maxViewportY);
}

bool Creature::canSeeCreature(const std::shared_ptr<const Creature>& creature) const
{
	if (!canSeeGhostMode(creature) && creature->isInGhostMode()) {
		return false;
	}

	if (!canSeeInvisibility() && creature->isInvisible()) {
		return false;
	}
	return true;
}

std::chrono::milliseconds Creature::getTimeSinceLastMove() const
{
	if (lastStep != std::chrono::steady_clock::time_point{}) {
		return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastStep);
	}
	return std::numeric_limits<std::chrono::milliseconds>::max();
}

std::chrono::milliseconds Creature::getWalkDelay(Direction dir) const
{
	if (lastStep == std::chrono::steady_clock::time_point{}) {
		return std::chrono::milliseconds::zero();
	}

	auto ct = std::chrono::steady_clock::now();
	auto stepDuration = getStepDuration(dir);
	return duration_cast<std::chrono::milliseconds>(stepDuration - (ct - lastStep));
}

std::chrono::milliseconds Creature::getWalkDelay() const
{
	// Used for auto-walking
	if (lastStep == std::chrono::steady_clock::time_point{}) {
		return std::chrono::milliseconds::zero();
	}

	auto ct = std::chrono::steady_clock::now();
	auto stepDuration = getStepDuration() * lastStepCost;
	return duration_cast<std::chrono::milliseconds>(stepDuration - (ct - lastStep));
}

void Creature::onThink(std::chrono::milliseconds interval)
{
	if (const auto& followCreature = getFollowCreature();
	    followCreature && !tfs::owner_equal(master, followCreature) && !canSeeCreature(followCreature)) {
		setFollowCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->sendCancelTarget();
			player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
		}
	}

	if (const auto& attackedCreature = getAttackedCreature();
	    attackedCreature && !tfs::owner_equal(master, attackedCreature) && !canSeeCreature(attackedCreature)) {
		setAttackedCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->sendCancelTarget();
			player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
		}
	}

	blockTicks += interval;
	if (blockTicks >= 1s) {
		blockCount = std::min<uint32_t>(blockCount + 1, 2);
		blockTicks = std::chrono::milliseconds::zero();
	}

	tfs::events::creature::onThink(asCreature(), interval);
}

void Creature::forceUpdatePath()
{
	if (attackedCreature.expired() && followCreature.expired()) {
		return;
	}

	lastPathUpdate =
	    std::chrono::steady_clock::now() + std::chrono::milliseconds{getNumber(ConfigManager::PATHFINDING_DELAY)};
	g_dispatcher.addTask(createTask([id = getID()]() { g_game.updateCreatureWalk(id); }));
}

void Creature::onIdleStatus()
{
	if (!isDead()) {
		damageMap.clear();
		lastAttacker.reset();
	}
}

void Creature::onWalk()
{
	if (getWalkDelay() <= std::chrono::milliseconds::zero()) {
		Direction dir;
		uint32_t flags = FLAG_IGNOREFIELDDAMAGE;
		if (getNextStep(dir, flags)) {
			ReturnValue ret = g_game.internalMoveCreature(asCreature(), dir, flags);
			if (ret != RETURNVALUE_NOERROR) {
				if (const auto& player = asPlayer()) {
					player->sendCancelMessage(ret);
					player->sendCancelWalk();
				}
			}
		} else {
			stopEventWalk();

			if (listWalkDir.empty()) {
				onWalkComplete();
			}
		}
	}

	updateFollowersPaths();

	if (cancelNextWalk) {
		listWalkDir.clear();
		onWalkAborted();
		cancelNextWalk = false;
	}

	if (eventWalk != 0) {
		eventWalk = 0;
		addEventWalk();
	}

	if (!attackedCreature.expired() || !followCreature.expired()) {
		if (lastPathUpdate < std::chrono::steady_clock::now()) {
			g_dispatcher.addTask(createTask([id = getID()]() { g_game.updateCreatureWalk(id); }));
			lastPathUpdate = std::chrono::steady_clock::now() +
			                 std::chrono::milliseconds{getNumber(ConfigManager::PATHFINDING_DELAY)};
		}
	}
}

void Creature::onWalk(Direction& dir)
{
	if (!hasCondition(CONDITION_DRUNK)) {
		return;
	}

	uint16_t rand = uniform_random(0, 399);
	if (rand / 4 > getDrunkenness()) {
		return;
	}

	dir = static_cast<Direction>(rand % 4);
	g_game.internalCreatureSay(asCreature(), TALKTYPE_MONSTER_SAY, "Hicks!", false);
}

bool Creature::getNextStep(Direction& dir, uint32_t&)
{
	if (listWalkDir.empty()) {
		return false;
	}

	dir = listWalkDir.back();
	listWalkDir.pop_back();
	onWalk(dir);
	return true;
}

void Creature::startAutoWalk()
{
	if (const auto& player = asPlayer(); player && player->isMovementBlocked()) {
		player->sendCancelWalk();
		return;
	}

	addEventWalk(listWalkDir.size() == 1);
}

void Creature::startAutoWalk(Direction direction)
{
	if (const auto& player = asPlayer(); player && player->isMovementBlocked()) {
		player->sendCancelWalk();
		return;
	}

	listWalkDir = {direction};
	addEventWalk(true);
}

void Creature::startAutoWalk(const std::vector<Direction>& listDir)
{
	if (hasCondition(CONDITION_ROOT)) {
		return;
	}

	if (const auto& player = asPlayer(); player && player->isMovementBlocked()) {
		player->sendCancelWalk();
		return;
	}

	listWalkDir = listDir;
	addEventWalk(listWalkDir.size() == 1);
}

void Creature::addEventWalk(bool firstStep)
{
	cancelNextWalk = false;

	if (getStepSpeed() <= 0) {
		return;
	}

	if (eventWalk != 0) {
		return;
	}

	auto ticks = getEventStepTicks(firstStep);
	if (ticks <= std::chrono::milliseconds::zero()) {
		return;
	}

	// Take first step right away, but still queue the next
	if (ticks == 1ms) {
		g_game.checkCreatureWalk(getID());
	}

	eventWalk = g_scheduler.addEvent(createSchedulerTask(ticks, [id = getID()]() { g_game.checkCreatureWalk(id); }));
}

void Creature::stopEventWalk()
{
	if (eventWalk != 0) {
		g_scheduler.stopEvent(eventWalk);
		eventWalk = 0;
	}
}

void Creature::updateIcons() const
{
	SpectatorVec spectators;
	g_game.map.getSpectators(spectators, position, true, true);
	for (const auto& spectator : spectators) {
		assert(spectator->asPlayer() != nullptr);
		std::static_pointer_cast<Player>(spectator)->sendUpdateCreatureIcons(asCreature());
	}
}

void Creature::onRemoveCreature(const std::shared_ptr<Creature>& creature, bool)
{
	if (const auto& attackedCreature = getAttackedCreature(); creature == attackedCreature) {
		setAttackedCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->sendCancelTarget();
		}

		if (const auto& monster = asMonster()) {
			monster->resetAttackTicks();
		}
	}

	if (const auto& followCreature = getFollowCreature(); creature == followCreature) {
		setFollowCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->sendCancelTarget();
		}
	}
}

void Creature::updateFollowCreaturePath(FindPathParams& fpp)
{
	listWalkDir.clear();

	if (const auto& followCreature = getFollowCreature(); getPathTo(followCreature->getPosition(), listWalkDir, fpp)) {
		hasFollowPath = true;
		startAutoWalk();
	} else {
		hasFollowPath = false;
	}
}

void Creature::onChangeZone(ZoneType_t zone)
{
	if (zone == ZONE_PROTECTION) {
		if (const auto& attackedCreature = getAttackedCreature()) {
			setAttackedCreature(nullptr);

			if (const auto& player = asPlayer()) {
				player->sendCancelTarget();
				player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
			}

			if (const auto& monster = asMonster()) {
				monster->resetAttackTicks();
			}
		}

		if (const auto& followCreature = getFollowCreature()) {
			setFollowCreature(nullptr);

			if (const auto& player = asPlayer()) {
				player->sendCancelTarget();
				player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
			}

			if (const auto& monster = asMonster()) {
				monster->resetAttackTicks();
			}
		}
	}
}

void Creature::onCreatureMove(const std::shared_ptr<Creature>& creature, const std::shared_ptr<const Tile>& newTile,
                              const Position& newPos, const std::shared_ptr<const Tile>& oldTile,
                              const Position& oldPos, bool teleport)
{
	if (creature.get() == this) {
		lastStep = std::chrono::steady_clock::now();
		lastStepCost = 1;

		if (!teleport) {
			if (oldPos.z != newPos.z) {
				// floor change extra cost
				lastStepCost = 2;
			} else if (newPos.getDistanceX(oldPos) >= 1 && newPos.getDistanceY(oldPos) >= 1) {
				// diagonal extra cost
				lastStepCost = 3;
			}
		} else {
			stopEventWalk();
		}

		if (!summons.empty()) {
			// check if any of our summons is out of range (+/- 2 floors or 30 tiles away)
			auto despawnList = summons | tfs::views::lock_weak_ptrs | std::views::filter([&newPos](const auto& summon) {
				                   const Position& pos = summon->getPosition();
				                   return newPos.getDistanceZ(pos) > 2 ||
				                          std::max(newPos.getDistanceX(pos), newPos.getDistanceY(pos)) > 30;
			                   }) |
			                   std::ranges::to<std::vector>();

			for (const auto& despawnCreature : despawnList) {
				g_game.removeCreature(despawnCreature, true);
			}
		}

		if (newTile->getZone() != oldTile->getZone()) {
			tfs::events::creature::onChangeZone(asCreature(), oldTile->getZone(), newTile->getZone());
			onChangeZone(getZone());
		}
	}

	if (const auto& followCreature = getFollowCreature();
	    creature == followCreature || (creature.get() == this && followCreature)) {
		if (newPos.z != oldPos.z || !canSee(followCreature->getPosition())) {
			setFollowCreature(nullptr);

			if (const auto& player = asPlayer()) {
				player->sendCancelTarget();
				player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
			}

			if (const auto& monster = asMonster()) {
				monster->resetAttackTicks();
			}
		}
	}

	if (const auto& attackedCreature = getAttackedCreature();
	    creature == attackedCreature || (creature.get() == this && attackedCreature)) {
		if (newPos.z != oldPos.z || !canSee(attackedCreature->getPosition())) {
			setAttackedCreature(nullptr);

			if (const auto& player = asPlayer()) {
				player->sendCancelTarget();
				player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
			}

			if (const auto& monster = asMonster()) {
				monster->resetAttackTicks();
			}
		} else {
			if (hasExtraSwing()) {
				// our target is moving lets see if we can get in hit
				g_dispatcher.addTask([id = getID()]() { g_game.checkCreatureAttack(id); });
			}

			if (newTile->getZone() != oldTile->getZone()) {
				const auto zone = attackedCreature->getZone();

				if (const auto& player = asPlayer()) {
					if (zone == ZONE_PROTECTION) {
						if (!player->hasFlag(PlayerFlag_IgnoreProtectionZone)) {
							player->setAttackedCreature(nullptr);
							player->sendCancelTarget();
							player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
						}
					} else if (zone == ZONE_NOPVP) {
						if (attackedCreature->asPlayer()) {
							if (!player->hasFlag(PlayerFlag_IgnoreProtectionZone)) {
								player->setAttackedCreature(nullptr);
								player->sendCancelTarget();
								player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
							}
						}
					} else if (zone == ZONE_NORMAL) {
						// attackedCreature can leave a pvp zone if not pzlocked
						if (g_game.getWorldType() == WORLD_TYPE_NO_PVP) {
							if (attackedCreature->asPlayer()) {
								player->setAttackedCreature(nullptr);
								player->sendCancelTarget();
								player->sendTextMessage(MESSAGE_STATUS_SMALL, "Target lost.");
							}
						}
					}
				} else {
					if (zone == ZONE_PROTECTION) {
						setAttackedCreature(nullptr);

						if (const auto& monster = asMonster()) {
							monster->resetAttackTicks();
						}
					}
				}
			}
		}
	}
}

void Creature::onDeath()
{
	bool lastHitUnjustified = false;
	bool mostDamageUnjustified = false;
	auto lastHitCreature = lastAttacker.lock();

	std::shared_ptr<Creature> lastHitCreatureMaster = nullptr;
	if (lastHitCreature) {
		lastHitUnjustified = lastHitCreature->onKilledCreature(asCreature());
		lastHitCreatureMaster = lastHitCreature->getMaster();
	}

	std::shared_ptr<Creature> mostDamageCreature = nullptr;

	const auto timeNow = std::chrono::steady_clock::now();
	const auto inFightTicks = std::chrono::milliseconds{getNumber(ConfigManager::PZ_LOCKED)};
	int32_t mostDamage = 0;
	std::map<std::shared_ptr<Creature>, uint64_t> experienceMap;
	for (const auto& [id, cb] : damageMap | std::views::as_const) {
		if (auto attacker = g_game.getCreatureByID(id)) {
			if ((cb.total > mostDamage && (timeNow - cb.ticks <= inFightTicks))) {
				mostDamage = cb.total;
				mostDamageCreature = attacker;
			}

			if (attacker.get() != this) {
				uint64_t gainExp = getGainedExperience(attacker);
				if (const auto& attackerPlayer = attacker->asPlayer()) {
					attackerPlayer->removeAttacked(asPlayer());

					if (const auto& party = attackerPlayer->getParty()) {
						if (party->isSharedExperienceActive() && party->isSharedExperienceEnabled()) {
							if (const auto& leader = party->getLeader()) {
								attacker = leader;
							}
						}
					}
				}

				auto tmpIt = experienceMap.find(attacker);
				if (tmpIt == experienceMap.end()) {
					experienceMap[attacker] = gainExp;
				} else {
					tmpIt->second += gainExp;
				}
			}
		}
	}

	for (auto&& [attacker, gainExp] : experienceMap | std::views::as_const) {
		attacker->onGainExperience(gainExp, asCreature());
	}

	if (mostDamageCreature) {
		if (mostDamageCreature != lastHitCreature && mostDamageCreature != lastHitCreatureMaster) {
			const auto& mostDamageCreatureMaster = mostDamageCreature->getMaster();
			if (lastHitCreature != mostDamageCreatureMaster &&
			    (!lastHitCreatureMaster || mostDamageCreatureMaster != lastHitCreatureMaster)) {
				mostDamageUnjustified = mostDamageCreature->onKilledCreature(asCreature(), false);
			}
		}
	}

	bool droppedCorpse = dropCorpse(lastHitCreature, mostDamageCreature, lastHitUnjustified, mostDamageUnjustified);
	death(lastHitCreature);

	if (!master.expired()) {
		setMaster(nullptr);
	}

	if (droppedCorpse) {
		g_game.removeCreature(asCreature(), false);
	} else {
		while (!conditions.empty()) {
			removeCondition(conditions.back().get(), true);
		}
	}
}

bool Creature::dropCorpse(const std::shared_ptr<Creature>& lastHitCreature,
                          const std::shared_ptr<Creature>& mostDamageCreature, bool lastHitUnjustified,
                          bool mostDamageUnjustified)
{
	if (!lootDrop && asMonster()) {
		if (!master.expired()) {
			tfs::events::creature::onDeath(asCreature(), nullptr, lastHitCreature, mostDamageCreature,
			                               lastHitUnjustified, mostDamageUnjustified);
		}

		g_game.addMagicEffect(getPosition(), CONST_ME_POFF);
	} else {
		std::shared_ptr<Item> splash = nullptr;
		switch (getRace()) {
			case RACE_VENOM:
				splash = Item::CreateItem(ITEM_FULLSPLASH, FLUID_SLIME);
				break;

			case RACE_BLOOD:
				splash = Item::CreateItem(ITEM_FULLSPLASH, FLUID_BLOOD);
				break;

			case RACE_INK:
				splash = Item::CreateItem(ITEM_FULLSPLASH, FLUID_INK);
				break;

			default:
				splash = nullptr;
				break;
		}

		const auto& tile = getTile();

		if (splash) {
			g_game.internalAddItem(tile, splash, INDEX_WHEREEVER, FLAG_NOLIMIT);
			g_game.startDecay(splash);
		}

		const auto& corpse = getCorpse(lastHitCreature, mostDamageCreature);
		if (corpse) {
			g_game.internalAddItem(tile, corpse, INDEX_WHEREEVER, FLAG_NOLIMIT);
			g_game.startDecay(corpse);
		}

		tfs::events::creature::onDeath(asCreature(), corpse, lastHitCreature, mostDamageCreature, lastHitUnjustified,
		                               mostDamageUnjustified);

		if (corpse) {
			dropLoot(corpse->asContainer(), lastHitCreature);
		}
	}

	return true;
}

bool Creature::hasBeenAttacked(uint32_t attackerId)
{
	auto it = damageMap.find(attackerId);
	if (it == damageMap.end()) {
		return false;
	}
	return (std::chrono::steady_clock::now() - it->second.ticks) <=
	       std::chrono::milliseconds{getNumber(ConfigManager::PZ_LOCKED)};
}

std::shared_ptr<Item> Creature::getCorpse(const std::shared_ptr<Creature>&, const std::shared_ptr<Creature>&)
{
	return Item::CreateItem(getLookCorpse());
}

void Creature::changeHealth(int32_t healthChange, bool sendHealthChange /* = true*/)
{
	int32_t oldHealth = health;

	if (healthChange > 0) {
		health += std::min<int32_t>(healthChange, getMaxHealth() - health);
	} else {
		health = std::max<int32_t>(0, health + healthChange);
	}

	if (sendHealthChange && oldHealth != health) {
		g_game.addCreatureHealth(asCreature());
	}

	if (isDead()) {
		g_dispatcher.addTask([id = getID()]() { g_game.executeDeath(id); });
	}
}

void Creature::gainHealth(const std::shared_ptr<Creature>& healer, int32_t healthGain)
{
	changeHealth(healthGain);
	if (healer) {
		healer->onTargetCreatureGainHealth(asCreature(), healthGain);
	}
}

void Creature::drainHealth(const std::shared_ptr<Creature>& attacker, int32_t damage)
{
	changeHealth(-damage, false);

	if (attacker) {
		attacker->onAttackedCreatureDrainHealth(asCreature(), damage);
	} else {
		lastAttacker.reset();
	}
}

BlockType_t Creature::blockHit(const std::shared_ptr<Creature>& attacker, CombatType_t combatType, int32_t& damage,
                               bool checkDefense /* = false */, bool checkArmor /* = false */, bool /* field = false */,
                               bool /* ignoreResistances = false */)
{
	BlockType_t blockType = BLOCK_NONE;

	if (isImmune(combatType)) {
		damage = 0;
		blockType = BLOCK_IMMUNITY;
	} else if (combatType != COMBAT_HEALING && (checkDefense || checkArmor)) {
		bool hasDefense = false;

		if (blockCount > 0) {
			--blockCount;
			hasDefense = true;
		}

		if (checkDefense && hasDefense && canUseDefense) {
			int32_t defense = getDefense();
			damage -= uniform_random(defense / 2, defense);
			if (damage <= 0) {
				damage = 0;
				blockType = BLOCK_DEFENSE;
				checkArmor = false;
			}
		}

		if (checkArmor) {
			int32_t armor = getArmor();
			if (armor > 3) {
				damage -= uniform_random(armor / 2, armor - (armor % 2 + 1));
			} else if (armor > 0) {
				--damage;
			}

			if (damage <= 0) {
				damage = 0;
				blockType = BLOCK_ARMOR;
			}
		}

		if (hasDefense && blockType != BLOCK_NONE) {
			onBlockHit();
		}
	}

	if (attacker) {
		if (const auto& attackerPlayer = attacker->asPlayer()) {
			for (int32_t slot = CONST_SLOT_FIRST; slot <= CONST_SLOT_LAST; ++slot) {
				if (!attackerPlayer->isItemAbilityEnabled(static_cast<slots_t>(slot))) {
					continue;
				}

				const auto& item = attackerPlayer->getInventoryItem(static_cast<slots_t>(slot));
				if (!item) {
					continue;
				}

				const uint16_t boostPercent = item->getBoostPercent(combatType);
				if (boostPercent != 0) {
					damage += std::round(damage * (boostPercent / 100.));
				}
			}
		}

		if (damage <= 0) {
			damage = 0;
			blockType = BLOCK_ARMOR;
		}

		if (combatType != COMBAT_HEALING) {
			attacker->onAttackedCreature(asCreature());
			attacker->onAttackedCreatureBlockHit(blockType);
			if (const auto& master = attacker->getMaster()) {
				if (const auto& masterPlayer = master->asPlayer()) {
					masterPlayer->onAttackedCreature(asCreature());
				}
			}
		}
	}

	if (combatType != COMBAT_HEALING) {
		if (const auto& player = asPlayer()) {
			player->addInFightTicks();
		}
	}
	return blockType;
}

void Creature::setAttackedCreature(const std::shared_ptr<Creature>& creature)
{
	if (!creature) {
		attackedCreature.reset();

		for (const auto& summon : summons | tfs::views::lock_weak_ptrs) {
			summon->setAttackedCreature(nullptr);
		}

		if (const auto& player = asPlayer()) {
			if (player->getFollowCreature()) {
				player->setFollowCreature(nullptr);
			}
		}

		if (const auto& monster = asMonster()) {
			monster->resetAttackTicks();
		}
		return;
	}

	if (tfs::owner_equal(creature, attackedCreature)) {
		return;
	}

	const auto& creaturePosition = creature->getPosition();
	if (creaturePosition.z != getPosition().z || !canSee(creaturePosition)) {
		setAttackedCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->sendCancelTarget();
		}
		return;
	}

	attackedCreature = creature;

	onAttackedCreature(creature);

	if (const auto& player = creature->asPlayer()) {
		player->addInFightTicks();
	}

	forceUpdatePath();

	for (const auto& summon : summons | tfs::views::lock_weak_ptrs) {
		summon->setAttackedCreature(creature);
	}

	if (const auto& player = asPlayer()) {
		const auto& followCreature = player->getFollowCreature();
		if (player->getChaseMode()) {
			if (followCreature != creature) {
				// chase opponent
				player->setFollowCreature(creature);
			}
		} else if (followCreature) {
			player->setFollowCreature(nullptr);
		}

		g_dispatcher.addTask([id = player->getID()]() { g_game.checkCreatureAttack(id); });
	}
}

void Creature::getPathSearchParams(const std::shared_ptr<const Creature>&, FindPathParams& fpp) const
{
	fpp.fullPathSearch = !hasFollowPath;
	fpp.clearSight = true;
	fpp.maxSearchDist = Map::maxViewportX + Map::maxViewportY;
	fpp.minTargetDist = 1;
	fpp.maxTargetDist = 1;
}

void Creature::setFollowCreature(const std::shared_ptr<Creature>& creature)
{
	if (!creature) {
		followCreature.reset();

		hasFollowPath = false;

		if (const auto& player = asPlayer()) {
			player->stopWalk();
		}
		return;
	}

	if (tfs::owner_equal(followCreature, creature)) {
		return;
	}

	const auto& creaturePosition = creature->getPosition();
	if (creaturePosition.z != getPosition().z || !canSee(creaturePosition)) {
		setFollowCreature(nullptr);

		if (const auto& player = asPlayer()) {
			player->setAttackedCreature(nullptr);
			player->sendCancelTarget();
			player->sendCancelMessage(RETURNVALUE_THEREISNOWAY);
			player->stopWalk();
		}
		return;
	}

	if (const auto& oldFollow = getFollowCreature()) {
		oldFollow->removeFollower(asCreature());
	}
	creature->addFollower(asCreature());

	followCreature = creature;
	hasFollowPath = false;

	if (!listWalkDir.empty()) {
		listWalkDir.clear();
		onWalkAborted();
	}

	forceUpdatePath();
}

// Pathfinding Events
void Creature::updateFollowersPaths()
{
	if (followers.empty()) {
		return;
	}

	followers = followers | tfs::views::lock_weak_ptrs | std::views::filter([this](const auto& creature) {
		            if (position.z != creature->position.z) {
			            return false;
		            }

		            return position.getDistanceX(creature->position) < Map::maxViewportX &&
		                   position.getDistanceY(creature->position) < Map::maxViewportY;
	            }) |
	            std::ranges::to<decltype(followers)>();

	for (const auto& follower : followers | tfs::views::lock_weak_ptrs) {
		if (follower->lastPathUpdate >= std::chrono::steady_clock::now()) {
			continue;
		}

		g_dispatcher.addTask(createTask([id = follower->getID()]() { g_game.updateCreatureWalk(id); }));
		follower->lastPathUpdate =
		    std::chrono::steady_clock::now() + std::chrono::milliseconds{getNumber(ConfigManager::PATHFINDING_DELAY)};
	}
}

double Creature::getDamageRatio(const std::shared_ptr<Creature>& attacker) const
{
	uint32_t totalDamage = 0;
	uint32_t attackerDamage = 0;

	for (auto&& [id, cb] : damageMap | std::views::as_const) {
		totalDamage += cb.total;
		if (id == attacker->getID()) {
			attackerDamage += cb.total;
		}
	}

	if (totalDamage == 0) {
		return 0;
	}

	return (static_cast<double>(attackerDamage) / totalDamage);
}

uint64_t Creature::getGainedExperience(const std::shared_ptr<Creature>& attacker) const
{
	return std::floor(getDamageRatio(attacker) * getLostExperience());
}

void Creature::addDamagePoints(const std::shared_ptr<Creature>& attacker, int32_t damagePoints)
{
	if (damagePoints <= 0) {
		return;
	}

	uint32_t attackerId = attacker->id;

	auto& cb = damageMap[attackerId];
	cb.ticks = std::chrono::steady_clock::now();
	cb.total += damagePoints;

	lastAttacker = attacker;
}

void Creature::onAddCondition(ConditionType_t type)
{
	if (type == CONDITION_PARALYZE && hasCondition(CONDITION_HASTE)) {
		removeCondition(CONDITION_HASTE);
	} else if (type == CONDITION_HASTE && hasCondition(CONDITION_PARALYZE)) {
		removeCondition(CONDITION_PARALYZE);
	}
}

void Creature::onAddCombatCondition(ConditionType_t)
{
	//
}

void Creature::onEndCondition(ConditionType_t)
{
	//
}

void Creature::onTickCondition(ConditionType_t type, bool& bRemove)
{
	const auto& tile = getTile();
	if (!tile) {
		return;
	}

	const auto& field = tile->getFieldItem();
	if (!field) {
		return;
	}

	switch (type) {
		case CONDITION_FIRE:
			bRemove = (field->getCombatType() != COMBAT_FIREDAMAGE);
			break;
		case CONDITION_ENERGY:
			bRemove = (field->getCombatType() != COMBAT_ENERGYDAMAGE);
			break;
		case CONDITION_POISON:
			bRemove = (field->getCombatType() != COMBAT_EARTHDAMAGE);
			break;
		case CONDITION_FREEZING:
			bRemove = (field->getCombatType() != COMBAT_ICEDAMAGE);
			break;
		case CONDITION_DAZZLED:
			bRemove = (field->getCombatType() != COMBAT_HOLYDAMAGE);
			break;
		case CONDITION_CURSED:
			bRemove = (field->getCombatType() != COMBAT_DEATHDAMAGE);
			break;
		case CONDITION_DROWN:
			bRemove = (field->getCombatType() != COMBAT_DROWNDAMAGE);
			break;
		case CONDITION_BLEEDING:
			bRemove = (field->getCombatType() != COMBAT_PHYSICALDAMAGE);
			break;
		default:
			break;
	}
}

void Creature::onCombatRemoveCondition(Condition* condition) { removeCondition(condition); }

void Creature::onAttackedCreatureDrainHealth(const std::shared_ptr<Creature>& target, int32_t points)
{
	target->addDamagePoints(asCreature(), points);
}

bool Creature::onKilledCreature(const std::shared_ptr<Creature>& target, bool)
{
	if (const auto& master = getMaster()) {
		master->onKilledCreature(target);
	}

	tfs::events::creature::onKill(asCreature(), target);
	return false;
}

void Creature::onGainExperience(uint64_t gainExp, const std::shared_ptr<Creature>& target)
{
	const auto& master = getMaster();
	if (gainExp == 0 || !master) {
		return;
	}

	gainExp /= 2;
	master->onGainExperience(gainExp, target);

	SpectatorVec spectators;
	g_game.map.getSpectators(spectators, position, false, true);
	if (spectators.empty()) {
		return;
	}

	TextMessage message(MESSAGE_EXPERIENCE_OTHERS, ucfirst(getNameDescription()) + " gained " +
	                                                   std::to_string(gainExp) +
	                                                   (gainExp != 1 ? " experience points." : " experience point."));
	message.position = position;
	message.primary.color = TEXTCOLOR_WHITE_EXP;
	message.primary.value = gainExp;

	for (const auto& spectator : spectators) {
		assert(spectator->asPlayer() != nullptr);
		std::static_pointer_cast<Player>(spectator)->sendTextMessage(message);
	}
}

bool Creature::setMaster(const std::shared_ptr<Creature>& newMaster)
{
	if (!newMaster && master.expired()) {
		return false;
	}

	if (newMaster) {
		// store a shared_ptr reference in the master's summons list to keep the summon alive
		newMaster->summons.push_back(asCreature());
	}

	const auto oldMaster = getMaster();
	master = newMaster;

	if (oldMaster) {
		oldMaster->removeSummon(asCreature());
	}
	return true;
}

bool Creature::addCondition(std::unique_ptr<Condition> condition, bool force /* = false*/)
{
	if (!condition) {
		return false;
	}

	if (!force && condition->getType() == CONDITION_HASTE && hasCondition(CONDITION_PARALYZE)) {
		auto walkDelay = getWalkDelay();
		if (walkDelay > std::chrono::milliseconds::zero()) {
			g_scheduler.addEvent(
			    createSchedulerTask(walkDelay, [id = getID(), condition = std::move(condition)]() mutable {
				    g_game.forceAddCondition(id, std::move(condition));
			    }));
			return false;
		}
	}

	Condition* prevCond = getCondition(condition->getType(), condition->getId(), condition->getSubId());
	if (prevCond) {
		prevCond->addCondition(asCreature(), condition.get());
		return true;
	}

	if (condition->startCondition(asCreature())) {
		onAddCondition(condition->getType());
		conditions.push_back(std::move(condition));
		return true;
	}

	return false;
}

bool Creature::addCombatCondition(std::unique_ptr<Condition> condition)
{
	ConditionType_t type = condition->getType();

	if (!addCondition(std::move(condition))) {
		return false;
	}

	onAddCombatCondition(type);
	return true;
}

void Creature::removeCondition(ConditionType_t type, bool force /* = false*/)
{
	auto it = conditions.begin();
	while (it != conditions.end()) {
		auto& condition = *it;
		if (condition->getType() != type) {
			++it;
			continue;
		}

		if (!force && type == CONDITION_PARALYZE) {
			auto walkDelay = getWalkDelay();
			if (walkDelay > std::chrono::milliseconds::zero()) {
				g_scheduler.addEvent(
				    createSchedulerTask(walkDelay, [=, id = getID()]() { g_game.forceRemoveCondition(id, type); }));
				return;
			}
		}

		condition->endCondition(asCreature());
		it = conditions.erase(it);

		onEndCondition(type);
	}
}

void Creature::removeCondition(ConditionType_t type, ConditionId_t conditionId, bool force /* = false*/)
{
	auto it = conditions.begin();
	while (it != conditions.end()) {
		auto& condition = *it;
		if (condition->getType() != type || condition->getId() != conditionId) {
			++it;
			continue;
		}

		if (!force && type == CONDITION_PARALYZE) {
			auto walkDelay = getWalkDelay();
			if (walkDelay > std::chrono::milliseconds::zero()) {
				g_scheduler.addEvent(
				    createSchedulerTask(walkDelay, [=, id = getID()]() { g_game.forceRemoveCondition(id, type); }));
				return;
			}
		}

		condition->endCondition(asCreature());
		it = conditions.erase(it);

		onEndCondition(type);
	}
}

void Creature::removeCombatCondition(ConditionType_t type)
{
	std::vector<Condition*> removeConditions;
	for (const auto& condition : conditions) {
		if (condition->getType() == type) {
			removeConditions.push_back(condition.get());
		}
	}

	for (Condition* condition : removeConditions) {
		onCombatRemoveCondition(condition);
	}
}

void Creature::removeCondition(Condition* condition, bool force /* = false*/)
{
	auto it = std::find_if(conditions.begin(), conditions.end(),
	                       [condition](const std::unique_ptr<Condition>& c) { return c.get() == condition; });
	if (it == conditions.end()) {
		return;
	}

	if (!force && condition->getType() == CONDITION_PARALYZE) {
		auto walkDelay = getWalkDelay();
		if (walkDelay > std::chrono::milliseconds::zero()) {
			g_scheduler.addEvent(createSchedulerTask(
			    walkDelay, [id = getID(), type = condition->getType()]() { g_game.forceRemoveCondition(id, type); }));
			return;
		}
	}

	condition->endCondition(asCreature());
	onEndCondition(condition->getType());
	conditions.erase(it);
}

Condition* Creature::getCondition(ConditionType_t type) const
{
	for (const auto& condition : conditions) {
		if (condition->getType() == type) {
			return condition.get();
		}
	}
	return nullptr;
}

Condition* Creature::getCondition(ConditionType_t type, ConditionId_t conditionId, uint32_t subId /* = 0*/) const
{
	for (const auto& condition : conditions) {
		if (condition->getType() == type && condition->getId() == conditionId && condition->getSubId() == subId) {
			return condition.get();
		}
	}
	return nullptr;
}

void Creature::executeConditions(std::chrono::milliseconds interval)
{
	std::vector<Condition*> snapshot;
	snapshot.reserve(conditions.size());
	for (const auto& condition : conditions) {
		snapshot.push_back(condition.get());
	}

	auto findOwning = [this](Condition* c) {
		return std::ranges::find_if(conditions, [c](const std::unique_ptr<Condition>& p) { return p.get() == c; });
	};

	for (Condition* condition : snapshot) {
		if (findOwning(condition) == conditions.end()) {
			continue;
		}

		if (condition->executeCondition(asCreature(), interval)) {
			continue;
		}

		auto it = findOwning(condition);
		if (it == conditions.end()) {
			continue;
		}

		condition->endCondition(asCreature());
		onEndCondition(condition->getType());
		conditions.erase(it);
	}
}

bool Creature::hasCondition(ConditionType_t type, uint32_t subId /* = 0*/) const
{
	if (isSuppress(type)) {
		return false;
	}

	auto timeNow = std::chrono::steady_clock::now();
	for (const auto& condition : conditions) {
		if (condition->getType() != type || condition->getSubId() != subId) {
			continue;
		}

		if (condition->getEndTime() >= timeNow || condition->getTicks() < std::chrono::milliseconds::zero()) {
			return true;
		}
	}
	return false;
}

bool Creature::isImmune(CombatType_t type) const
{
	return hasBitSet(static_cast<uint32_t>(type), getDamageImmunities());
}

bool Creature::isImmune(ConditionType_t type) const
{
	return hasBitSet(static_cast<uint32_t>(type), getConditionImmunities());
}

bool Creature::isSuppress(ConditionType_t type) const
{
	return hasBitSet(static_cast<uint32_t>(type), getConditionSuppressions());
}

std::chrono::milliseconds Creature::getStepDuration(Direction dir) const
{
	auto stepDuration = getStepDuration();
	if ((dir & DIRECTION_DIAGONAL_MASK) != 0) {
		stepDuration *= 3;
	}
	return stepDuration;
}

std::chrono::milliseconds Creature::getStepDuration() const
{
	if (isRemoved()) {
		return std::chrono::milliseconds::zero();
	}

	int32_t stepSpeed = getStepSpeed();
	uint32_t calculatedStepSpeed = 1;
	if (stepSpeed > -Creature::speedB) {
		calculatedStepSpeed =
		    floor((Creature::speedA * log((stepSpeed / 2) + Creature::speedB) + Creature::speedC) + 0.5);
		if (calculatedStepSpeed == 0) {
			calculatedStepSpeed = 1;
		}
	}

	uint32_t groundSpeed = 150;
	if (const auto& tile = getTile()) {
		if (const auto& ground = tile->getGround()) {
			groundSpeed = Item::items[ground->getID()].speed;
			if (groundSpeed == 0) {
				groundSpeed = 150;
			}
		}
	}

	double duration = std::floor(1000 * groundSpeed / calculatedStepSpeed);
	int64_t stepDuration = std::ceil(duration / 50) * 50;

	const auto& monster = this->asMonster();
	if (monster && monster->isTargetNearby() && !monster->isFleeing() && !monster->getMaster()) {
		stepDuration *= 2;
	}

	return std::chrono::milliseconds(stepDuration);
}

std::chrono::milliseconds Creature::getEventStepTicks(bool onlyDelay) const
{
	auto ret = getWalkDelay();
	if (ret <= std::chrono::milliseconds::zero()) {
		auto stepDuration = getStepDuration();
		if (onlyDelay && stepDuration > std::chrono::milliseconds::zero()) {
			ret = 1ms;
		} else {
			ret = stepDuration * lastStepCost;
		}
	}
	return ret;
}

LightInfo Creature::getCreatureLight() const { return internalLight; }

void Creature::setCreatureLight(LightInfo lightInfo) { internalLight = std::move(lightInfo); }

void Creature::setNormalCreatureLight() { internalLight = {}; }

bool FrozenPathingConditionCall::isInRange(const Position& startPos, const Position& testPos,
                                           const FindPathParams& fpp) const
{
	if (fpp.fullPathSearch) {
		if (testPos.x > targetPos.x + fpp.maxTargetDist) {
			return false;
		}

		if (testPos.x < targetPos.x - fpp.maxTargetDist) {
			return false;
		}

		if (testPos.y > targetPos.y + fpp.maxTargetDist) {
			return false;
		}

		if (testPos.y < targetPos.y - fpp.maxTargetDist) {
			return false;
		}
	} else {
		int32_t dx = startPos.getOffsetX(targetPos);

		int32_t dxMax = (dx >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x > targetPos.x + dxMax) {
			return false;
		}

		int32_t dxMin = (dx <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x < targetPos.x - dxMin) {
			return false;
		}

		int32_t dy = startPos.getOffsetY(targetPos);

		int32_t dyMax = (dy >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y > targetPos.y + dyMax) {
			return false;
		}

		int32_t dyMin = (dy <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y < targetPos.y - dyMin) {
			return false;
		}
	}
	return true;
}

bool FrozenPathingConditionCall::operator()(const Position& startPos, const Position& testPos,
                                            const FindPathParams& fpp, int32_t& bestMatchDist) const
{
	if (!isInRange(startPos, testPos, fpp)) {
		return false;
	}

	if (fpp.clearSight && !g_game.isSightClear(testPos, targetPos, true)) {
		return false;
	}

	int32_t testDist = std::max(targetPos.getDistanceX(testPos), targetPos.getDistanceY(testPos));
	if (fpp.maxTargetDist == 1) {
		if (testDist < fpp.minTargetDist || testDist > fpp.maxTargetDist) {
			return false;
		}

		return true;
	} else if (testDist <= fpp.maxTargetDist) {
		if (testDist < fpp.minTargetDist) {
			return false;
		}

		if (testDist == fpp.maxTargetDist) {
			bestMatchDist = 0;
			return true;
		} else if (testDist > bestMatchDist) {
			// not quite what we want, but the best so far
			bestMatchDist = testDist;
			return true;
		}
	}
	return false;
}

bool Creature::isInvisible() const
{
	return std::find_if(conditions.begin(), conditions.end(), [](const std::unique_ptr<Condition>& condition) {
		       return condition->getType() == CONDITION_INVISIBLE;
	       }) != conditions.end();
}

bool Creature::getPathTo(const Position& targetPos, std::vector<Direction>& dirList, const FindPathParams& fpp) const
{
	return g_game.map.getPathMatching(asCreature(), targetPos, dirList, FrozenPathingConditionCall(targetPos), fpp);
}

bool Creature::getPathTo(const Position& targetPos, std::vector<Direction>& dirList, int32_t minTargetDist,
                         int32_t maxTargetDist, bool fullPathSearch /*= true*/, bool clearSight /*= true*/,
                         int32_t maxSearchDist /*= 0*/) const
{
	FindPathParams fpp;
	fpp.fullPathSearch = fullPathSearch;
	fpp.maxSearchDist = maxSearchDist;
	fpp.clearSight = clearSight;
	fpp.minTargetDist = minTargetDist;
	fpp.maxTargetDist = maxTargetDist;
	return getPathTo(targetPos, dirList, fpp);
}

void Creature::setStorageValue(uint32_t key, std::optional<int32_t> value, bool isSpawn)
{
	auto oldValue = getStorageValue(key);
	if (value) {
		storageMap.insert_or_assign(key, value.value());
	} else {
		storageMap.erase(key);
	}
	tfs::events::creature::onUpdateStorage(asCreature(), key, oldValue, value, isSpawn);
}

std::optional<int32_t> Creature::getStorageValue(uint32_t key) const
{
	auto it = storageMap.find(key);
	if (it == storageMap.end()) {
		return std::nullopt;
	}
	return std::make_optional(it->second);
}
