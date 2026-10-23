#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <format>

#include "Game.hpp"
#include "Utils/Math.hpp"
#include "Renderer.hpp"

void Game::gameLoop() {
  using Clock = std::chrono::steady_clock;

  auto lastFrame = Clock::now();
  double speed = 1;

  while (window.isOpen()) {
    window.pollEvents();

    auto now = Clock::now();
    auto dt = std::chrono::duration<double>(now - lastFrame).count();
    auto ep = dt * speed;
    lastFrame = now;

    if (window.isKeyPressed(GLFW_KEY_LEFT_BRACKET)) {
      speed /= pow(1.25, ep);
    }
    if (window.isKeyPressed(GLFW_KEY_RIGHT_BRACKET)) {
      speed *= pow(1.25, ep);
    }

    if (window.isKeyPressed(GLFW_KEY_LEFT) ||
       window.isKeyPressed(GLFW_KEY_Q) ) {
      player.rotate(2 * ep);
    }
    if (window.isKeyPressed(GLFW_KEY_RIGHT) ||
       window.isKeyPressed(GLFW_KEY_E)) {
      player.rotate(-2 * ep);
    }

    if (window.isKeyPressed(GLFW_KEY_W)) {
      move(player.dir * (ep * 300));
    }
    if (window.isKeyPressed(GLFW_KEY_S)) {
      move(-player.dir * (ep * 300));
    }
    if (window.isKeyPressed(GLFW_KEY_A)) {
      move(-player.plane * (ep * 300));
    }
    if (window.isKeyPressed(GLFW_KEY_D)) {
      move(player.plane * (ep * 300));
    }

    if (window.isKeyPressed(GLFW_KEY_Z)) {
      player.setHeight(player.getHeight() + ep * 20);
    }
    if (window.isKeyPressed(GLFW_KEY_X)) {
      player.setHeight(player.getHeight() - ep * 20);
    }

    if (window.isKeyPressed(GLFW_KEY_W) ||
        window.isKeyPressed(GLFW_KEY_S) ||
        window.isKeyPressed(GLFW_KEY_A) ||
        window.isKeyPressed(GLFW_KEY_D)) {
      player.shake(ep);
    }

    renderer.renderFrame();
    fps.tick(dt);

    auto [width, height] = window.getFramebufferSize();
    renderer.bf.present(width, height);
    drawHud(width, height);
    window.display();
  }
}

void Game::drawHud(int width, int height) {
  // 3 at 1080p: stays readable as the window grows
  float scale = std::max(1, height / 360);

  auto stats = std::format("{:.0f} FPS  {:.1f} ms",
                           fps.getFps(), fps.getFrameTime());

  text.print(stats, 4 * scale, 4 * scale, scale, {255, 255, 0});
  text.render(width, height);
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
