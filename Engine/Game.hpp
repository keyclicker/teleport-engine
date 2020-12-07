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

  void setMap(const Map &m) { //todo move
    map = m;
  }

  void gameLoop();

private:
  void renderSector(const Map::Sector *sec);

  static Map::Vertex intersec(const Map::Vertex &v1, const Map::Vertex &v2,
                       const Map::Vertex &v3, const Map::Vertex &v4);

  static bool isVertexOnSeg(const Map::Vertex &v,
          const Map::Vertex &sv1, const Map::Vertex &sv2);

  static int fit(int var, int a, int b);
};