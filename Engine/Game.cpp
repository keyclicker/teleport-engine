#include <iostream> //todo for debug
#include <vector>
#include <cmath>

#include "Game.hpp"
#include "Utils/Math.hpp"
#include "Renderer.hpp"

void Game::gameLoop() {
  sf::Clock cl;
  double speed = 1;

  while (renderer.bf.display()) {
    auto ep = cl.restart().asSeconds() * speed;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LBracket)) {
      speed /= pow(1.25, ep);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::RBracket)) {
      speed *= pow(1.25, ep);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ) {
      player.rotate(2 * ep);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
      player.rotate(-2 * ep);
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

    renderer.renderFrame();
  }
}


Map::Line *Game::collidedLine(const Vector &pos) {
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
void Game::move(const Vector &mv) {
  auto cl = collidedLine(player.pos + mv);
  if (!cl) {
    player.move(mv);
  }
  else if (cl->portal) {
    //todo think on better plane bug solution
    auto vcl = cl->v2 - cl->v1;
    auto per = Vector(-vcl.y, vcl.x) / (vcl.length() / 20.0);

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
