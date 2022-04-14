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
   * @brief Renders floor of the sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   */
  void renderFloor(const Map::Vertex &pos, const Map::Sector *sec, 
                  const Clip &clip);

  /**
   * @brief Renders ceiling of the sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   */
  void renderCeiling(const Map::Vertex &pos, const Map::Sector *sec, 
                     const Clip &clip);

private:
  /**
   * @brief Move player considering walls collision
   * 
   * @param mv Movement vector
   */
  void move(const Map::Vertex &mv);

  /**
   * @brief Renders sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param portal Portal
   */
  void renderSector(const Map::Vertex &pos, const Map::Sector *sec, 
                    const Clip &clip, Map::Line *portal = nullptr);

  /**
   * @brief Renders wall of the sector
   * 
   * @param pos View position
   * @param sec Sector to render
   * @param clip Clip info
   * @param portal ...
   */
  void renderWalls(const Map::Vertex &pos, const Map::Sector *sec, 
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
  void renderPlain(const Map::Vertex &pos, const Map::Sector *sec, 
                   const Map::Line *a, const sf::Image &tex, 
                   const Game::Clip &clip, const Game::Clip &fclip);

  /**
   * @brief Returns intersection of the lines
   * 
   * @param v1 First vertex of the first line
   * @param v2 Second vertex of the first line
   * @param v3 First vertex of the second line
   * @param v4 Second vertex of the second line
   * @return Map::Vertex Intersection
   */
  static Map::Vertex intersec(const Map::Vertex &v1, const Map::Vertex &v2,
                              const Map::Vertex &v3, const Map::Vertex &v4);

  /**
   * @brief Returns line collided with the player
   * 
   * @param pos Player position
   * @return Map::Line* 
   */
  Map::Line *collidedLine(const Map::Vertex &pos);

  /**
   * @brief Returns side of the line where vertex is located
   * 
   * @param lv1 First vertex of the line
   * @param lv2 Second vertex of the line
   * @param v Vertex
   * @return true Vertex is on the positive side of the line
   * @return false Vertex is on the negative side of the line
   */
  static bool side(const Map::Vertex &lv1, const Map::Vertex &lv2, 
                   const Map::Vertex &v);

  /**
   * @brief Distant from the line to the vertex
   * 
   * @param lv1 First vertex of the line
   * @param lv2 Second vertex of the line
   * @param v Vertex
   * @return double 
   */
  static double dist(const Map::Vertex &lv1, const Map::Vertex &lv2, 
                     const Map::Vertex &v);


  static double deepFunc(double dist) {
    auto d = 100.0 / dist;
    d = fit(0.05 + d * 0.95, 0.0, 1.0);
    return 1;
  }

  /**
   * @brief Returns value in range [a, b] depending on k,
   * k = 0.0 returns a, k = 1.0 returns b
   * 
   * @tparam T Range bounds type
   * @param a Range start
   * @param b Range end
   * @param k [0, 1]
   * @return double Value in range [a, b]
   */
  template<typename T>
  static double grad(T a, T b, double k) {
    return ((1 - k) * a + k * b);
  }


  /**
   * @brief Clips value to range
   * 
   * @tparam T Value type
   * @param value Value
   * @param a Range start
   * @param b Range end
   * @return T Result of cliping
   */
  template<typename T>
  static T fit(T value, T a, T b) {
    if (value < a)
      return a;
    if (value > b)
      return b;
    return value;
  }
};