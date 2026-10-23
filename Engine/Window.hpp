#pragma once

// epoxy provides the GL headers, GLFW must not pull its own
#include <epoxy/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <utility>

/**
 * @brief GLFW window with a current OpenGL 3.3 core context
 *
 * Construct it before anything that creates GL objects, destroy it after.
 */
class Window {
private:
  GLFWwindow *window;

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
  void pollEvents() {
    glfwPollEvents();
  }

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
   * @brief Whether a key is held down
   *
   * @param key GLFW key code, e.g. `GLFW_KEY_W`
   */
  bool isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
  }

  /**
   * @brief Framebuffer size in pixels, differs from window size on HiDPI
   *
   * @return std::pair<int, int> Width and height
   */
  std::pair<int, int> getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return {width, height};
  }
};
