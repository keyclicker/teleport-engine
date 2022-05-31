#include <iostream> //todo for debug
#include <future>

#include "Game.hpp"
#include "Math.hpp"

void Game::gameLoop() {
  sf::Clock cl;

  while (bf.display()) {
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

    renderSector(player.pos, activeSector, clip);
  }
}

void Game::renderSector(const Vertex &pos, const Map::Sector *sec, 
                        const Clip &clip, Map::Line *portal) {
//  auto frc = std::thread(&Game::renderCeiling, this, sec, clip);
//  auto frf = std::thread(&Game::renderFloor, this, sec, clip);
//
//  frc.join();
//  frf.join();

  static int depth = 0;
  ++depth;
  if (depth > 10)  {
    --depth;
    return;
  }

  // floor
//  renderHorizontalPlane(pos, sec, clip, true);
  // ceiling
//  renderHorizontalPlane(pos, sec, clip, false);

  renderWalls(pos, sec, clip, portal);

  --depth;
}

void Game::renderHorizontalPlane(const Vertex &pos, const Map::Sector *sec, 
                                 const Clip &clip, bool floor) {
  auto rayDirL = player.dir - player.plane;
  auto rayDirR = player.dir + player.plane;

  // position of the floor/ceiling relative to the player
  auto posZ = floor 
  ? player.getHeight() - sec->floorheight + activeSector->floorheight
  : sec->ceilingheight - player.getHeight() - activeSector->floorheight;

  auto yStart = floor ? bf.getHeight() / 2 : 0;
  auto yEnd = floor ? bf.getHeight() : bf.getHeight() / 2;

  for (int y = yStart; y < yEnd; ++y) { 
    // distance from the center of the view on the screen plane
    auto p = floor 
    ? (y - bf.getHeight() / 2.0) / bf.getWidth()
    : (bf.getHeight() / 2.0 - y) / bf.getWidth();

    // distance from the view to the row
    auto rowDistance = posZ / p;

    auto floorStep = (rowDistance / bf.getWidth()) * (rayDirR - rayDirL);
    auto floorP = pos + rowDistance * rayDirL + floorStep * clip.left;

    auto &tex = floor ? sec->floor : sec->ceiling;

    auto d = deepFunc(rowDistance);

    for (int x = clip.left; x < clip.right; ++x) {
      auto k = (double) (x - clip.left) / (clip.right - clip.left);

      double start = bf.getHeight() / 2.0 - grad(clip.leftStart, clip.rightStart, k);
      double finish = bf.getHeight() / 2.0 + grad(clip.leftEnd, clip.rightEnd, k);

      if (y > start && y < finish)  {
        uint16_t ty = int(floorP.y * tex.getSize().y / 100.0) % tex.getSize().y;
        uint16_t tx = int(floorP.x * tex.getSize().x / 100.0) % tex.getSize().x;

        auto c = tex.getPixel(tx, ty);
        c.a = 255 * d;
        bf.setPixel(x, y, c);
      }

      floorP += floorStep;
    }
  }
}

