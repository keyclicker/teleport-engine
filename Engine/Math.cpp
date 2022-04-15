#include <limits>

#include "Math.hpp"

Vertex intersec(const Vertex &v1, const Vertex &v2,
                const Vertex &v3, const Vertex &v4) {
  double a1 = v2.y - v1.y;
  double b1 = v1.x - v2.x;
  double c1 = a1*(v1.x) + b1*(v1.y);

  double a2 = v4.y - v3.y;
  double b2 = v3.x - v4.x;
  double c2 = a2*(v3.x) + b2*(v3.y);

  double determinant = a1*b2 - a2*b1;

  if (determinant == 0) {
    return Vertex(std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max());
  }
  else {
    double x = (b2*c1 - b1*c2)/determinant;
    double y = (a1*c2 - a2*c1)/determinant;
    return Vertex(x, y);
  }
}

bool side(const Vertex &lv1, const Vertex &lv2, 
          const Vertex &v) {
  double a = lv2.y - lv1.y;
  double b = lv1.x - lv2.x;
  double c = - a * lv1.x - b * lv1.y;

  return a * v.x + b * v.y + c < 0;
}

double dist(const Vertex &lv1, const Vertex &lv2, 
            const Vertex &v) {
  double a = lv2.y - lv1.y;
  double b = lv1.x - lv2.x;
  double c = - a * lv1.x - b * lv1.y;

  return (a * v.x + b * v.y + c) / std::hypot(a, b);
}