#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include <list>
#include <cmath>
#include <cstdint>
#include <memory>

//temporary
struct Color {
  uint8_t r, g, b;

  Color(): r(0), g(0), b(0) {}
  Color(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
};

class Map {
public:
  struct Vertex;
  struct Line;
  struct Side;
  struct Sector;

  std::vector<Sector*> sectors;

  ~Map() {
    for (auto a: sectors) {
      delete a;
    }
  }
};

struct Map::Vertex {
  double x, y;

  Vertex() = default;
  Vertex(double x, double y) : x(x), y(y) {}

  double length() const {
    return std::hypot(x, y);
  }

  Vertex operator-() const {
    return Vertex(-x, -y);
  }

  double operator*(const Vertex &rhs) const {
    return x * rhs.x + y * rhs.y;
  }
  Vertex operator+(const Vertex &rhs) const {
    return Vertex(x + rhs.x, y + rhs.y);
  }
  Vertex operator-(const Vertex &rhs) const {
    return Vertex(x - rhs.x, y - rhs.y);
  }

  friend Vertex operator*(double mlt, const Vertex &ver) {
    return Vertex(ver.x * mlt, ver.y * mlt);
  }
  friend Vertex operator*(const Vertex &ver, double mlt) {
    return Vertex(ver.x * mlt, ver.y * mlt);
  }
  friend Vertex operator/(const Vertex &ver, double mlt) {
    return Vertex(ver.x / mlt, ver.y / mlt);
  }
};

struct Map::Side {
  Color top; //Zaglushka
  sf::Image middle; //Zaglushka
  Color bottom; //Zaglushka
};



struct Map::Sector {
  short	lightlevel = 60;
  double floorheight = -64;
  double ceilingheight = 80;

  sf::Image floor; //Zaglushka
  sf::Image ceiling; //Zaglushka

  std::vector<Line*> lines;

  Sector() = default;
  explicit Sector(Map &map) {
    map.sectors.push_back(this);
  }

  ~Sector() {
    for (auto a: lines) {
      delete a;
    }
  }
};

struct Map::Line {
  Vertex v1, v2;
  std::shared_ptr<Side> side;
  Line *portal;
  Sector *sector;

  Line(): sector(nullptr), portal(nullptr), v1(Vertex()), v2(Vertex()), side(nullptr)  {}

  Line(Sector *sector, Vertex v1, Vertex v2, std::shared_ptr<Side> side):
          sector(sector), v1(v1), v2(v2), side(std::move(side)), portal(nullptr) {
    sector->lines.push_back(this);
  }
};
