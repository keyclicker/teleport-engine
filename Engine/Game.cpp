#include "Game.hpp"

#include "iostream" //todo for debug

void Game::gameLoop() {
  sf::Clock cl;

  while (bf.isOpen()) {
    auto ep = cl.restart().asSeconds();

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ) {
      player.rotate(2 * ep);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
      player.rotate(-2 * ep);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
      player.move(player.dir * (ep * 300));
      player.shake(ep);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
      player.move(-player.dir * (ep * 300));
      player.shake(ep);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
      player.move(-player.plane * (ep * 300));
      player.shake(ep);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
      player.move(player.plane * (ep * 300));
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

    renderSector(map.sectors.front(), clip);
  }
}

void Game::renderSector(const Map::Sector *sec, const Clip &clip, Map::Line *portal) {
  renderFloorCeiling(sec, clip);
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

void Game::renderFloorCeiling(const Map::Sector *sec, const Game::Clip &clip) {
  auto rayDirL = player.dir - player.plane;
  auto rayDirR = player.dir + player.plane;

  auto posZ = player.height - sec->floorheight;
  auto posZCeil = sec->ceilingheight - sec->floorheight - player.height;

  for (int y = 0; y < bf.getHeight() / 2; ++y) {
    double p = ((double) bf.getHeight() / 2 - y) / bf.getWidth();
    double rowDistance = (double) posZCeil / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL + floorStep * clip.left;

    auto &tex = sec->ceiling;

    for (int x = clip.left; x < clip.right; ++x) {
      auto k = (double) (x - clip.left) / (clip.right - clip.left);
      double start = bf.getHeight() / 2.0 - ((1 - k) * clip.leftStart + k * clip.rightStart);

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

  for (int y = bf.getHeight() / 2; y < bf.getHeight(); ++y) {
    double p = (double)(y - bf.getHeight() / 2.0) / bf.getWidth();
    double rowDistance = (double) posZ / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL + floorStep * clip.left;

    auto &tex = sec->floor;

    for (int x = clip.left; x < clip.right; ++x) {
      auto k = (double) (x - clip.left) / (clip.right - clip.left);
      double finish = bf.getHeight() / 2.0 + ((1 - k) * clip.leftEnd + k * clip.rightEnd);

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

    //todo test
    if (v1pl > v2pl) {
      //std::swap(v1pl, v2pl); //todo check
      //std::swap(v1, v2);
    }

    int16_t leftCol = fit<int>(bf.getWidth() * ((v1pl + 1) / 2.0), clip.left, clip.right);
    int16_t rightCol = fit<int>(bf.getWidth() * ((v2pl + 1) / 2.0), clip.left, clip.right);

    auto upper = a->sector->ceilingheight -
                 a->sector->floorheight - player.height;
    auto bottom = player.height - a->sector->floorheight;

    auto leftStart = bf.getWidth() * upper / v1DistProj;
    auto rightStart = bf.getWidth() * upper / v2DistProj;

    auto leftEnd = bf.getWidth() * bottom / v1DistProj;
    auto rightEnd = bf.getWidth() * bottom / v2DistProj;

    if (a->portal) {
      Clip cl{};
      cl.left = leftCol;
      cl.right = rightCol;

      //todo check
      auto dCeiling = bf.getWidth() * (sec->ceilingheight - a->portal->sector->ceilingheight +
                                          a->portal->sector->floorheight - sec->floorheight);
      auto dFloor = bf.getWidth() * (a->portal->sector->floorheight - sec->floorheight);

      cl.leftStart = leftStart - dCeiling / v1DistProj;
      cl.leftEnd = leftEnd - dFloor / v1DistProj;
      cl.rightStart = rightStart - dCeiling / v2DistProj;
      cl.rightEnd = rightEnd - dFloor / v2DistProj;

      renderSector(a->portal->sector, cl, a->portal);
    }

    //
    auto sp = player.pos + player.dir - player.plane;

    //todo
    if (!a->portal) {
      for (int i = leftCol; i < rightCol; ++i) {
        auto k = (double) (i - leftCol) / (rightCol - leftCol);

        double start = bf.getHeight() / 2.0 - ((1 - k) * leftStart + k * rightStart);
        double finish = bf.getHeight() / 2.0 + ((1 - k) * leftEnd + k * rightEnd);

        auto fstart = fit<int>(start, 0, bf.getHeight());
        auto ffinish = fit<int>(finish, 0, bf.getHeight());

        auto d = 1;//((1 - k) * v1DistProj + k * v2DistProj) / 80.0;


        auto icp = sp + player.plane * (2.0 * i / bf.getWidth());
        auto its = intersec(player.pos, icp, v1, v2);
        auto tx = int((its - a->v1).length() * a->side->middle.getSize().x
                      / (sec->ceilingheight - sec->floorheight)) % a->side->middle.getSize().x; //todo fix aspect ratio

        auto &texture  = a->side->middle;

        for (int j = fstart; j < ffinish; ++j) {
          auto ty = int(texture.getSize().y * (j-start)/(finish-start)) % texture.getSize().y;
          auto c = texture.getPixel(tx, ty);
          bf.setPixel(i, j, fit(c.r/d, 0, 255),
                      fit(c.g/d, 0, 255),
                      fit(c.b/d, 0, 255));
        }
      }
    }
  }
}



