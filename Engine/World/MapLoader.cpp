#include "MapLoader.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace te {

using json = nlohmann::json;

// ---- Helpers -----------------------------------------------------------

/**
 * @brief Map file error with the location it happened at
 */
static std::runtime_error mapError(const std::string &where,
                                   const std::string &what) {
  return std::runtime_error("Map " + where + ": " + what);
}

static Vec2 readVec2(const json &value) {
  return {value.at(0).get<float>(), value.at(1).get<float>()};
}

/**
 * @brief First of `wall`, `style`, `sectorStyle` that has `key`
 */
static const json *lookup(const std::string &key, const json &wall,
                          const json *style, const json *sectorStyle) {
  for (const json *source : {&wall, style, sectorStyle}) {
    if (source && source->contains(key)) return &source->at(key);
  }

  return nullptr;
}

/**
 * @brief Style object by name, nullptr if `name` is null
 */
static const json *findStyle(const json &styles, const json *name,
                             const std::string &where) {
  if (!name) return nullptr;

  auto key = name->get<std::string>();
  if (!styles.contains(key)) throw mapError(where, "no style \"" + key + "\"");

  return &styles.at(key);
}

/**
 * @brief Signed area, > 0 when the walls run counterclockwise
 */
static float signedArea(const Sector &sector) {
  float area = 0;

  for (const auto &wall : sector.walls) {
    area += wall.a.cross(wall.b);
  }

  return area / 2;
}

/**
 * @brief Warn about vertices outside some wall's inner side
 */
static void checkConvex(const Sector &sector) {
  for (const auto &wall : sector.walls) {
    for (const auto &other : sector.walls) {
      if (wall.sideOf(other.a) < -0.01f) {
        std::cerr << "Map sector \"" << sector.name
                  << "\" is not convex, it may render wrong" << std::endl;
        return;
      }
    }
  }
}

// ---- Loading -----------------------------------------------------------

/**
 * @brief Wall partner indices from the file, resolved after all sectors
 */
struct PendingPortal {
  Wall *wall;
  std::string sector;
  size_t index;
  std::string where;
};

static void readSector(const json &data, const json &styles,
                       TextureCache &textures, Map &map,
                       std::vector<PendingPortal> &portals) {
  auto sector = std::make_unique<Sector>();

  sector->name = data.at("name").get<std::string>();
  auto where = "sector \"" + sector->name + "\"";

  if (map.find(sector->name)) throw mapError(where, "duplicate name");

  sector->floor = data.value("floor", 0.0f);
  sector->ceiling = data.value("ceiling", 128.0f);
  sector->flatScale = data.value("flatScale", 1.0f);
  sector->light = data.value("light", 1.0f);

  if (data.contains("floorTexture")) {
    sector->floorTexture = textures.get(data["floorTexture"]);
  }
  if (data.contains("ceilingTexture")) {
    sector->ceilingTexture = textures.get(data["ceilingTexture"]);
  }

  auto sectorStyle = findStyle(styles,
    data.contains("style") ? &data["style"] : nullptr, where);

  const auto &walls = data.at("walls");
  if (walls.size() < 3) throw mapError(where, "needs at least 3 walls");

  sector->walls.resize(walls.size());

  for (size_t i = 0; i < walls.size(); ++i) {
    const auto &w = walls[i];
    auto wallWhere = where + " wall " + std::to_string(i);

    Wall &wall = sector->walls[i];
    wall.sector = sector.get();
    wall.a = readVec2(w.at("from"));
    wall.b = readVec2(walls[(i + 1) % walls.size()].at("from"));

    auto style = findStyle(styles, w.contains("style") ? &w["style"]
                                                       : nullptr, wallWhere);

    auto texture = [&](const char *key) -> TextureRef {
      auto name = lookup(key, w, style, sectorStyle);
      return name ? textures.get(*name) : nullptr;
    };

    wall.upper = texture("upper");
    wall.middle = texture("middle");
    wall.lower = texture("lower");

    if (auto scale = lookup("textureScale", w, style, sectorStyle)) {
      wall.textureScale = scale->get<float>();
    }

    if (w.contains("portal")) {
      portals.push_back({&wall, w["portal"].at(0).get<std::string>(),
                         w["portal"].at(1).get<size_t>(), wallWhere});
    }
  }

  // Clockwise, so the interior is on every wall's right
  if (signedArea(*sector) > 0) {
    for (auto &wall : sector->walls) std::swap(wall.a, wall.b);
  }

  checkConvex(*sector);
  map.sectors.push_back(std::move(sector));
}

/**
 * @brief Link a portal to its partner and compute the teleport
 */
static void linkPortal(const PendingPortal &pending, const Map &map) {
  Sector *target = map.find(pending.sector);

  if (!target) {
    throw mapError(pending.where, "no sector \"" + pending.sector + "\"");
  }
  if (pending.index >= target->walls.size()) {
    throw mapError(pending.where, "no wall " + std::to_string(pending.index)
                                  + " in \"" + pending.sector + "\"");
  }

  Wall &wall = *pending.wall;
  Wall &partner = target->walls[pending.index];

  if (&partner == &wall) throw mapError(pending.where, "portal to itself");

  if (std::abs(wall.length() - partner.length()) > 0.01f * wall.length()) {
    throw mapError(pending.where, "partner has another length");
  }

  wall.portal = &partner;

  // `a` lands on the partner's `b`: walls face each other through it
  auto rotation = Rotation::between(wall.b - wall.a, partner.a - partner.b);
  wall.toPartner = {rotation, partner.b - rotation.apply(wall.a)};
}

Map loadMap(const std::string &path, TextureCache &textures) {
  std::ifstream file(path);
  if (!file) throw std::runtime_error("Failed to open map \"" + path + "\"");

  json data;

  try {
    data = json::parse(file);
  } catch (const json::exception &e) {
    throw std::runtime_error("Map \"" + path + "\": " + e.what());
  }

  Map map;
  std::vector<PendingPortal> portals;

  try {
    auto styles = data.value("styles", json::object());

    for (const auto &sector : data.at("sectors")) {
      readSector(sector, styles, textures, map, portals);
    }

    for (const auto &portal : portals) {
      linkPortal(portal, map);
    }

    // Partners that named no portal link back to whoever named them
    for (const auto &portal : portals) {
      Wall &partner = *portal.wall->portal;

      if (!partner.portal) {
        partner.portal = portal.wall;
        partner.toPartner = portal.wall->toPartner.inverse();
      } else if (partner.portal != portal.wall) {
        throw mapError(portal.where, "partner leads to another wall");
      }
    }

    for (const auto &t : data.value("things", json::array())) {
      Thing thing;
      thing.type = t.at("type").get<std::string>();
      thing.pos = readVec2(t.at("pos"));
      thing.angle = radians(t.value("angle", 0.0f));

      auto sector = t.at("sector").get<std::string>();
      thing.sector = map.find(sector);

      if (!thing.sector) {
        throw mapError("thing \"" + thing.type + "\"",
                       "no sector \"" + sector + "\"");
      }

      map.things.push_back(thing);
    }
  } catch (const json::exception &e) {
    throw std::runtime_error("Map \"" + path + "\": " + e.what());
  }

  return map;
}

} // namespace te
