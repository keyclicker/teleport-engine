#include "Window.hpp"

#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, const char *title) {
  glfwSetErrorCallback([](int code, const char *description) {
    std::cerr << "GLFW error " << code << ": " << description << std::endl;
  });

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  // Core profile: macOS has no compatibility one past 2.1
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create an OpenGL 3.3 window");
  }

  glfwMakeContextCurrent(window);

  // Uncapped, so the FPS counter measures the renderer, not the display
  glfwSwapInterval(0);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
