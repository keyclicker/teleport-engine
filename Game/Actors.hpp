#pragma once

// The demo's entities: everything that fights, flies or lies around

#include <random>

#include "../Engine/World/World.hpp"
#include "Assets.hpp"

/**
 * @brief Entity with health: the player and monsters
 */
class Actor : public te::Entity {
public:
  int health = 100;

  bool isDead() const { return health <= 0; }

  /**
   * @brief Take damage; dying is up to the subclass
   *
   * @param source Who did it, may be nullptr
   */
  virtual void damage(te::World &world, int amount, te::Entity *source) {
    (void) world;
    (void) source;
    health -= amount;
  }
};

/**
 * @brief Short animation that removes itself: explosions, bullet puffs
 */
class Effect : public te::Entity {
private:
  const Assets &assets;
  float age = 0;

public:
  static constexpr float DURATION = 0.3f;

  Effect(const Assets &assets, float scale);

  void update(te::World &world, float dt) override;
};

/**
 * @brief Spawn an Effect centered at a point
 *
 * @param scale World units per sprite texel
 */
void spawnPuff(te::World &world, const Assets &assets, te::Sector *sector,
               te::Vec2 pos, float z, float scale);

/**
 * @brief Monster fireball: flies straight, hurts what it touches
 */
class Fireball : public te::Entity {
private:
  const Assets &assets;
  float age = 0;

public:
  int damageAmount = 12;

  /**
   * @param shooter Not hit by its own fireball
   */
  Fireball(const Assets &assets, const te::Entity *shooter);

  void update(te::World &world, float dt) override;
  void onBlocked(te::World &world, const te::MoveResult &result) override;
};

/**
 * @brief Horned demon: wakes on sight, chases, throws fireballs, claws
 */
class Monster : public Actor {
private:
  const Assets &assets;
  Actor &target;

  std::mt19937 rng;

  enum class State { Idle, Chase, Windup, Pain, Dead };
  State state = State::Idle;

  // Time left in the current state, and until the next shot
  float stateTime = 0;
  float attackCooldown = 1;

  // Walk direction while it can't see the target
  te::Vec2 roam;
  float roamTime = 0;

  float sightTime = 0;
  bool seesTarget = false;
  float animTime = 0;

  float random(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng);
  }

  /**
   * @brief Whether a ray toward the target reaches it
   */
  bool canSee(te::World &world) const;

  void throwFireball(te::World &world);

public:
  static constexpr float SPEED = 130;

  Monster(const Assets &assets, Actor &target, unsigned seed);

  void update(te::World &world, float dt) override;
  void onBlocked(te::World &world, const te::MoveResult &result) override;
  void damage(te::World &world, int amount, te::Entity *source) override;
};

/**
 * @brief Item the player walks over
 */
class Pickup : public te::Entity {
public:
  enum class Kind { Medkit, Ammo, Shotgun };

  Kind kind;

  Pickup(const Assets &assets, Kind kind);
};
