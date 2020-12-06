#include "Game.hpp"

#include "iostream" //todo for debug

void Game::gameLoop() {
  sf::Clock cl;

  while (bf.isOpen()) {
    auto ep = cl.restart().asSeconds();

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
      player.rotate(1 * ep);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
      player.rotate(-1 * ep);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
      player.move(player.dir * (ep * 1000));
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
      player.move(-player.dir * (ep * 1000));
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
      player.move(-player.plane * (ep * 1000));
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
      player.move(player.plane * (ep * 1000));
    }

    static auto d = 0.0;

    d += ep;
    if (d > 1) {
      d = 0;
      std::cout << 1.0/ep << std::endl;
    }

    for (int i = 0; i < bf.getWidth(); ++i) {
      for (int j = 0; j < bf.getHeight() / 2; ++j) {
        bf.setPixel(i, j, 25, 25, 25);
      }
      for (int j = bf.getHeight() / 2; j < bf.getHeight(); ++j) {
        bf.setPixel(i, j, 50, 50, 50);
      }
    }
    renderSector(map.sectors.back());
  }
}

void Game::renderSector(const Map::Sector &sec) {
  for (auto &a : sec.lines) {
    //v1 projection on view plane
    auto p1 = intersec(player.pos + player.dir,
                       player.pos + player.dir + player.plane, player.pos, a.v1);

    //v2 projection on view plane
    auto p2 = intersec(player.pos + player.dir,
                       player.pos + player.dir + player.plane, player.pos, a.v2);


    //calculating wall start and end on plane
    auto v1pl = (p1 - (player.pos + player.dir)) * player.plane / player.plane.length();
    auto v2pl = (p2 - (player.pos + player.dir)) * player.plane / player.plane.length();

    //calculating distances to the wall's start and end
    auto v1DistProj = (a.v1 - player.pos) * player.dir / player.dir.length();
    auto v2DistProj = (a.v2 - player.pos) * player.dir / player.dir.length();

    auto v1 = a.v1;
    auto v2 = a.v2;

    //if wall start and end are out of screen, casts two view rays
    if (v1pl < -1 || v1DistProj < 0) {
      v1pl = -1;
      v1 = intersec(player.pos, player.pos + player.dir - player.plane, a.v1, a.v2);
      v1DistProj = (v1 - player.pos) * player.dir / player.dir.length();
    }

    if (v2pl > 1 || v2DistProj < 0) {
      v2pl = 1;
      v2 = intersec(player.pos, player.pos + player.dir + player.plane, a.v1, a.v2);
      v2DistProj = (v2 - player.pos) * player.dir / player.dir.length();
    }

    if (v1pl > 1 || v2pl < -1) continue;

    //skips line behind view
    if (v1DistProj < 0 || v2DistProj < 0)
      continue;

    //for safety
    if (v1pl > v2pl) {
      std::swap(v1pl, v2pl); //todo check
      std::swap(v1, v2);
    }


    int16_t leftCol = bf.getWidth() * ((v1pl + 1) / 2.0);
    int16_t rightCol = bf.getWidth() * ((v2pl + 1) / 2.0);


    auto upper = a.sector->ceilingheight -
                 a.sector->floorheight - player.height;
    auto bottom = player.height;

    auto leftUpper = upper / v1DistProj;
    auto rightUpper = upper / v2DistProj;

    auto leftBottom = bottom / v1DistProj;
    auto rightBottom = bottom / v2DistProj;

    //
    auto sp = player.pos + player.dir - player.plane;

    for (int i = leftCol; i < rightCol; ++i) {
      auto k = (double) (i - leftCol) / (rightCol - leftCol);

      int16_t start = bf.getHeight() *
                      (1 - ((1 - k) * leftUpper + k * rightUpper)) / 2.0;

      int16_t finish = bf.getHeight() *
                       (1 + ((1 - k) * leftBottom + k * rightBottom)) / 2.0;

      auto fstart = fit(start, 0, bf.getHeight());
      auto ffinish = fit(finish, 0, bf.getHeight());

      auto d = ((1 - k) * v1DistProj + k * v2DistProj) / 80.0;

      auto icp = sp + player.plane * (2.0 * i / bf.getWidth());
      auto its = intersec(player.pos, icp, v1, v2);
      auto tx = (its - a.v1).length() * 5.0;

      for (int j = fstart; j < ffinish; ++j) {
        auto ty = a.side.middle.getSize().y * (j-start)/(finish-start);
        auto c = a.side.middle.getPixel(tx, ty);
        bf.setPixel(i, j, fit(c.r/d, 0, 255),
                    fit(c.g/d, 0, 255),
                    fit(c.b/d, 0, 255));
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
  if (var < a)
    return a;
  if (var > b)
    return b;
  return var;
}
