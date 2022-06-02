#pragma once

#include <vector>

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

  // Vertical clipping is global for all portal branches,
  // B

  // Vertical clipping of the frame
  // gets to initial state after each frame
  std::vector<Segment<>> vClip;

  // Used for storing horizontal clipping
  // for each portal branch and providing interface
  // for clipping both vClip and horizontal clip
  struct Clip {
    Game *game;
    Segment<> hClip;

    explicit Clip(Game *game): game(game), hClip({-1, 1}) {};
    Clip(Game *game, Segment<> hClip): game(game), hClip(hClip) {};

    Clip clamped(Plain portal) const {
      auto phClamped = hClamp(portal.hSeg);
      Segment<> hClamped = {
          std::max(hClip.begin, phClamped.begin),
          std::min(hClip.end, phClamped.end)
      };

      auto hSeg = game->hToScreen(hClamped);
      auto &vClip = game->vClip;

      for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
        auto k = (double) (game->xToView(x) - portal.hSeg.begin)
            / (portal.hSeg.end - portal.hSeg.begin);

        Segment<> secCol = vClamp(x,{
            interpolate(portal.lSeg.begin, portal.rSeg.begin, k),
            interpolate(portal.lSeg.end, portal.rSeg.end, k)
        });

        vClip[x] = {
            std::min(vClip[x].begin, secCol.begin),
            std::max(vClip[x].end, secCol.end)
        };
      }

      return Clip(game, hClamped);
    }

    Segment<> hClamp(Segment<> seg) const {
      return {
          std::clamp(seg.begin, hClip.begin, hClip.end),
          std::clamp(seg.end, hClip.begin, hClip.end)
      };
    }

    Segment<> vClamp(uint16_t x, Segment<> seg) const {
      auto &vClip = game->vClip;
      return {
          std::clamp(seg.begin, vClip[x].begin, vClip[x].end),
          std::clamp(seg.end, vClip[x].begin, vClip[x].end)
      };
    }
  };

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
   * @brief Game loop
   */
  void gameLoop();


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

  void renderPlain(Plain plain, Clip clip, sf::Color color);

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

  uint16_t xToScreen(double x) {
    return (uint16_t) bf.getWidth() * ((x + 1.0) / 2.0);
  }

  uint16_t yToScreen(double y) {
    return (uint16_t) bf.getHeight() * ((1.0 - y) / 2.0);
  }

  Segment<uint16_t> hToScreen(Segment<double> hSeg) {
    return {
        xToScreen(hSeg.begin),
        xToScreen(hSeg.end)
    };
  }

  Segment<uint16_t> vToScreen(Segment<double> vSeg) {
    return {
        yToScreen(vSeg.begin),
        yToScreen(vSeg.end)
    };
  }

  double xToView(uint16_t x) {
    return (double) x / bf.getWidth() * 2.0 - 1.0;
  }

  double yToView(uint16_t y) {
    return (double) y / bf.getHeight() * 2.0 - 1.0;
  }


};