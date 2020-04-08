#include "Game.hpp"

void Game::gameLoop() {
  while (bf.isOpen()) {
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
      player.rotate(0.001);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
      player.rotate(-0.001);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
      player.move(player.dir);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
      player.move(-player.dir);
    }

    renderSector(map.sectors.back(), player.pos);
  }
}

void Game::renderSector(const Map::Sector &sec, const Map::Vertex &pos) {
  for (auto &a : sec.lines) {
    //v1 projection on view plane
    auto p1 = intersec(pos + player.dir,
            pos + player.dir + player.plane, pos, a.v1);

    //v2 projection on view plane
    auto p2 = intersec(pos + player.dir,
            pos + player.dir + player.plane, pos, a.v2);

    auto v1pl = (pos + player.dir + player.plane - p1).length();
    auto v2pl = (pos + player.dir + player.plane - p2).length();

    if (v1pl < 0 || v1pl > 2 || v2pl < 0 || v2pl > 2) {
      int test;
      test = 0;
    }

    auto v1 = a.v1;
    auto v2 = a.v2;

    if (v1pl > v2pl) {
      std::swap(v1pl, v2pl);
      std::swap(v1, v2);
    }

    uint16_t leftCol = bf.getWidth() * ((v1pl) / 2.0);
    uint16_t rightCol = bf.getWidth() * ((v2pl) / 2.0);

    auto v1DistProj = (v1 - pos) * player.dir / player.dir.length();
    auto v2DistProj = (v2 - pos) * player.dir / player.dir.length();

    if (v1DistProj < 0) continue; //temp
    if (v2DistProj < 0) continue; //temp

    auto upper = a.sector->ceilingheight -
            a.sector->floorheight - player.height;
    auto bottom = player.height;

    auto leftUpper = upper / v1DistProj;
    auto rightUpper = upper / v2DistProj;

    auto leftBottom = bottom / v1DistProj;
    auto rightBottom = bottom / v2DistProj;


    for (int i = leftCol; i < rightCol; ++i) {
      auto k = (double) (i - leftCol) / (rightCol - leftCol);

      uint16_t start = bf.getHeight() *
              (1 - ((1 - k) * leftUpper + k * rightUpper)) / 2.0;

      uint16_t finish = bf.getHeight() *
              (1 + ((1 - k) * leftBottom + k * rightBottom)) / 2.0;

      start = fit(start, 0, bf.getHeight());
      finish = fit(finish, 0, bf.getHeight());

      for (int j = start; j < finish; ++j) {
        auto d = ((1 - k) * v1DistProj + k * v2DistProj) / 40.0;
        auto c = a.side.middle;
        bf.setPixel(i, j, c.r/d, c.g/d, c.b/d);
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

int Game::fit(int var, int a, int b) {
  if (var <= a) return a;
  if (var >= b) return b;
  return var;
}
