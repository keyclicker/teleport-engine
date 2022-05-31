#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "Buffer.hpp"
#include "Math.hpp"

class Game {
private:
  Map *map;
  Player player;
  Buffer bf;

  Map::Sector *activeSector;

public:
  Game(const Game &game) = delete;
  Game &operator=(const Game &game) = delete;

  Game(uint16_t width, uint16_t height, char *str):
    bf(Buffer(width, height, str)) {}

  void setMap(Map *m) { //todo move
    map = m;
    activeSector = map->sectors.front();
  }

  ~Game() {
    delete map;
  }

  /**
   * @brief 
   * 
   */
  struct Clip {
    int16_t fLeft, fRight; //todo change design
    int16_t left, right;
    double leftStart, leftEnd;
    double rightStart, rightEnd;
  };

  /**
   * @brief Game loop
   * 
   */
  void gameLoop();

  /**
   * @brief Renders floor or ceiling of the sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param floor true - render floor, false - render ceiling
   */
  void renderHorizontalPlane(const Vertex &pos, const Map::Sector *sec, 
                             const Clip &clip, bool floor);

private:
  /**
   * @brief Move player considering walls collision
   * 
   * @param mv Movement vector
   */
  void move(const Vertex &mv);

  /**
   * @brief Renders sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param portal Portal
   */
  void renderSector(const Vertex &pos, const Map::Sector *sec, 
                    const Clip &clip, Map::Line *portal = nullptr);

  /**
   * @brief Renders wall of the sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param portal ...
   */
  void renderWalls(const Vertex &pos, const Map::Sector *sec, 
                  const Clip &clip, Map::Line *portal);

  /**
   * @brief Renders plane
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param a ...
   * @param tex Plane texture
   * @param clip Clip info
   * @param fclip ...
   */
  void renderPlain(const Vertex &pos, const Map::Sector *sec, 
                   const Map::Line *a, const sf::Image &tex, 
                   const Game::Clip &clip, const Game::Clip &fclip);

  /**
   * @brief Returns line collided with the player
   * 
   * @param pos Player position
   * @return Map::Line* 
   */
  Map::Line *collidedLine(const Vertex &pos);

  /**
   * @brief Calculates the factor of darkening for pixel 
   * 
   * @param dist distance to pixel
   * @return double factor of darkening
   */
  double deepFunc(double dist) {
    auto d = 512.0 / dist;
    d = fit(0.05 + d * 0.95, 0.0, 1.0);
    return d;
  }
};