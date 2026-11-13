#include "World.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace te {

// Portal hops a single trace may take, guards against mirror loops
static constexpr int MAX_TRACE_HOPS = 64;

// Push-out passes per step: corners need more than one
static constexpr int COLLIDE_PASSES = 4;

// ---- Sector membership ---------------------------------------------------

void World::link(Entity &entity, Sector *sector) {
  entity.sector = sector;
  sector->entities.push_back(&entity);
}

void World::unlink(Entity &entity) {
  auto &list = entity.sector->entities;
  list.erase(std::find(list.begin(), list.end(), &entity));
  entity.sector = nullptr;
}

// ---- Tick ----------------------------------------------------------------

void World::update(float dt) {
  // By index: update() may spawn, which grows the vector
  for (size_t i = 0; i < entities.size(); ++i) {
    auto entity = entities[i];
    if (entity->isRemoved()) continue;

    entity->update(*this, dt);
    if (entity->isRemoved()) continue;

    MoveResult result;

    if (entity->velocity.x != 0 || entity->velocity.y != 0) {
      result = move(*entity, entity->velocity * dt);
    }

    moveVertically(*entity, dt, result);

    if (result.blocked || result.hitEntity) {
      entity->onBlocked(*this, result);
    }
  }

  // Drop removed entities, sector lists first
  for (auto &entity : entities) {
    if (entity->isRemoved() && entity->sector) unlink(*entity);
  }

  std::erase_if(entities, [](const auto &e) { return e->isRemoved(); });
}

void World::moveVertically(Entity &entity, float dt, MoveResult &result) {
  const Sector &sector = *entity.sector;

  if (entity.gravity) entity.verticalVelocity -= gravity * dt;
  entity.z += entity.verticalVelocity * dt;

  entity.onGround = false;

  if (entity.z <= sector.floor) {
    entity.z = sector.floor;
    entity.verticalVelocity = std::max(entity.verticalVelocity, 0.0f);
    entity.onGround = entity.gravity;
    result.blocked |= !entity.gravity;
  }

  if (entity.z + entity.height > sector.ceiling) {
    entity.z = std::max(sector.floor, sector.ceiling - entity.height);
    entity.verticalVelocity = std::min(entity.verticalVelocity, 0.0f);
    result.blocked |= !entity.gravity;
  }
}

// ---- Horizontal movement -------------------------------------------------

bool World::canEnter(const Entity &entity, const Wall &portal) const {
  const Sector &next = *portal.next();

  if (entity.gravity) {
    if (next.floor - entity.z > entity.stepHeight) return false;
  } else if (entity.z < next.floor) {
    return false;
  }

  float bottom = std::max(entity.z, next.floor);
  return bottom + entity.height <= next.ceiling;
}

MoveResult World::move(Entity &entity, Vec2 delta) {
  MoveResult result;

  // Steps under half the radius can't skip over a wall
  float length = delta.length();
  int steps = std::max(1, int(std::ceil(length / (entity.radius * 0.5f))));
  Vec2 step = delta / float(steps);

  for (int i = 0; i < steps; ++i) {
    Vec2 from = entity.pos;
    entity.pos += step;

    collide(entity, result);
    crossPortals(entity, from, step, result);
  }

  return result;
}

void World::collide(Entity &entity, MoveResult &result) {
  float radius = entity.radius;

  // Push the center `radius` away from a segment, in this sector's space
  auto pushFrom = [&](Vec2 a, Vec2 b, Vec2 inward) {
    Vec2 closest = closestOnSegment(a, b, entity.pos);
    Vec2 away = entity.pos - closest;
    float distance = away.length();

    if (distance >= radius) return;

    Vec2 normal = distance > 1e-4f ? away / distance : inward;
    entity.pos += normal * (radius - distance);

    // Slide: drop the velocity part that goes into the wall
    float into = entity.velocity.dot(normal);
    if (into < 0) entity.velocity -= normal * into;

    result.blocked = true;
  };

  for (int pass = 0; pass < COLLIDE_PASSES; ++pass) {
    for (const Wall &wall : entity.sector->walls) {
      if (!wall.portal || !canEnter(entity, wall)) {
        pushFrom(wall.a, wall.b, wall.inwardNormal());
        continue;
      }

      // Near an open portal, the walls behind it count too
      Vec2 closest = closestOnSegment(wall.a, wall.b, entity.pos);
      if ((entity.pos - closest).length() >= radius) continue;

      const Wall &partner = *wall.portal;
      const Transform &back = partner.toPartner;

      for (const Wall &far : partner.sector->walls) {
        if (&far == &partner) continue;
        if (far.portal && canEnter(entity, far)) continue;

        pushFrom(back.apply(far.a), back.apply(far.b),
                 back.applyDir(far.inwardNormal()));
      }
    }

    for (Entity *other : entity.sector->entities) {
      if (other == &entity || !other->blocking) continue;
      if (other == entity.ignored || other->ignored == &entity) continue;
      if (other->isRemoved()) continue;

      bool overlapsZ = entity.z < other->z + other->height &&
                       other->z < entity.z + entity.height;
      if (!overlapsZ) continue;

      Vec2 away = entity.pos - other->pos;
      float distance = away.length();
      float minDistance = radius + other->radius;

      if (distance >= minDistance) continue;

      result.hitEntity = other;

      // Non-blocking movers (projectiles) only report the touch
      if (entity.blocking && distance > 1e-4f) {
        entity.pos += away / distance * (minDistance - distance);
      }
    }
  }
}

