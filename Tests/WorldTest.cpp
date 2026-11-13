// Headless checks of map loading, movement through portals and traces,
// on the test map. Run with `make test`.

#include <cmath>
#include <cstdio>
#include <string>

#include "../Engine/World/MapLoader.hpp"
#include "../Engine/World/World.hpp"

using namespace te;

static int failures = 0;

/**
 * @brief Report a failed check without stopping
 */
static void check(bool ok, const std::string &what) {
  std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
  if (!ok) ++failures;
}

static bool near(Vec2 a, Vec2 b, float tolerance) {
  return (a - b).length() < tolerance;
}

/**
 * @brief Walk at a fixed velocity for some seconds, 60 ticks a second
 */
static void walk(World &world, Entity &entity, Vec2 velocity,
                 float seconds) {
  for (int i = 0; i < int(seconds * 60); ++i) {
    entity.velocity = velocity;
    world.update(1.0f / 60);
  }
}

int main() {
  TextureCache textures("Resources/Textures/");
  World world(loadMap("Resources/Maps/teleport.json", textures));
  const Map &map = world.getMap();

  check(map.sectors.size() == 10, "loads 10 sectors");

  for (const auto &sector : map.sectors) {
    for (const auto &wall : sector->walls) {
      // Every interior point sees its walls clockwise
      Vec2 center;
      for (const auto &w : sector->walls) center += w.a;
      center = center / float(sector->walls.size());

      if (wall.sideOf(center) <= 0) {
        check(false, sector->name + " winds clockwise");
      }

      // Portals land on their partner's ends, in reverse
      if (wall.portal) {
        bool lands = near(wall.toPartner.apply(wall.a), wall.portal->b, 0.01f)
                  && near(wall.toPartner.apply(wall.b), wall.portal->a, 0.01f);
        if (!lands) check(false, sector->name + " portal lands on partner");
      }
    }
  }

  // ---- The teleport loop: hall -> tunnel -> loop -> back into the hall --

  Sector *hall = map.find("hall");
  auto body = world.spawn<Entity>(hall, {300, -200}, 0);

  walk(world, *body, {300, 0}, 0.5f);
  check(body->sector == map.find("tunnel"), "east into the tunnel");

  walk(world, *body, {0, -300}, 2);
  check(body->sector == map.find("tunnel"), "south along the tunnel");

  walk(world, *body, {300, 0}, 2);
  check(body->sector == map.find("loop"), "east into the loop room");

  walk(world, *body, {300, 0}, 1.4f);
  check(body->sector == hall, "out of the loop, back in the hall");
  check(body->pos.x > -400 && body->pos.x < -300,
        "came out of the hall's west wall, x=" + std::to_string(body->pos.x));

  // ---- Stairs: annex -> landing -> up the steps ------------------------

  Sector *landing = map.find("landing");
  auto climber = world.spawn<Entity>(landing, {680, 560}, 0);

  walk(world, *climber, {0, -300}, 0.6f);
  check(climber->sector == map.find("stair1"), "steps onto stair 1");
  check(std::abs(climber->z - map.find("stair1")->floor) < 0.01f,
        "stands on stair 1's floor");

  walk(world, *climber, {0, -300}, 1.5f);
  check(climber->sector == map.find("secret"), "climbs to the secret room");

  // ---- Walls stop and slide ----------------------------------------------

  auto bumper = world.spawn<Entity>(hall, {0, -200}, 0);
  walk(world, *bumper, {-300, 0}, 3);
  check(bumper->sector == hall && std::abs(bumper->pos.x + 384) < 0.5f,
        "wall stops at the radius, x=" + std::to_string(bumper->pos.x));

  auto wallHugger = world.spawn<Entity>(hall, {-300, -100}, 0);
  walk(world, *wallHugger, {-300, -300}, 3);
  check(hall->contains(wallHugger->pos), "corner keeps it inside");

  // ---- Traces ------------------------------------------------------------

  // East from the hall center: the tunnel's far wall at x = 800
  auto hit = world.trace(hall, {0, -300}, 30, {1, 0}, 5000, bumper.get());
  check(hit.hit && hit.wall && hit.sector == map.find("tunnel"),
        "trace stops on the tunnel wall");
  check(std::abs(hit.distance - 800) < 1,
        "trace distance " + std::to_string(hit.distance));

  auto target = world.spawn<Entity>(map.find("tunnel"), {600, -300}, 0);
  hit = world.trace(hall, {0, -300}, 30, {1, 0}, 5000, bumper.get());
  check(hit.entity == target.get(), "trace hits an entity behind a portal");

  std::printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
