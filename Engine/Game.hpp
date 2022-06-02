#pragma once

#include <vector>
#include <memory>

#include "Map.hpp"
#include "Player.hpp"
#include "Utils/Math.hpp"
#include "Renderer.hpp"

class Game {
private:
  friend class Renderer;

  Map *map{};
  Player player;

  // Sector that player currently is in
  Map::Sector *activeSector{};

  Renderer renderer;

public:
  Game(const Game &game) = delete;
  Game &operator=(const Game &game) = delete;

  Game(uint16_t width, uint16_t height, char *str):
    renderer(width, height, str, this) {}

  void setMap(Map *m) { //todo move
    map = m;
    activeSector = map->sectors.front();
  }

  ~Game() {
    delete map;
  }

  /**
   * @brief Game loop
   */
  void gameLoop();

private:
  /**
   * @brief Move player considering walls collision
   * 
   * @param mv Movement vector
   */
  void move(const Vector &mv);

  /**
   * @brief Returns line collided with the player
   * 
   * @param pos Player position
   * @return Map::Line* 
   */
  Map::Line *collidedLine(const Vector &pos);
};