#include "Engine.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <format>
#include <numeric>
#include <stdexcept>
#include <string_view>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace te {

// Longest step handed to the app; slower frames run the game slower
static constexpr float MAX_DT = 1.0f / 20;

Engine::Engine(const EngineConfig &config):
  config(config),
  window(config.width, config.height, config.title.c_str()),
  input(window),
  pool(config.threads),
  renderer(pool) {
  setRenderScale(config.renderScale);
}

void Engine::setRenderScale(int scale) {
  config.renderScale = std::clamp(scale, 1, 8);
}

// ---- Main loop -----------------------------------------------------------

void Engine::run(App &game) {
  app = &game;
  lastFrame = Clock::now();

#ifdef __EMSCRIPTEN__
  // The browser owns the loop: one frame per requestAnimationFrame
  emscripten_set_main_loop_arg(
    [](void *engine) { static_cast<Engine *>(engine)->frameStep(); },
    this, 0, true);
#else
  while (window.isOpen()) {
    frameStep();
  }
#endif
}

void Engine::frameStep() {
  input.beginFrame();
  window.pollEvents();

  auto now = Clock::now();
  float dt = std::chrono::duration<float>(now - lastFrame).count();
  lastFrame = now;

  fps.tick(dt);
  handleEngineKeys();

  bool benchmarking = config.benchFrames > 0 && benchResult.empty();

  // The benchmark watches the start, so the world stays still
  if (!benchmarking) app->update(*this, std::min(dt, MAX_DT));

  // ---- 3D view ----

  auto [width, height] = window.getFramebufferSize();
  int frameWidth = std::max(1, width / config.renderScale);
  int frameHeight = std::max(1, height / config.renderScale);

  if (frameWidth != frame.getWidth() || frameHeight != frame.getHeight()) {
    frame.resize(frameWidth, frameHeight);
  }

  Camera camera = app->getCamera();

  if (benchmarking) {
    camera.angle += radians(360.0f) * benchTimes.size() / config.benchFrames;
  }

  auto renderStart = Clock::now();
  renderer.render(camera, frame);
  renderMs = std::chrono::duration<double, std::milli>(
    Clock::now() - renderStart).count();

  if (benchmarking) {
    benchTimes.push_back(renderMs);
    if (int(benchTimes.size()) == config.benchFrames) finishBenchmark();
  }

  // ---- HUD ----

  Hud hud = {Canvas(frame), text, width, height,
             float(std::max(1, height / 360))};

  app->drawHud(*this, hud);
  screen.present(frame, width, height);

  if (showStats) drawStats(hud);
  text.render(width, height);

  window.display();
}

void Engine::handleEngineKeys() {
  // Clicks capture the mouse, see Input. Browsers release pointer lock
  // on Esc themselves
  if (input.isPressed(GLFW_KEY_ESCAPE)) input.release();

  if (input.isPressed(GLFW_KEY_F1)) showStats = !showStats;
  if (input.isPressed(GLFW_KEY_F2)) {
    setRenderScale(config.renderScale % 4 + 1);
  }
  if (input.isPressed(GLFW_KEY_F11)) window.toggleFullscreen();
}

void Engine::drawStats(Hud &hud) {
  auto stats = std::format(
    "{:.0f} FPS  {:.1f} ms  render {:.1f} ms\n{}x{} /{}  {} threads",
    fps.getFps(), fps.getFrameTime(), renderMs,
    frame.getWidth(), frame.getHeight(), config.renderScale, pool.size());

  if (!benchResult.empty()) stats += "\n" + benchResult;

  float scale = hud.textScale;
  hud.text.print(stats, 4 * scale, 4 * scale, scale, {255, 255, 0});
}

// ---- Benchmark -----------------------------------------------------------

void Engine::finishBenchmark() {
  auto sorted = benchTimes;
  std::sort(sorted.begin(), sorted.end());

  double average = std::accumulate(sorted.begin(), sorted.end(), 0.0)
                   / sorted.size();

  benchResult = std::format(
    "bench {}x{} {}t: avg {:.2f} ms  p50 {:.2f}  p95 {:.2f}",
    frame.getWidth(), frame.getHeight(), pool.size(), average,
    sorted[sorted.size() / 2], sorted[sorted.size() * 95 / 100]);

  std::printf("%s\n", benchResult.c_str());
  std::fflush(stdout);

#ifndef __EMSCRIPTEN__
  glfwSetWindowShouldClose(window.handle(), true);
#endif
}

// ---- Arguments -----------------------------------------------------------

void parseArgs(int argc, char **argv, EngineConfig &config) {
  auto number = [&](int &i) {
    if (i + 1 >= argc) {
      throw std::runtime_error(std::string(argv[i]) + " needs a number");
    }

    return std::stoi(argv[++i]);
  };

  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];

    if (arg == "--bench") {
      bool hasCount = i + 1 < argc && std::isdigit(argv[i + 1][0]);
      config.benchFrames = hasCount ? number(i) : 360;
    } else if (arg == "--size") {
      if (i + 1 >= argc ||
          std::sscanf(argv[++i], "%dx%d", &config.width, &config.height) != 2) {
        throw std::runtime_error("--size needs WIDTHxHEIGHT");
      }
    } else if (arg == "--scale") {
      config.renderScale = number(i);
    } else if (arg == "--threads") {
      config.threads = std::max(1, number(i));
    } else {
      throw std::runtime_error("Unknown argument " + std::string(arg));
    }
  }
}

} // namespace te
