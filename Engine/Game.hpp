#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "Buffer.hpp"

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

  struct Clip {
    int16_t fLeft, fRight; //todo change design
    int16_t left, right;
    double leftStart, leftEnd;
    double rightStart, rightEnd;
  };


  void gameLoop();
  void renderFloor(const Map::Vertex &pos, const Map::Sector *sec, const Clip &clip);
  void renderCeiling(const Map::Vertex &pos, const Map::Sector *sec, const Clip &clip);

private:
  void move(const Map::Vertex &mv);
  void renderSector(const Map::Vertex &pos, const Map::Sector *sec, const Clip &clip, Map::Line *portal = nullptr);
  void renderWalls(const Map::Vertex &pos, const Map::Sector *sec, const Clip &clip, Map::Line *portal);
  void renderPlain(const Map::Vertex &pos, const Map::Sector *sec, const Map::Line *a,
                   const sf::Image &tex, const Game::Clip &clip, const Game::Clip &fclip);

  static Map::Vertex intersec(const Map::Vertex &v1, const Map::Vertex &v2,
                       const Map::Vertex &v3, const Map::Vertex &v4);


  Map::Line *collidedLine(const Map::Vertex &pos);
  static bool side(const Map::Vertex &v1, const Map::Vertex &v2, const Map::Vertex &v3);
  static double dist(const Map::Vertex &v1, const Map::Vertex &v2, const Map::Vertex &v3);

  static double deepFunc(double dist) {
    auto d = 100.0 / dist;
    d = fit(0.05 + d * 0.95, 0.0, 1.0);
    return 1;
  }

  template<typename T>
  static double grad(T a, T b, double k) {
    return ((1 - k) * a + k * b);
  }

  template<typename T>
  static T fit(T var, T a, T b) {
    if (var < a)
      return a;
    if (var > b)
      return b;
    return var;
  }
};