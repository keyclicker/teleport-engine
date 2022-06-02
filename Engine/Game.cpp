#include <iostream> //todo for debug
#include <future>
#include <vector>

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

    vClip = std::vector<Segment<>>{bf.getWidth(),
                     Segment<>{-1, 1}};

    auto clip = Clip(this);

    renderSector(player.pos, activeSector, clip);
  }
}

void Game::renderSector(const Vertex &pos, const Map::Sector *sec, 
                        const Clip &clip, Map::Line *portal) {

  static int depth = 0;
  ++depth;
  if (depth < 10)  {
    renderWalls(pos, sec, clip, portal);
  }
  --depth;
}



void Game::renderWalls(const Vertex &pos, const Map::Sector *sec, 
                       const Game::Clip &clip, Map::Line *portal) {

  auto a = new Map::Line((Map::Sector*) sec, {100, 100}, {100, -100}, sec->lines[0]->side);
//  for (auto &a : sec->lines) {
    // transform and rotate the line
    auto v1 = (a->v1 - pos).rotatedToDir(player.dir);
    auto v2 = (a->v2 - pos).rotatedToDir(player.dir);

    if (v1.x > v2.x) {
      std::swap(v1, v2);
    }

    Segment<> vSeg = {
        v1.x / v1.y,
        v2.x / v2.y
    };

    Segment<> lSeg = {
        -200.0 / v1.y,
        200.0 / v1.y
    };

    Segment<> rSeg = {
        -200.0 / v2.y,
        200.0 / v2.y
    };



    renderPlain({vSeg, lSeg, rSeg}, clip);
//  }
}

void Game::renderPlain(Plain plain, Clip clip) {
  // Horizontal segment of visible part of the wall
  auto hSeg = hToScreen(clip.hClamp(plain.hSeg));

  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    // Calculating k for interpolation
    auto k = (double) (xToView(x) - plain.hSeg.begin)
        / (plain.hSeg.end - plain.hSeg.begin);

    Segment vSeg = vToScreen(clip.vClamp(x, {
      interpolate(plain.lSeg.begin, plain.rSeg.begin, k),
      interpolate(plain.lSeg.end, plain.rSeg.end, k)
    }));

    for (uint16_t y = vSeg.begin; y < vSeg.end; ++y) {
      bf.setPixel(x, y, sf::Color::Red);
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
