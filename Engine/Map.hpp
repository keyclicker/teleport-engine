#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

//temporary
struct Color {
  uint8_t r, g, b;

  Color() {}
  Color(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
};

class Map {
public:
  struct Vertex;
  struct Line;
  struct Side;
  struct Sector;

  std::vector<Sector> sectors;
};

struct Map::Vertex {
  double x, y;

  Vertex() = default;
  Vertex(double x, double y) : x(x), y(y) {}

  double length() {
    return std::hypot(x, y);
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
    return Vertex(ver.x * mlt, ver.x * mlt);
  }
  friend Vertex operator*(const Vertex &ver, double mlt) {
    return Vertex(ver.x * mlt, ver.x * mlt);
  }
  friend Vertex operator/(const Vertex &ver, double mlt) {
    return Vertex(ver.x / mlt, ver.x / mlt);
  }
};

struct Map::Side {
  Color top; //Zaglushka
  Color middle; //Zaglushka
  Color bottom; //Zaglushka
};


struct Map::Line {
  Vertex v1, v2;
  Side side;
  std::vector<Line>::iterator portal;
  std::vector<Sector>::iterator sector;
};

struct Map::Sector {
  short	lightlevel = 60;
  int floorheight = -64;
  int ceilingheight = 80;

  Color floor; //Zaglushka
  Color ceiling; //Zaglushka

  std::vector<Line> lines;
};