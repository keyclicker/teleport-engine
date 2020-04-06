#pragma once

#include "Map.hpp"
#include <cmath>

class Player {
public:
  Map::Vertex pos;
  Map::Vertex dir = {1, 0};
  Map::Vertex plane = {0, 1};
  int height = 56;

  void rotate(double angle) {
    auto oldDir = dir;
    dir.x = dir.x * std::cos(angle) - dir.y * std::sin(angle);
    dir.y = oldDir.x * std::sin(angle) + dir.y * std::cos(angle);

    auto oldPlane = plane;
    plane.x = plane.x * std::cos(angle) - plane.y * std::sin(angle);
    plane.y = oldPlane.x * std::sin(angle) + plane.y * std::cos(angle);
  }

  void move(double dx, double dy) {
    pos.x += dx;
    pos.y += dy;
  }

  void move(Map::Vertex d) {
    pos.x += d.x;
    pos.y += d.y;
  }
};