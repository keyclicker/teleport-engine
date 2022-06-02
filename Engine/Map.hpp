#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include <list>
#include <cmath>
#include <cstdint>
#include <memory>

#include "Utils/Vector.hpp"

struct Map {
  struct Line;

  struct Side {
    sf::Image upper;
    sf::Image middle;
    sf::Image lower;
  };

  struct Sector {
    short	lightlevel = 60;
    double floorheight = -64;
    double ceilingheight = 80;

    sf::Image floor;
    sf::Image ceiling;

    std::vector<Line*> lines;

    Sector() = default;
    explicit Sector(Map *map) {
      map->sectors.push_back(this);
    }

    ~Sector() {
      for (auto a: lines) {
        delete a;
      }
    }
  };

  struct Line {
    Vector v1, v2;
    std::shared_ptr<Side> side;
    Line *portal;
    Sector *sector;

    Line(): sector(nullptr), portal(nullptr), v1(Vector()), v2(Vector()), side(nullptr)  {}

    Line(Sector *sector, Vector v1, Vector v2, std::shared_ptr<Side> side):
        sector(sector), v1(v1), v2(v2), side(std::move(side)), portal(nullptr) {
      sector->lines.push_back(this);
    }
  };

  std::vector<Sector*> sectors;

  ~Map() {
    for (auto a: sectors) {
      delete a;
    }
  }
};


