#pragma once

#include <cmath>

/**
 * @brief Vertex/Vector representation
 * 
 * @remark Moved from Map class, and maybe should be moved back
 */
struct Vertex {
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

  Vertex &operator+=(const Vertex &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  Vertex &operator-=(const Vertex &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  Vertex &operator*=(double mlt) {
    x *= mlt;
    y *= mlt;
    return *this;
  }

  Vertex &operator/=(double mlt) {
    x /= mlt;
    y /= mlt;
    return *this;
  }

  Vertex &rotate(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    double x = this->x * c - this->y * s;
    double y = this->x * s + this->y * c;
    this->x = x;
    this->y = y;
    return *this;
  }
};
  
  
/**
 * @brief Returns intersection of the lines
 * 
 * @param v1 First vertex of the first line
 * @param v2 Second vertex of the first line
 * @param v3 First vertex of the second line
 * @param v4 Second vertex of the second line
 * @return Vertex Intersection
 */
Vertex intersec(const Vertex &v1, const Vertex &v2,
                        const Vertex &v3, const Vertex &v4);

/**
 * @brief Returns side of the line where vertex is located
 * 
 * @param lv1 First vertex of the line
 * @param lv2 Second vertex of the line
 * @param v Vertex
 * @return true Vertex is on the positive side of the line
 * @return false Vertex is on the negative side of the line
 */
bool side(const Vertex &lv1, const Vertex &lv2, 
                  const Vertex &v);

/**
 * @brief Distant from the line to the vertex
 * 
 * @param lv1 First vertex of the line
 * @param lv2 Second vertex of the line
 * @param v Vertex
 * @return double 
 */
double dist(const Vertex &lv1, const Vertex &lv2, 
                   const Vertex &v);

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
double grad(T a, T b, double k) {
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
T fit(T value, T a, T b) {
  if (value < a)
    return a;
  if (value > b)
    return b;
  return value;
}