#pragma once

#include <set>
#include <utility>

#include "Utils/Utils.hpp"
#include "Utils/Vector.hpp"
#include "Utils/Clip.hpp"
#include "Buffer.hpp"
#include "Map.hpp"

class Game;

class Renderer {
private:
  static constexpr int MAX_DEPTH = 10;

  friend class Clip;
  friend class Game;

  Game *game;
  Buffer bf;

  // Vertical clipping of the frame
  // gets to initial state after each frame
  std::vector<Segment<>> vClip;
  std::set<std::tuple<Map::Sector*, double, double>> visitedSectors;

public:
  Renderer(uint16_t width, uint16_t height, char *str, Game *game):
    bf(width, height, str), game(game) {}

  void renderFrame();

  /**
 * @brief Renders sector
 *
 * @param pos View position
 * @param sec Sector to render
 * @param clip Clip info
 * @param portal Portal
 */
  void renderSector(const Vector &pos, Map::Sector *sec,
                    const Clip &clip, Map::Line *portal = nullptr);

  /**
   * @brief Renders wall of the sector
   *
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param portal ...
   */
  void renderWalls(const Vector &pos, const Map::Sector *sec,
                   const Clip &clip, Map::Line *portal);


  /**
   * @brief Renders wall
   */
  Clip renderPlane(Plane plane, Clip clip, const Map::Line *line,
                   Segment<Vector> tLine, Segment<Vector> ctLine);

  void renderVisplane(const Vector &pos, const Map::Sector *sec,
                      const Clip &clip, bool isFloor);

  void __debugRenderVClip();

  uint16_t xScreenToBuff(double x);

  uint16_t yScreenToBuff(double y);

  Segment<uint16_t> hScreenToBuff(Segment<double> hSeg);

  Segment<uint16_t> vScreenToBuff(Segment<double> vSeg);

  double xBuffToScreen(uint16_t x);

  double yBuffToScreen(uint16_t y);

  double zFunction(double z) {
    return std::clamp(255.0 * 2000.0 / z / z +
            std::clamp(100.0 + 255.0 * 250.0 / z, 0.0, 150.0),
            0.0, 255.0);
  }
};