void Game::renderWalls(const Vertex &pos, const Map::Sector *sec, 
                       const Game::Clip &clip, Map::Line *portal) {
  for (auto &a : sec->lines) {
    if (a == portal) continue;

    // v1 projection on view plane
    auto p1 = intersec(pos + player.dir,
                       pos + player.dir + player.plane, pos, a->v1);

    // v2 projection on view plane
    auto p2 = intersec(pos + player.dir,
                       pos + player.dir + player.plane, pos, a->v2);


    // calculating wall start and end on plane
    auto v1pl = (p1 - (pos + player.dir)) * player.plane / player.plane.length();
    auto v2pl = (p2 - (pos + player.dir)) * player.plane / player.plane.length();

    // calculating distances to the wall's start and end
    auto v1DistProj = (a->v1 - pos) * player.dir / player.dir.length();
    auto v2DistProj = (a->v2 - pos) * player.dir / player.dir.length();

    auto v1 = a->v1;
    auto v2 = a->v2;

    // if wall start and end are out of screen, casts two view rays
    if (v1pl < -1 || v1DistProj <= 0) {
      v1pl = -1;
      v1 = intersec(pos, pos + player.dir - player.plane, a->v1, a->v2);
      v1DistProj = (v1 - pos) * player.dir / player.dir.length();
    }

    if (v2pl > 1 || v2DistProj <= 0) {
      v2pl = 1;
      v2 = intersec(pos, pos + player.dir + player.plane, a->v1, a->v2);
      v2DistProj = (v2 - pos) * player.dir / player.dir.length();
    }

    if (v1pl > 1 || v2pl < -1) continue;

    // skips line behind view
    if (v1DistProj < 0 || v2DistProj < 0)
      continue;

    if (side(a->v1, a->v2, pos)) continue;

    int16_t leftCol = bf.getWidth() * ((v1pl + 1) / 2.0);
    int16_t rightCol = bf.getWidth() * ((v2pl + 1) / 2.0);

    int16_t fLeftCol = fit<int>(leftCol, clip.left, clip.right);
    int16_t fRightCol = fit<int>(rightCol, clip.left, clip.right);


    auto upper = a->sector->ceilingheight - player.getHeight() - activeSector->floorheight;
    auto bottom = player.getHeight() - a->sector->floorheight
            + activeSector->floorheight;

    double leftStart = (double) bf.getWidth() * upper / v1DistProj;
    double rightStart = (double) bf.getWidth() * upper / v2DistProj;

    double leftEnd = (double) bf.getWidth() * bottom / v1DistProj;
    double rightEnd = (double) bf.getWidth() * bottom / v2DistProj;

    if (a->portal) {
      //todo check
      auto dCeiling = bf.getWidth() * (sec->ceilingheight - a->portal->sector->ceilingheight - sec->floorheight);
      auto dFloor = bf.getWidth() * (a->portal->sector->floorheight - sec->floorheight);

      if (dCeiling < 0) dCeiling = 0;
      if (dFloor < 0) dFloor = 0;

      Clip cl{
              fLeftCol, fRightCol, fLeftCol, fRightCol,//todo test
              leftStart - dCeiling / v1DistProj,
              leftEnd - dFloor / v1DistProj,
              rightStart - dCeiling / v2DistProj,
              rightEnd - dFloor / v2DistProj
      };

      renderSector(pos + (a->portal->v2 - a->v1), a->portal->sector, cl, a->portal);
      //auto portalThr = std::async(&Game::renderSector, this, a->portal->sector, cl, a->portal);

      Clip clUpper{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              leftStart, -cl.leftStart, //todo why -
              rightStart, -cl.rightStart,
      };
      renderPlain(pos, sec, a, a->portal->side->upper, clUpper, clip);
      //auto upperThr = std::async(&Game::renderPlain, this, sec, a, a->portal->side->upper, clUpper);

      Clip clLower{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              -cl.leftEnd, leftEnd,
              -cl.rightEnd, rightEnd,
      };
      renderPlain(pos, sec, a, a->portal->side->lower, clLower, clip);
    }


    //todo
    if (!a->portal) {
      Clip cl{
              fLeftCol, fRightCol, leftCol, rightCol,//todo test
              leftStart, leftEnd, rightStart, rightEnd,
      };
      renderPlain(pos, sec, a, a->side->middle, cl, clip); //todo fix clips names
    }
  }
}

  void Game::renderPlain(const Vertex &pos, const Map::Sector *sec, 
                         const Map::Line *a, const sf::Image &texture, 
                         const Game::Clip &clip, const Game::Clip &fclip) {
    auto sp = pos + player.dir - player.plane;

    for (int i = clip.fLeft; i < clip.fRight; ++i) {
      auto k = (double) (i - clip.left) / (clip.right - clip.left);
      auto kClip = (double) (i - fclip.left) / (fclip.right - fclip.left);

      double start = bf.getHeight() / 2.0 - grad(clip.leftStart, clip.rightStart, k);
      double finish = bf.getHeight() / 2.0 + grad(clip.leftEnd, clip.rightEnd, k);

      double sClip = bf.getHeight() / 2.0 - grad(fclip.leftStart, fclip.rightStart, kClip);
      double fClip = bf.getHeight() / 2.0 + grad(fclip.leftEnd, fclip.rightEnd, kClip);

      //todo make this look less awful
      auto fstart = fit<double>(fit<double>(start, sClip, fClip), 0, bf.getHeight());
      auto ffinish = fit<double>(fit<double>(finish, sClip, fClip), 0, bf.getHeight());

      auto icp = sp + player.plane * (2.0 * i / bf.getWidth());
      auto its = intersec(pos, icp, a->v1, a->v2);
      auto tx = int((its - a->v1).length() * 0.5 * texture.getSize().y
                    / (sec->ceilingheight - sec->floorheight)) % texture.getSize().x; //todo fix aspect ratio and upper/lower

      auto d = deepFunc((its-pos).length());

      for (int j = fstart; j < ffinish; ++j) {
        auto ty = int(texture.getSize().y * (j - start) / (finish - start)) % texture.getSize().y;
        auto c = texture.getPixel(tx, ty);
        c.a = 255 * d;
        bf.setPixel(i, j, c);
      }
    }
  }

Map::Line *Game::collidedLine(const Vertex &pos) {
  for (auto &a: activeSector->lines) {
    auto d = dist(a->v1, a->v2, pos);

    double aa = a->v2.y - a->v1.y;
    double b = a->v1.x - a->v2.x;
    double c = - aa * a->v1.x - b * a->v1.y;

    double x = (b*(b * pos.x - aa * pos.y) - aa*c) / (aa*aa + b*b);
    double y = (aa*(-b * pos.x + aa * pos.y) - b*c) / (aa*aa + b*b);

    auto p1 = (std::min(a->v1.x, a->v2.x) <= x) && (std::max(a->v1.x, a->v2.x) >= x);
    auto p2 = (std::min(a->v1.y, a->v2.y) <= y) && (std::max(a->v1.y, a->v2.y) >= y);

    if ((p1 && p2) && ((d <= 20 && !a->portal) || (d <= 4 && a->portal))) { //todo think on better plane bug solution
      return a;
    }
  }
  return nullptr;
}

//todo move projection on colliding line
void Game::move(const Vertex &mv) {
  auto cl = collidedLine(player.pos + mv);
  if (!cl) {
    player.move(mv);
  }
  else if (cl->portal) {
    //todo think on better plane bug solution
    auto vcl = cl->v2 - cl->v1;
    auto per = Vertex(-vcl.y, vcl.x) / (vcl.length()/8.0);

    activeSector = cl->portal->sector;
    player.move(cl->portal->v2 - cl->v1);
    player.move(per + mv);
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
