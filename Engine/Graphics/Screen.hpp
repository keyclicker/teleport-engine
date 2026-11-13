#pragma once

#include "../Core/Window.hpp"
#include "Frame.hpp"

namespace te {

/**
 * @brief Shows a Frame by stretching it over the framebuffer
 *
 * The frame goes through one GL texture, nearest-filtered, so a frame
 * smaller than the window looks pixelated rather than blurry. Needs a
 * current GL context for its whole lifetime, see Window.
 */
class Screen {
private:
  GLuint texture = 0;
  GLuint program = 0;
  GLuint vao = 0;

  // Texture storage size, in frame pixels
  int textureWidth = 0, textureHeight = 0;

public:
  Screen(const Screen &rhs) = delete;
  Screen &operator=(const Screen &rhs) = delete;

  /**
   * @throw std::runtime_error The present shader failed to build
   */
  Screen();

  ~Screen();

  /**
   * @brief Draw the frame over the whole viewport
   *
   * Call Window::display() afterwards to show it.
   *
   * @param frame Frame to show
   * @param viewportWidth Framebuffer width in pixels
   * @param viewportHeight Framebuffer height in pixels
   */
  void present(const Frame &frame, int viewportWidth, int viewportHeight);
};

} // namespace te
