#pragma once

#include <cmath>
#include <numbers>

namespace te {

/**
 * @brief 2D vector: map positions, directions and offsets
 *
 * The map is y-up: angle 0 faces +x, angles grow counterclockwise.
 */
struct Vec2 {
  float x = 0, y = 0;

  constexpr Vec2() = default;
  constexpr Vec2(float x, float y): x(x), y(y) {}

  /**
   * @brief Unit vector at an angle
   *
   * @param angle Radians, 0 is +x, counterclockwise
   */
  static Vec2 fromAngle(float angle) {
    return {std::cos(angle), std::sin(angle)};
  }

  constexpr Vec2 operator-() const { return {-x, -y}; }
  constexpr Vec2 operator+(Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
  constexpr Vec2 operator-(Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }
  constexpr Vec2 operator*(float k) const { return {x * k, y * k}; }
  constexpr Vec2 operator/(float k) const { return {x / k, y / k}; }

  constexpr friend Vec2 operator*(float k, Vec2 v) { return v * k; }

  constexpr Vec2 &operator+=(Vec2 rhs) { return *this = *this + rhs; }
  constexpr Vec2 &operator-=(Vec2 rhs) { return *this = *this - rhs; }
  constexpr Vec2 &operator*=(float k) { return *this = *this * k; }

  constexpr float dot(Vec2 rhs) const { return x * rhs.x + y * rhs.y; }

  /**
   * @brief z of the 3D cross product: > 0 when `rhs` is to the left
   */
  constexpr float cross(Vec2 rhs) const { return x * rhs.y - y * rhs.x; }

  float length() const { return std::hypot(x, y); }
  constexpr float lengthSquared() const { return dot(*this); }

  /**
   * @brief Same direction, length 1; the zero vector stays zero
   */
  Vec2 normalized() const {
    float len = length();
    return len > 0 ? *this / len : Vec2{};
  }

  /**
   * @brief Angle in radians, see fromAngle()
   */
  float angle() const { return std::atan2(y, x); }

  /**
   * @brief Rotated 90 degrees clockwise: the right hand of a y-up
   *        direction
   */
  constexpr Vec2 right() const { return {y, -x}; }
};

/**
 * @brief Rotation by an angle, stored as its cosine and sine
 */
struct Rotation {
  float cos = 1, sin = 0;

  static Rotation fromAngle(float angle) {
    return {std::cos(angle), std::sin(angle)};
  }

  /**
   * @brief Rotation that turns direction `from` into direction `to`
   */
  static Rotation between(Vec2 from, Vec2 to) {
    return fromAngle(to.angle() - from.angle());
  }

  float angle() const { return std::atan2(sin, cos); }

  constexpr Vec2 apply(Vec2 v) const {
    return {v.x * cos - v.y * sin, v.x * sin + v.y * cos};
  }

  constexpr Rotation inverse() const { return {cos, -sin}; }
};

/**
 * @brief Rigid 2D transform: rotate, then translate
 *
 * Portals use it to map one sector's space into another's.
 */
struct Transform {
  Rotation rotation;
  Vec2 offset;

  constexpr Vec2 apply(Vec2 point) const {
    return rotation.apply(point) + offset;
  }

  /**
   * @brief Rotate a direction, ignoring the offset
   */
  constexpr Vec2 applyDir(Vec2 dir) const {
    return rotation.apply(dir);
  }

  constexpr Transform inverse() const {
    auto inv = rotation.inverse();
    return {inv, -inv.apply(offset)};
  }
};

/**
 * @brief Linear interpolation, `k = 0` is `a`, `k = 1` is `b`
 */
template <typename T>
constexpr T lerp(T a, T b, float k) {
  return a + (b - a) * k;
}

/**
 * @brief Closest point to `p` on segment `a`-`b`
 */
inline Vec2 closestOnSegment(Vec2 a, Vec2 b, Vec2 p) {
  Vec2 ab = b - a;
  float len2 = ab.lengthSquared();

  if (len2 == 0) return a;

  float t = (p - a).dot(ab) / len2;
  return a + ab * std::fmax(0.0f, std::fmin(1.0f, t));
}

constexpr float PI = std::numbers::pi_v<float>;

/**
 * @brief Degrees to radians
 */
constexpr float radians(float degrees) {
  return degrees * PI / 180;
}

} // namespace te
