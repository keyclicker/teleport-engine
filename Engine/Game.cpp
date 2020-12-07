#include "Game.hpp"

#include <iostream> //todo for debug
#include <future>

void Game::gameLoop() {
  sf::Clock cl;

  while (bf.isOpen()) {
    auto ep = cl.restart().asSeconds();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ) {
      player.rotate(2 * ep);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
      player.rotate(-2 * ep);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad8)) {
      move(player.dir * (ep * 30));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad5)) {
      move(-player.dir * (ep * 30));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad4)) {
      move(-player.plane * (ep * 30));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad6)) {
      move(player.plane * (ep * 30));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad7)) {
      player.rotate(0.2 * ep);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad9)) {
      player.rotate(-0.2 * ep);
    }


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
      move(player.dir * (ep * 300));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
      move(-player.dir * (ep * 300));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
      move(-player.plane * (ep * 300));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
      move(player.plane * (ep * 300));
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) {
      player.setHeight(player.getHeight() + ep * 20);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
      player.setHeight(player.getHeight() - ep * 20);
    }


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
      player.shake(ep);
    }

    static auto d = 0.0;

    d += ep;
    if (d > 1) {
      d = 0;
      std::cout << 1.0/ep << std::endl;
    }

    Clip clip{};
    clip.left = 0;
    clip.right = bf.getWidth();
    clip.leftStart = clip.rightStart = bf.getHeight() / 2;
    clip.leftEnd = clip.rightEnd = bf.getHeight() / 2;

    renderSector(activeSector, clip);
  }
}

