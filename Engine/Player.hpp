#pragma once

#include "Map.hpp"
#include <cmath>

class Player {
private:
  double realHeight = 28;
  double height = 28;
  double shakeState = 0;
  double shakeSpeed = 2;
  double shakeD = 5;

public:
  Map::Vertex pos = {0, 0};
  Map::Vertex dir = {1, 0};
  Map::Vertex plane = {0, -1};

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
    move(d.x, d.y);
  }

  void shake(double sectime) {
    shakeState += 2.0 * M_PI * shakeSpeed * sectime;
    height = realHeight + shakeD * std::sin(shakeState);
  }


  double getHeight() const {
    return height;
  }
  void setHeight(double height) {
    Player::height = height;
    Player::realHeight = height;
  }

  double getShakeState() const {
    return shakeState;
  }
  void setShakeState(double shakeState) {
    Player::shakeState = shakeState;
  }

  double getShakeSpeed() const {
    return shakeSpeed;
  }
  void setShakeSpeed(double shakeSpeed) {
    Player::shakeSpeed = shakeSpeed;
  }

  double getShakeD() const {
    return shakeD;
  }
  void setShakeD(double shakeD) {
    Player::shakeD = shakeD;
  }
};