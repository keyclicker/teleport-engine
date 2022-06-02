#pragma once

#include <cmath>

/**
 * @brief Vertex/Vector representation
 *
 * @remark Moved from Map class, and maybe should be moved back
 */
struct Vector {
  double x, y;

  Vector() = default;
  Vector(double x, double y) : x(x), y(y) {}

  double length() const {
    return std::hypot(x, y);
  }

  Vector operator-() const {
    return Vector(-x, -y);
  }

  double operator*(const Vector &rhs) const {
    return x * rhs.x + y * rhs.y;
  }
  Vector operator+(const Vector &rhs) const {
    return Vector(x + rhs.x, y + rhs.y);
  }
  Vector operator-(const Vector &rhs) const {
    return Vector(x - rhs.x, y - rhs.y);
  }

  friend Vector operator*(double mlt, const Vector &ver) {
    return Vector(ver.x * mlt, ver.y * mlt);
  }
  friend Vector operator*(const Vector &ver, double mlt) {
    return Vector(ver.x * mlt, ver.y * mlt);
  }
  friend Vector operator/(const Vector &ver, double mlt) {
    return Vector(ver.x / mlt, ver.y / mlt);
  }

  Vector &operator+=(const Vector &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  Vector &operator-=(const Vector &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  Vector &operator*=(double mlt) {
    x *= mlt;
    y *= mlt;
    return *this;
  }

  Vector &operator/=(double mlt) {
    x /= mlt;
    y /= mlt;
    return *this;
  }

  double cross(const Vector &rhs) const {
    return x * rhs.y - y * rhs.x;
  }

  void rotate(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    double ox = x;
    x = ox * c - y * s;
    y = ox * s + y * c;
  }

  // dir length is 1
  void rotateToDir(Vector dir) {
    double c = dir.x;
    double s = 1 / dir.y;
    double ox = x;
    x = ox * c - y * s;
    y = ox * s + y * c;
  }

  Vector rotatedToDir(Vector dir) const {
    double c = dir.y;
    double s = dir.x;
    return Vector(x * c - y * s, x * s + y * c);
  }
};