#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Graphics/Texture.hpp"
#include "../Math/Vec2.hpp"

namespace te {

struct Sector;
class Entity;

using TextureRef = std::shared_ptr<const Texture>;

/**
 * @brief One edge of a sector, solid or a portal
 *
 * Walls run clockwise around their sector (the loader fixes winding),
 * so the interior is on the right of `a` -> `b`.
 *
 * A portal joins this wall to a partner wall of equal length in another
 * sector, or the same one. The partner may sit anywhere on the map,
 * turned any way: crossing the portal teleports, and sight follows.
 * Plain neighbors are the special case of partners that coincide.
 */
struct Wall {
  Vec2 a, b;

  Sector *sector = nullptr;

  // Partner wall, or nullptr for a solid wall
  Wall *portal = nullptr;

  // Maps this sector's space to the partner sector's, for portals
  Transform toPartner;

  // Solid walls draw `middle`; portals draw `upper` above the partner's
  // ceiling and `lower` below its floor. Missing parts draw black.
  TextureRef upper, middle, lower;

  // World units per texel; 0 stretches the texture to fit each part's
  // height
  float textureScale = 0;

  float length() const { return (b - a).length(); }

  /**
   * @brief Unit normal pointing into the sector
   */
  Vec2 inwardNormal() const { return (b - a).right().normalized(); }

  /**
   * @brief Signed distance from the wall's line, > 0 inside the sector
   */
  float sideOf(Vec2 p) const { return (p - a).dot(inwardNormal()); }

  /**
   * @brief The sector behind a portal, nullptr for a solid wall
   */
  Sector *next() const { return portal ? portal->sector : nullptr; }
};

/**
 * @brief Convex room with flat floor and ceiling
 */
struct Sector {
  std::string name;

  float floor = 0, ceiling = 128;
  TextureRef floorTexture, ceilingTexture;

  // World units per floor and ceiling texel
  float flatScale = 1;

  // Brightness multiplier, 1 is normal
  float light = 1;

  std::vector<Wall> walls;

  // Entities inside, kept by World
  std::vector<Entity *> entities;

  /**
   * @brief Whether a point is inside: on the inner side of every wall
   */
  bool contains(Vec2 p) const {
    for (const auto &wall : walls) {
      if (wall.sideOf(p) < 0) return false;
    }

    return true;
  }
};

/**
 * @brief Something a map places, for the game to spawn by type
 */
struct Thing {
  std::string type;
  Sector *sector = nullptr;
  Vec2 pos;

  // Radians, 0 faces +x
  float angle = 0;
};

/**
 * @brief Sectors joined by portals, plus things to spawn
 */
struct Map {
  // Pointers stay valid: walls and portals point into them
  std::vector<std::unique_ptr<Sector>> sectors;
  std::vector<Thing> things;

  /**
   * @brief Sector by name, nullptr if missing
   */
  Sector *find(const std::string &name) const {
    for (const auto &sector : sectors) {
      if (sector->name == name) return sector.get();
    }

    return nullptr;
  }
};

} // namespace te
