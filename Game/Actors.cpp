#include "Actors.hpp"

#include <algorithm>
#include <cmath>

using namespace te;

// ---- Effect --------------------------------------------------------------

Effect::Effect(const Assets &assets, float scale): assets(assets) {
  gravity = false;
  blocking = false;
  radius = 1;
  height = 1;
  spriteScale = scale;
  sprite = assets.puff[0];
}

void Effect::update(World &, float dt) {
  age += dt;

  if (age >= DURATION) {
    remove();
    return;
  }

  sprite = assets.puff[std::min(2, int(age / DURATION * 3))];
}

void spawnPuff(World &world, const Assets &assets, Sector *sector, Vec2 pos,
               float z, float scale) {
  auto puff = world.spawn<Effect>(sector, pos, 0, assets, scale);

  // Sprites stand on their z; center this one on the point instead
  float half = assets.puff[0]->getHeight() * scale / 2;
  puff->z = std::clamp(z - half, sector->floor, sector->ceiling - half);
}

// ---- Fireball ------------------------------------------------------------

Fireball::Fireball(const Assets &assets, const Entity *shooter):
  assets(assets) {
  gravity = false;
  blocking = false;
  ignored = shooter;
  radius = 6;
  height = 12;
  spriteScale = 1;
  sprite = assets.fireball[0];
}

void Fireball::update(World &, float dt) {
  age += dt;
  sprite = assets.fireball[int(age * 10) % 2];

  if (age > 6) remove();
}

void Fireball::onBlocked(World &world, const MoveResult &result) {
  // Hits show as pain and red flashes; a burst would fill the screen
  if (auto actor = dynamic_cast<Actor *>(result.hitEntity)) {
    actor->damage(world, damageAmount, this);
  } else {
    spawnPuff(world, assets, sector, pos, z + height / 2, 1.5f);
  }

  remove();
}

// ---- Monster -------------------------------------------------------------

// Eye and hand heights over its feet
static constexpr float MONSTER_EYE = 44;
static constexpr float MONSTER_HAND = 36;

// Aim at this height over the target's feet
static constexpr float TARGET_CHEST = 30;

static constexpr float FIREBALL_SPEED = 420;
static constexpr float SIGHT_RANGE = 2000;
static constexpr float MELEE_REACH = 14;

Monster::Monster(const Assets &assets, Actor &target, unsigned seed):
  assets(assets), target(target), rng(seed) {
  radius = 20;
  height = 56;
  health = 60;
  spriteScale = 0.9f;
  sprite = assets.monsterWalk[0];

  roam = Vec2::fromAngle(random(0, 2 * PI));
}

bool Monster::canSee(World &world) const {
  Vec2 toTarget = target.pos - pos;
  float distance = toTarget.length();

  if (distance > SIGHT_RANGE || distance < 1) return false;

  float eye = z + MONSTER_EYE;
  float slope = (target.z + TARGET_CHEST - eye) / distance;

  auto hit = world.trace(sector, pos, eye, toTarget / distance,
                         distance + target.radius, this, slope);

  return hit.entity == &target;
}

void Monster::throwFireball(World &world) {
  Vec2 toTarget = target.pos - pos;
  float distance = std::max(1.0f, toTarget.length());

  auto ball = world.spawn<Fireball>(sector, pos, angle, assets, this);
  ball->z = z + MONSTER_HAND;
  ball->velocity = toTarget / distance * FIREBALL_SPEED;

  // Climb or drop to arrive at chest height
  float flight = distance / FIREBALL_SPEED;
  ball->verticalVelocity = (target.z + TARGET_CHEST - ball->z) / flight;
}

void Monster::update(World &world, float dt) {
  animTime += dt;
  attackCooldown -= dt;
  stateTime -= dt;

  switch (state) {
  case State::Dead:
    velocity = {};
    return;

  case State::Pain:
    velocity = {};
    if (stateTime <= 0) state = State::Chase;
    return;

  case State::Windup:
    velocity = {};
    angle = (target.pos - pos).angle();

    if (stateTime <= 0) {
      throwFireball(world);
      state = State::Chase;
      sprite = assets.monsterWalk[0];
      attackCooldown = random(1.2f, 2.8f);
    }
    return;

  case State::Idle:
  case State::Chase:
    break;
  }

  // ---- Look around, a few times a second ----

  sightTime -= dt;

  if (sightTime <= 0) {
    sightTime = 0.25f;
    seesTarget = !target.isDead() && canSee(world);
    if (seesTarget) state = State::Chase;
  }

  if (state == State::Idle) {
    velocity = {};
    sprite = assets.monsterWalk[0];
    return;
  }

  // ---- Attack ----

  Vec2 toTarget = target.pos - pos;
  float distance = toTarget.length();
  bool inReach = distance < radius + target.radius + MELEE_REACH;

  if (seesTarget && attackCooldown <= 0) {
    if (inReach) {
      target.damage(world, 8, this);
      attackCooldown = 1;
    } else if (distance > 120) {
      state = State::Windup;
      stateTime = 0.45f;
      sprite = assets.monsterAttack;
      velocity = {};
      return;
    }
  }

  // ---- Move: straight at the target, or roam when lost or stuck ----

  Vec2 dir;

  if (roamTime > 0) {
    roamTime -= dt;
    dir = roam;
  } else if (seesTarget) {
    dir = toTarget / std::max(distance, 1.0f);
  } else {
    dir = roam;
  }

  velocity = inReach ? Vec2{} : dir * SPEED;
  angle = dir.angle();
  sprite = assets.monsterWalk[int(animTime * 5) % 2];
}

void Monster::onBlocked(World &, const MoveResult &result) {
  if (state == State::Dead || !result.blocked) return;

  // Bumped into a wall: try another way for a moment
  roam = Vec2::fromAngle(random(0, 2 * PI));
  roamTime = random(0.4f, 1.0f);
}

void Monster::damage(World &world, int amount, Entity *source) {
  if (state == State::Dead) return;

  Actor::damage(world, amount, source);

  if (isDead()) {
    state = State::Dead;
    sprite = assets.monsterDead;
    blocking = false;
    velocity = {};
    return;
  }

  // Getting shot wakes it and turns it toward the fight
  roamTime = 0;
  sightTime = 0;

  if (random(0, 1) < 0.5f) {
    state = State::Pain;
    stateTime = 0.25f;
    sprite = assets.monsterPain;
  } else if (state == State::Idle) {
    state = State::Chase;
  }
}

// ---- Pickup --------------------------------------------------------------

Pickup::Pickup(const Assets &assets, Kind kind): kind(kind) {
  blocking = false;
  radius = 16;
  height = 16;
  spriteScale = 1.2f;

  switch (kind) {
  case Kind::Medkit: sprite = assets.medkit; break;
  case Kind::Ammo: sprite = assets.ammo; break;
  case Kind::Shotgun: sprite = assets.shotgunPickup; break;
  }
}
