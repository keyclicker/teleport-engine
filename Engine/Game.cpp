#include "Game.hpp"

void Game::gameLoop() {
  while (bf.isOpen()) {
    renderSector(map.sectors.back(), player.pos);
  }
}

void Game::renderSector(const Map::Sector &sec, const Map::Vertex &pos) {
  for (auto &a : sec.lines) {
    uint16_t leftCol, rightCol; //Column
    double leftDist, rightDist;

    //v1 projection on view plane
    auto p1 = intersec(pos + player.dir,
            pos + player.dir + player.plane, pos, a.v1);

    //v2 projection on view plane
    auto p2 = intersec(pos + player.dir,
            pos + player.dir + player.plane, pos, a.v2);

    auto v1pl = (pos + player.dir - p1).length();
    auto v2pl = (pos + player.dir - p2).length();

    auto v1 = a.v1;
    auto v2 = a.v2;

    if (v1pl > v2pl) {
      std::swap(v1pl, v2pl);
      std::swap(v1, v2);
    }

    leftCol = bf.getWidth() * ((v1pl - 1.0) / 2.0);
    rightCol = bf.getWidth() * ((v2pl - 1.0) / 2.0);

    auto v1DistProj = (v1 - pos) * player.dir / player.dir.length();
    auto v2DistProj = (v2 - pos) * player.dir / player.dir.length();

    if (v1DistProj < 0) continue; //temp
    if (v2DistProj < 0) continue; //temp

    auto upper = a.sector->ceilingheight - player.height;
    auto bottom = player.height - a.sector->floorheight;

    auto leftUpper = upper / v1DistProj;
    auto rightUpper = upper / v2DistProj;

    auto leftBottom = bottom / v1DistProj;
    auto rightBottom = bottom / v2DistProj;

    for (int i = leftCol; i < rightCol; ++i) {
      auto k = (double) (i - leftCol) / (rightCol - leftCol);

      auto start = bf.getHeight() *
              (((1 - k) * leftUpper - k * rightUpper) - 1) / 2.0;

      auto finish = bf.getHeight() *
              (((1 - k) * leftBottom - k * rightBottom) - 1) / 2.0;



      for (int j = start; j < finish; ++j) {
        auto c = a.side.middle;
        bf.setPixel(i, j, c.r, c.g, c.b);
      }

    }

  }
}

Map::Vertex Game::intersec(const Map::Vertex &v1, const Map::Vertex &v2,
                           const Map::Vertex &v3, const Map::Vertex &v4) {
  double a1 = v2.y - v1.y;
  double b1 = v1.x - v2.x;
  double c1 = a1*(v1.x) + b1*(v1.y);

  double a2 = v4.y - v3.y;
  double b2 = v3.x - v4.x;
  double c2 = a2*(v3.x) + b2*(v3.y);

  double determinant = a1*b2 - a2*b1;

  if (determinant == 0) {
    return Map::Vertex(std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max());
  }
  else {
    double x = (b2*c1 - b1*c2)/determinant;
    double y = (a1*c2 - a2*c1)/determinant;
    return Map::Vertex(x, y);
  }
}

bool Game::isVertexOnSeg(const Map::Vertex &v,
        const Map::Vertex &sv1, const Map::Vertex &sv2) {
  return v.x > std::min(sv1.x, sv2.x) &&
         v.x < std::max(sv1.x, sv2.x) &&
         v.y > std::min(sv1.y, sv2.y) &&
         v.y < std::max(sv1.y, sv2.y);
}