void World::crossPortals(Entity &entity, Vec2 from, Vec2 &step,
                         MoveResult &result) {
  for (const Wall &wall : entity.sector->walls) {
    if (!wall.portal) continue;

    float sideFrom = wall.sideOf(from);
    float sideTo = wall.sideOf(entity.pos);

    if (sideFrom < 0 || sideTo >= 0) continue;

    // Crossed the line; check it was within the wall, not beside it
    Vec2 crossing = lerp(from, entity.pos, sideFrom / (sideFrom - sideTo));
    Vec2 along = wall.b - wall.a;
    float t = (crossing - wall.a).dot(along) / along.lengthSquared();

    if (t < 0 || t > 1) continue;

    if (!canEnter(entity, wall)) {
      entity.pos = from;
      result.blocked = true;
      return;
    }

    const Transform &teleport = wall.toPartner;
    Sector *next = wall.next();

    entity.pos = teleport.apply(entity.pos);
    entity.velocity = teleport.applyDir(entity.velocity);
    entity.angle += teleport.rotation.angle();
    step = teleport.applyDir(step);

    // Walk up the step right away, falling is moveVertically()'s job
    if (entity.gravity) entity.z = std::max(entity.z, next->floor);

    unlink(entity);
    link(entity, next);

    result.teleported = true;
    return;
  }
}

// ---- Traces --------------------------------------------------------------

TraceHit World::trace(Sector *sector, Vec2 from, float z, Vec2 dir,
                      float maxDistance, const Entity *ignored) const {
  constexpr float INF = std::numeric_limits<float>::infinity();

  TraceHit hit;
  float travelled = 0;

  for (int hop = 0; hop < MAX_TRACE_HOPS; ++hop) {
    // Wall the ray leaves through
    const Wall *exit = nullptr;
    float exitT = INF;

    for (const Wall &wall : sector->walls) {
      float approach = dir.dot(wall.inwardNormal());
      if (approach >= 0) continue;

      float t = std::max(0.0f, wall.sideOf(from) / -approach);
      Vec2 point = from + dir * t;

      Vec2 along = wall.b - wall.a;
      float u = (point - wall.a).dot(along) / along.lengthSquared();

      if (u >= -1e-3f && u <= 1 + 1e-3f && t < exitT) {
        exit = &wall;
        exitT = t;
      }
    }

    // Nearest blocking entity before that wall
    Entity *target = nullptr;
    float targetT = exitT;

    for (Entity *entity : sector->entities) {
      if (entity == ignored || !entity->blocking) continue;
      if (z < entity->z || z > entity->z + entity->height) continue;

      Vec2 toCenter = entity->pos - from;
      float along = toCenter.dot(dir);
      float miss2 = toCenter.lengthSquared() - along * along;
      float r2 = entity->radius * entity->radius;

      if (miss2 > r2) continue;

      float t = along - std::sqrt(r2 - miss2);
      if (t < 0) t = 0;
      if (along < 0 || t >= targetT) continue;

      target = entity;
      targetT = t;
    }

    float stopT = std::min(targetT, exitT);
    bool inRange = travelled + stopT <= maxDistance;

    hit.sector = sector;

    if (!inRange || (!target && !exit)) {
      hit.pos = from + dir * (maxDistance - travelled);
      hit.distance = maxDistance;
      return hit;
    }

    hit.pos = from + dir * stopT;
    hit.distance = travelled + stopT;

    if (target) {
      hit.hit = true;
      hit.entity = target;
      return hit;
    }

    // Through the portal if the opening spans the ray's height
    const Sector *next = exit->next();
    bool open = next && z > next->floor && z < next->ceiling;

    if (!open) {
      hit.hit = true;
      hit.wall = exit;
      return hit;
    }

    const Transform &teleport = exit->toPartner;
    from = teleport.apply(hit.pos);
    dir = teleport.applyDir(dir);
    travelled = hit.distance;
    sector = exit->portal->sector;
  }

  return hit;
}

} // namespace te
