#pragma once

#include <memory>
#include <vector>

#include "Entity.hpp"
#include "Map.hpp"

namespace te {

/**
 * @brief Where a World::trace() ended
 */
struct TraceHit {
  // Stopped by a wall or an entity, not by distance
  bool hit = false;

  Entity *entity = nullptr;
  const Wall *wall = nullptr;

  // Where it ended, in that sector's space
  Sector *sector = nullptr;
  Vec2 pos;
  float z = 0;

  // Ray direction there: portals may have turned it
  Vec2 dir;

  // Along the ray, portals included
  float distance = 0;
};

/**
 * @brief A map with entities in it: spawning, physics and ray casts
 *
 * Space is portal-aware throughout: moving and tracing through a portal
 * continues in the partner's space, turned and shifted.
 */
class World {
private:
  Map map;
  std::vector<std::shared_ptr<Entity>> entities;

  void link(Entity &entity, Sector *sector);
  void unlink(Entity &entity);

  /**
   * @brief Whether an entity fits through a portal: step and headroom
   */
  bool canEnter(const Entity &entity, const Wall &portal) const;

  /**
   * @brief Push an entity out of walls and blocking entities
   */
  void collide(Entity &entity, MoveResult &result);

  /**
   * @brief Teleport an entity whose last step crossed a portal
   *
   * @param from Position before the step
   * @param step The step, turned along with the entity
   */
  void crossPortals(Entity &entity, Vec2 from, Vec2 &step,
                    MoveResult &result);

  /**
   * @brief Gravity, floor and ceiling
   */
  void moveVertically(Entity &entity, float dt, MoveResult &result);

public:
  // Downward acceleration, world units per second squared
  float gravity = 800;

  explicit World(Map map): map(std::move(map)) {}

  const Map &getMap() const { return map; }

  const std::vector<std::shared_ptr<Entity>> &getEntities() const {
    return entities;
  }

  /**
   * @brief Add an entity standing on the sector's floor
   *
   * @tparam T Entity type
   * @param sector Sector to put it in
   * @param pos Position in that sector
   * @param angle Facing in radians
   * @param args Forwarded to T's constructor
   */
  template <typename T, typename... Args>
  std::shared_ptr<T> spawn(Sector *sector, Vec2 pos, float angle,
                           Args &&...args) {
    auto entity = std::make_shared<T>(std::forward<Args>(args)...);
    entity->pos = pos;
    entity->z = sector->floor;
    entity->angle = angle;

    link(*entity, sector);
    entities.push_back(entity);

    return entity;
  }

  /**
   * @brief Run every entity's update(), move them, drop removed ones
   *
   * @param dt Seconds, keep it small: big steps tunnel through things
   */
  void update(float dt);

  /**
   * @brief Slide an entity by `delta`, through portals it fits
   */
  MoveResult move(Entity &entity, Vec2 delta);

  /**
   * @brief Cast a ray through portals
   *
   * Stops at solid walls, floors and ceilings, at portals whose opening
   * the ray misses, and at blocking entities whose height it passes.
   *
   * @param sector Start sector
   * @param from Start, in that sector's space
   * @param z Start height
   * @param dir Unit horizontal direction
   * @param maxDistance Range, measured horizontally
   * @param ignored Entity the ray passes through, e.g. the shooter
   * @param slope Height gained per unit of horizontal distance
   */
  TraceHit trace(Sector *sector, Vec2 from, float z, Vec2 dir,
                 float maxDistance, const Entity *ignored = nullptr,
                 float slope = 0) const;
};

} // namespace te
