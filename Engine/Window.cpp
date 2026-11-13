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

#ifdef __EMSCRIPTEN__
  // WebGL 2, the GLSL version is in Shader.cpp
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
  // Core profile: macOS has no compatibility one past 2.1
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create an OpenGL window");
  }

  glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
  // Uncapped, so the FPS counter measures the renderer, not the display.
  // Browsers always pace frames by requestAnimationFrame
  glfwSwapInterval(0);
#endif
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