void Game::renderSector(const Map::Sector *sec, const Clip &clip, Map::Line *portal) {
//  auto frc = std::thread(&Game::renderCeiling, this, sec, clip);
//  auto frf = std::thread(&Game::renderFloor, this, sec, clip);
//
//  frc.join();
//  frf.join();

  renderCeiling(sec, clip);
  renderFloor(sec, clip);

  renderWalls(sec, clip, portal);
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

void Game::renderFloor(const Map::Sector *sec, const Game::Clip &clip) {
  auto rayDirL = player.dir - player.plane;
  auto rayDirR = player.dir + player.plane;

  auto posZ = player.getHeight() - sec->floorheight + activeSector->floorheight;

  for (int y = bf.getHeight() / 2; y < bf.getHeight(); ++y) {
    double p = (double)(y - bf.getHeight() / 2.0) / bf.getWidth();
    double rowDistance = (double) posZ / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL + floorStep * clip.left;

    auto &tex = sec->floor;

    for (int x = clip.left; x < clip.right; ++x) {
      auto k = (double) (x - clip.left) / (clip.right - clip.left);
      double finish = bf.getHeight() / 2.0 + grad(clip.leftEnd, clip.rightEnd, k);

      if (y > finish)  {
        floorP = floorP + floorStep;
        continue;
      }

      uint16_t ty = int(floorP.y * tex.getSize().y / 100) % tex.getSize().y;
      uint16_t tx = int(floorP.x * tex.getSize().x / 100) % tex.getSize().x;

      auto c = tex.getPixel(tx, ty);
      bf.setPixel(x, y, c.r, c.g, c.b);
      floorP = floorP + floorStep; // todo +=
    }
  }
}
void Game::renderCeiling(const Map::Sector *sec, const Game::Clip &clip) {
  auto rayDirL = player.dir - player.plane;
  auto rayDirR = player.dir + player.plane;

  auto posZCeil = sec->ceilingheight - sec->floorheight - player.getHeight() - activeSector->floorheight;

  for (int y = 0; y < bf.getHeight() / 2; ++y) {
    double p = ((double) bf.getHeight() / 2 - y) / bf.getWidth();
    double rowDistance = (double) posZCeil / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL + floorStep * clip.left;

    auto &tex = sec->ceiling;

    for (int x = clip.left; x < clip.right; ++x) {
      auto k = (double) (x - clip.left) / (clip.right - clip.left);
      double start = bf.getHeight() / 2.0 - grad(clip.leftStart, clip.rightStart, k);


      if (y < start)  {
        floorP = floorP + floorStep;
        continue;
      }

      uint16_t ty = int(floorP.y * tex.getSize().y / 100.0) % tex.getSize().y;
      uint16_t tx = int(floorP.x * tex.getSize().x / 100.0) % tex.getSize().x;

      auto c = tex.getPixel(tx, ty);
      bf.setPixel(x, y, c.r, c.g, c.b);
      floorP = floorP + floorStep; // todo +=
    }
  }
}

void Game::renderWalls(const Map::Sector *sec, const Game::Clip &clip, Map::Line *portal) {
  for (auto &a : sec->lines) {
    if (a == portal) continue;

    //v1 projection on view plane
    auto p1 = intersec(player.pos + player.dir,
                       player.pos + player.dir + player.plane, player.pos, a->v1);

    //v2 projection on view plane
    auto p2 = intersec(player.pos + player.dir,
                       player.pos + player.dir + player.plane, player.pos, a->v2);


    //calculating wall start and end on plane
    auto v1pl = (p1 - (player.pos + player.dir)) * player.plane / player.plane.length();
    auto v2pl = (p2 - (player.pos + player.dir)) * player.plane / player.plane.length();

    //calculating distances to the wall's start and end
    auto v1DistProj = (a->v1 - player.pos) * player.dir / player.dir.length();
    auto v2DistProj = (a->v2 - player.pos) * player.dir / player.dir.length();

    auto v1 = a->v1;
    auto v2 = a->v2;

    //if wall start and end are out of screen, casts two view rays
    if (v1pl < -1 || v1DistProj <= 0) {
      v1pl = -1;
      v1 = intersec(player.pos, player.pos + player.dir - player.plane, a->v1, a->v2);
      v1DistProj = (v1 - player.pos) * player.dir / player.dir.length();
    }

    if (v2pl > 1 || v2DistProj <= 0) {
      v2pl = 1;
      v2 = intersec(player.pos, player.pos + player.dir + player.plane, a->v1, a->v2);
      v2DistProj = (v2 - player.pos) * player.dir / player.dir.length();
    }

    if (v1pl > 1 || v2pl < -1) continue;

    //skips line behind view
    if (v1DistProj < 0 || v2DistProj < 0)
      continue;

    if (side(a->v1, a->v2, player.pos)) continue;

    double leftCol = bf.getWidth() * ((v1pl + 1) / 2.0);
    double rightCol = bf.getWidth() * ((v2pl + 1) / 2.0);

    int16_t fLeftCol = fit<int>(leftCol, clip.left, clip.right);
    int16_t fRightCol = fit<int>(rightCol, clip.left, clip.right);


    auto upper = a->sector->ceilingheight - a->sector->floorheight
                  - player.getHeight() - activeSector->floorheight;
    auto bottom = player.getHeight() - a->sector->floorheight
            + activeSector->floorheight;

    int16_t leftStart = bf.getWidth() * upper / v1DistProj;
    int16_t rightStart = bf.getWidth() * upper / v2DistProj;

    int16_t leftEnd = bf.getWidth() * bottom / v1DistProj;
    int16_t rightEnd = bf.getWidth() * bottom / v2DistProj;

    if (a->portal) {
      //todo check
      auto dCeiling = bf.getWidth() * (sec->ceilingheight - a->portal->sector->ceilingheight +
                                       a->portal->sector->floorheight - sec->floorheight);
      auto dFloor = bf.getWidth() * (a->portal->sector->floorheight - sec->floorheight);


      Clip cl{
              fLeftCol, fRightCol, fLeftCol, fRightCol,//todo test
              int16_t(leftStart - dCeiling / v1DistProj),
              int16_t(leftEnd - dFloor / v1DistProj),
              int16_t(rightStart - dCeiling / v2DistProj),
              int16_t(rightEnd - dFloor / v2DistProj)
      };

      renderSector(a->portal->sector, cl, a->portal);
      //auto portalThr = std::async(&Game::renderSector, this, a->portal->sector, cl, a->portal);

      Clip clUpper{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              leftStart, -cl.leftStart, //todo why -
              rightStart, -cl.rightStart,
      };
      renderPlain(sec, a, a->portal->side->upper, clUpper);
      //auto upperThr = std::async(&Game::renderPlain, this, sec, a, a->portal->side->upper, clUpper);

      Clip clLower{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              -cl.leftEnd, leftEnd,
              -cl.rightEnd, rightEnd,
      };
      renderPlain(sec, a, a->portal->side->lower, clLower);
    }


    //todo
    if (!a->portal) {
      Clip cl{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              leftStart, leftEnd, rightStart, rightEnd,
      };
      renderPlain(sec, a, a->side->middle, cl);
    }
  }
}

  void Game::renderPlain(const Map::Sector *sec,
                         const Map::Line *a, const sf::Image &texture, const Game::Clip &clip) {
    auto sp = player.pos + player.dir - player.plane;

    for (int i = clip.fLeft; i < clip.fRight; ++i) {
      auto k = (double) (i - clip.left) / (clip.right - clip.left);

      double start = bf.getHeight() / 2.0 - grad(clip.leftStart, clip.rightStart, k);
      double finish = bf.getHeight() / 2.0 + grad(clip.leftEnd, clip.rightEnd, k);

      auto fstart = fit<int>(start, 0, bf.getHeight());
      auto ffinish = fit<int>(finish, 0, bf.getHeight());

      auto d = 1;//((1 - k) * v1DistProj + k * v2DistProj) / 80.0;

      auto icp = sp + player.plane * (2.0 * i / bf.getWidth());
      auto its = intersec(player.pos, icp, a->v1, a->v2);
      auto tx = int((its - a->v1).length() * texture.getSize().x
                    / (sec->ceilingheight - sec->floorheight)) % texture.getSize().x; //todo fix aspect ratio and upper/lower

      for (int j = fstart; j < ffinish; ++j) {
        auto ty = int(texture.getSize().y * (j - start) / (finish - start)) % texture.getSize().y;
        auto c = texture.getPixel(tx, ty);
        bf.setPixel(i, j, fit(c.r / d, 0, 255),
                    fit(c.g / d, 0, 255),
                    fit(c.b / d, 0, 255));
      }
    }
  }

Map::Line *Game::collidedLine(const Map::Vertex &pos) {
  for (auto &a: activeSector->lines) {
    auto d = dist(a->v1, a->v2, pos);
    if ((d <= 20 && !a->portal) || (d < 0 && a->portal)) {
      return a;
    }
  }
  return nullptr;
}

bool Game::side(const Map::Vertex &v1, const Map::Vertex &v2, const Map::Vertex &v3) {
  double a = v2.y - v1.y;
  double b = v1.x - v2.x;
  double c = - a * v1.x - b * v1.y;

  return a * v3.x + b * v3.y + c < 0;
}

double Game::dist(const Map::Vertex &v1, const Map::Vertex &v2, const Map::Vertex &v3) {
  double a = v2.y - v1.y;
  double b = v1.x - v2.x;
  double c = - a * v1.x - b * v1.y;

  return (a * v3.x + b * v3.y + c) / std::hypot(a, b);
}

//todo move projection on colliding line
void Game::move(const Map::Vertex &mv) {
  auto cl = collidedLine(player.pos + mv);
  if (!cl) {
    player.move(mv);
  }
  else if (cl->portal) {
    player.move(mv);
    activeSector = cl->portal->sector;
  }
  else {
    auto vcl = cl->v2 - cl->v1;
    auto vclen = vcl.length();
    auto mproj = vcl * ((mv * vcl) / (vclen * vclen));

    if (!collidedLine(player.pos + mproj)) {
      player.move(mproj);
    }
  }

}

