#pragma once

#include "Map.hpp"

namespace te {

class World;

/**
 * @brief What stopped a World::move()
 */
struct MoveResult {
  // Pushed back by a wall, a closed portal, a floor or a ceiling
  bool blocked = false;

  // Last blocking entity touched, see Entity::blocking
  Entity *hitEntity = nullptr;

  // Went through at least one portal
  bool teleported = false;
};

/**
 * @brief Anything that lives in a sector: players, monsters, items,
 *        projectiles
 *
 * World moves it by `velocity` and gravity each tick and draws `sprite`
 * at its position. Games derive from it and override update().
 */
class Entity {
public:
  virtual ~Entity() = default;

  // ---- Placement, managed by World -----------------------------------

  Sector *sector = nullptr;
  Vec2 pos;

  // Feet height
  float z = 0;

  // Facing in radians, portals turn it
  float angle = 0;

  // ---- Physics -------------------------------------------------------

  // World units per second; portals turn it
  Vec2 velocity;
  float verticalVelocity = 0;

  float radius = 16;
  float height = 56;

  // Highest floor rise it walks up without jumping
  float stepHeight = 24;

  // Falls and stands on floors; otherwise flies at its height
  bool gravity = true;

  // Standing on the floor, valid after a World tick
  bool onGround = false;

  // Other entities collide with it and traces hit it
  bool blocking = true;

  // An entity this one passes through, e.g. a projectile's shooter
  const Entity *ignored = nullptr;

  // ---- Look ----------------------------------------------------------

  // Billboard that faces the camera, nullptr for invisible entities
  TextureRef sprite;

  // World units per sprite texel
  float spriteScale = 1;

  // ---- Lifetime ------------------------------------------------------

  /**
   * @brief Ask World to delete it at the end of the tick
   */
  void remove() { removed = true; }

  bool isRemoved() const { return removed; }

  /**
   * @brief Unit vector it faces
   */
  Vec2 forward() const { return Vec2::fromAngle(angle); }

  /**
   * @brief Game logic, called each tick before World moves it
   */
  virtual void update(World &world, float dt) {
    (void) world;
    (void) dt;
  }

  /**
   * @brief Called after a tick's movement hit something
   */
  virtual void onBlocked(World &world, const MoveResult &result) {
    (void) world;
    (void) result;
  }

private:
  bool removed = false;
};

} // namespace te
