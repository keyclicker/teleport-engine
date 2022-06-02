#pragma once

#include <cmath>
#include "Vector.hpp"
  
/**
 * @brief Returns intersection of the lines
 * 
 * @param v1 First vertex of the first line
 * @param v2 Second vertex of the first line
 * @param v3 First vertex of the second line
 * @param v4 Second vertex of the second line
 * @return Vertex Intersection
 */
Vector intersec(const Vector &v1, const Vector &v2,
                const Vector &v3, const Vector &v4);

/**
 * @brief Returns side of the line where vertex is located
 * 
 * @param lv1 First vertex of the line
 * @param lv2 Second vertex of the line
 * @param v Vertex
 * @return true Vertex is on the positive side of the line
 * @return false Vertex is on the negative side of the line
 */
bool side(const Vector &lv1, const Vector &lv2,
          const Vector &v);

/**
 * @brief Distant from the line to the vertex
 * 
 * @param lv1 First vertex of the line
 * @param lv2 Second vertex of the line
 * @param v Vertex
 * @return double 
 */
double dist(const Vector &lv1, const Vector &lv2,
            const Vector &v);

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
T interpolate(T a, T b, double k) {
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