#pragma once

#include "Utils/Utils.hpp"
#include "Utils/Vector.hpp"
#include "Utils/Clip.hpp"
#include "Buffer.hpp"
#include "Map.hpp"

class Game;

class Renderer {
private:
  friend class Clip;
  friend class Game;

  Game *game;
  Buffer bf;

  // Vertical clipping of the frame
  // gets to initial state after each frame
  std::vector<Segment<>> vClip;

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
  void renderSector(const Vector &pos, const Map::Sector *sec,
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
  void renderPlain(Plain plain, Clip clip,
                   double height, Vector v1, Vector v2,
                   const sf::Image &texture);


  uint16_t xToScreen(double x);

  uint16_t yToScreen(double y);

  Segment<uint16_t> hToScreen(Segment<double> hSeg);

  Segment<uint16_t> vToScreen(Segment<double> vSeg);

  double xToView(uint16_t x);

  double yToView(uint16_t y);
};
