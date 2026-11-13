#pragma once

// The web gets OpenGL ES 3.0 (WebGL 2), desktops get epoxy's loader.
// Either way GLFW must not pull its own GL header
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <epoxy/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <utility>

namespace te {

/**
 * @brief GLFW window with a current OpenGL 3.3 core context
 *
 * On the web it's the `#canvas` element with an OpenGL ES 3.0 (WebGL 2)
 * context, sized to its CSS box. Construct it before anything that
 * creates GL objects, destroy it after.
 */
class Window {
private:
  GLFWwindow *window;

#ifndef __EMSCRIPTEN__
  // Windowed geometry to restore when leaving fullscreen
  int windowedX = 0, windowedY = 0, windowedW = 0, windowedH = 0;
#endif

public:
  Window(const Window &rhs) = delete;
  Window &operator=(const Window &rhs) = delete;

  /**
   * @brief Open a window and make its GL context current
   *
   * @param width Window width
   * @param height Window height
   * @param title Window title
   * @throw std::runtime_error GLFW or the GL context failed to start
   */
  Window(int width, int height, const char *title);

  ~Window();

  /**
   * @brief Process pending window events, call once per frame
   */
  void pollEvents();

  /**
   * @brief Show the frame drawn into the default framebuffer
   */
  void display() {
    glfwSwapBuffers(window);
  }

  /**
   * @brief Whether the window wasn't asked to close
   */
  bool isOpen() const {
    return !glfwWindowShouldClose(window);
  }

  /**
   * @brief Switch between fullscreen and windowed
   */
  void toggleFullscreen();

  /**
   * @brief Framebuffer size in pixels, differs from window size on HiDPI
   *
   * @return std::pair<int, int> Width and height
   */
  std::pair<int, int> getFramebufferSize() const;

  /**
   * @brief The GLFW window, for Input
   */
  GLFWwindow *handle() const {
    return window;
  }
};

} // namespace te
