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

    renderSector(map.sectors.front());
  }
}

void Game::renderSector(const Map::Sector *sec) {

  auto rayDirL = player.dir - player.plane;
  auto rayDirR = player.dir + player.plane;

  auto posZ = player.height;
  auto posZCeil = sec->ceilingheight - player.height ;

  for (int y = 0; y < bf.getHeight() / 2; ++y) {
    double p = (double)(bf.getHeight() / 2 - y) / bf.getWidth();
    double rowDistance = (double) posZCeil / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL;

    auto &tex = sec->ceiling;

    for (int x = 0; x < bf.getWidth(); ++x) {
      int ty = int(floorP.y * tex.getSize().y / 100) % tex.getSize().y;
      int tx = int(floorP.x * tex.getSize().x / 100) % tex.getSize().x;

      auto c = tex.getPixel(tx, ty);
      bf.setPixel(x, y, c.r, c.g, c.b);
      floorP = floorP + floorStep; // todo +=
    }
  }

  for (int y = bf.getHeight() / 2; y < bf.getHeight(); ++y) {
    double p = (double)(y - bf.getHeight() / 2) / bf.getWidth();
    double rowDistance = (double) posZ / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = player.pos + rowDistance * rayDirL;

    auto &tex = sec->floor;

    for (int x = 0; x < bf.getWidth(); ++x) {
      int ty = int(floorP.y * tex.getSize().y / 100) % tex.getSize().y;
      int tx = int(floorP.x * tex.getSize().x / 100) % tex.getSize().x;

      auto c = tex.getPixel(tx, ty);
      bf.setPixel(x, y, c.r, c.g, c.b);
      floorP = floorP + floorStep; // todo +=
    }
  }


  for (auto &a : sec->lines) {
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
      continue;
    }

    int16_t leftCol = bf.getWidth() * ((v1pl + 1) / 2.0);
    int16_t rightCol = bf.getWidth() * ((v2pl + 1) / 2.0);

    auto upper = a->sector->ceilingheight -
                 a->sector->floorheight - player.height;
    auto bottom = player.height;

    auto leftUpper = 2.0 * upper / v1DistProj; //2.0 - is 2 * plain.len / dir.len
    auto rightUpper = 2.0 * upper / v2DistProj;

    auto leftBottom = 2.0 * bottom / v1DistProj;
    auto rightBottom = 2.0 * bottom / v2DistProj;

    if (a->portal) {
      renderSector(a->portal->sector);
    }

    //
    auto sp = player.pos + player.dir - player.plane;

    //todo
    if (!a->portal) {
      for (int i = leftCol; i < rightCol; ++i) {
        auto k = (double) (i - leftCol) / (rightCol - leftCol);

        double start = (bf.getHeight() - bf.getWidth()*((1 - k) * leftUpper + k * rightUpper)) / 2.0;

        double finish = (bf.getHeight() + bf.getWidth()*((1 - k) * leftBottom + k * rightBottom)) / 2.0;

        auto fstart = fit(start, 0, bf.getHeight());
        auto ffinish = fit(finish, 0, bf.getHeight());

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
