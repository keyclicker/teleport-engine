#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "../Graphics/Canvas.hpp"
#include "../Graphics/Frame.hpp"
#include "../Graphics/Screen.hpp"
#include "../Graphics/TextRenderer.hpp"
#include "../Graphics/Texture.hpp"
#include "../Render/Renderer.hpp"
#include "FpsCounter.hpp"
#include "Input.hpp"
#include "ThreadPool.hpp"
#include "Window.hpp"

namespace te {

class Engine;

/**
 * @brief What a game draws over the 3D view each frame
 *
 * Two layers with their own units: `canvas` paints into the
 * low-resolution frame (pixel-art HUDs, weapons), `text` prints crisp
 * text in framebuffer pixels on top.
 */
struct Hud {
  Canvas canvas;
  TextRenderer &text;

  // Framebuffer size, the units of `text`
  int screenWidth, screenHeight;

  // Readable text size for this screen, see TextRenderer::print()
  float textScale;
};

/**
 * @brief A game: the engine calls it every frame
 */
class App {
public:
  virtual ~App() = default;

  /**
   * @brief Advance the game
   *
   * @param dt Seconds since the last frame, at most 1/20
   */
  virtual void update(Engine &engine, float dt) = 0;

  /**
   * @brief Where to render the 3D view from this frame
   */
  virtual Camera getCamera() const = 0;

  /**
   * @brief Draw the HUD over the 3D view
   */
  virtual void drawHud(Engine &engine, Hud &hud) {
    (void) engine;
    (void) hud;
  }
};

struct EngineConfig {
  std::string title = "Teleport Engine";

  // Window size; the web uses the canvas's size instead
  int width = 1280, height = 720;

  // Screen pixels per rendered pixel, F2 cycles it
  int renderScale = 2;

  // Threads that render, see ThreadPool
  int threads = ThreadPool::defaultSize();

  // Benchmark: render this many frames turning in place, print the
  // timings and quit (0: play normally)
  int benchFrames = 0;
};

/**
 * @brief Window, input, renderer and main loop
 *
 * Engine keys: click captures the mouse, Esc releases it, F1 toggles
 * stats, F2 cycles the render scale, F11 toggles fullscreen.
 */
class Engine {
private:
  using Clock = std::chrono::steady_clock;

  EngineConfig config;

  // The window owns the GL context, so it goes first
  Window window;
  Input input;
  Screen screen;
  TextRenderer text;

  ThreadPool pool;
  Renderer renderer;
  Frame frame;

  App *app = nullptr;

  Clock::time_point lastFrame;
  FpsCounter fps;
  double renderMs = 0;

  bool showStats = true;

  // Render time per benchmark frame, in milliseconds
  std::vector<double> benchTimes;
  std::string benchResult;

  void frameStep();
  void handleEngineKeys();
  void drawStats(Hud &hud);
  void finishBenchmark();

public:
  Engine(const Engine &rhs) = delete;
  Engine &operator=(const Engine &rhs) = delete;

  /**
   * @throw std::runtime_error The window or GL failed to start
   */
  explicit Engine(const EngineConfig &config);

  /**
   * @brief Run the main loop until the window closes
   *
   * On the web it never returns and the browser drives frames, so the
   * Engine and the App must outlive the caller's scope (`static`).
   */
  void run(App &app);

  Window &getWindow() { return window; }
  Input &getInput() { return input; }
  Renderer &getRenderer() { return renderer; }

  int getRenderScale() const { return config.renderScale; }

  /**
   * @param scale Screen pixels per rendered pixel, 1 to 8
   */
  void setRenderScale(int scale);
};

/**
 * @brief Parse `--bench [frames]`, `--size WxH`, `--scale N`,
 *        `--threads N` into a config
 *
 * @throw std::runtime_error Unknown or malformed argument
 */
void parseArgs(int argc, char **argv, EngineConfig &config);

} // namespace te
