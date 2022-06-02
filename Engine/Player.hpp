#pragma once

#include <cmath>

#include "Map.hpp"
#include "Utils/Math.hpp"


/**
 * @brief Player representation
 * 
 */
class Player {
private:
  double realHeight = 28;
  double height = 28;
  double shakeState = 0;
  double shakeSpeed = 2;
  double shakeD = 5;

public:
  // todo make getters and setters
  Vector pos = {0, 0};
  Vector dir = {1, 0};
  Vector plane = {0, -1};


  /**
   * @brief Rotate player
   * 
   * @param angle Rotation angle in radians
   */
  void rotate(double angle) {
    dir.rotate(angle);
    plane.rotate(angle);
  }

  /**
   * @brief Sets position of the player
   * 
   * @param d Position
   */
  void setPos(const Vector &d) {
    pos = pos + d;
  }

  /**
   * @brief Moves player
   * 
   * @param dx Movement in x direction
   * @param dy Movement in y direction
   */
  void move(double dx, double dy) {
    pos.x += dx;
    pos.y += dy;
  }

  /**
   * @brief  Moves player by vector
   * 
   * @param d Movement vector
   */
  void move(const Vector &d) {
    move(d.x, d.y);
  }

  /**
   * @brief Changes player height
   * 
   * @param t Time passed since last frame in seconds
   */
  void shake(double t) {
    shakeState += 2.0 * M_PI * shakeSpeed * t;
    height = realHeight + shakeD * std::sin(shakeState);
  }


  /**
   * @brief Get the Height of the player
   * 
   * @return double Height
   */
  double getHeight() const {
    return height;
  }

  /**
   * @brief Set the Height of the player
   * 
   * @param height Height
   */
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