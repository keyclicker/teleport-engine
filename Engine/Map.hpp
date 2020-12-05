#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <cstdint>

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

  std::list<Sector> sectors;
};

struct Map::Vertex {
  double x, y;

  Vertex() = default;
  Vertex(double x, double y) : x(x), y(y) {}

  double length() {
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


struct Map::Line {
  Vertex v1, v2;
  Side side;
  Line *portal;
  Sector *sector;
};

struct Map::Sector {
  short	lightlevel = 60;
  int floorheight = -64;
  int ceilingheight = 80;

  Color floor; //Zaglushka
  Color ceiling; //Zaglushka

  std::list<Line> lines;
};