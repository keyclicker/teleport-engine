#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "Buffer.hpp"

class Game {
private:
  Map map;
  Player player;
  Buffer bf;

public:
  Game(const Game &game) = delete;
  Game &operator=(const Game &game) = delete;

  Game(uint16_t width, uint16_t height, char *str):
    bf(Buffer(width, height, str)) {}

  void setMap(const Map &m) { //Temp
    map = m;
  }

  void gameLoop();

private:
  void renderSector(const Map::Sector &sec);

};