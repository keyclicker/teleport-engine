#include "Window.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

namespace te {

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

void Window::pollEvents() {
  glfwPollEvents();

#ifdef __EMSCRIPTEN__
  // Follow the canvas's CSS box, in device pixels, so it stays sharp
  double cssWidth, cssHeight;
  emscripten_get_element_css_size("#canvas", &cssWidth, &cssHeight);

  double ratio = emscripten_get_device_pixel_ratio();
  int width = int(std::lround(cssWidth * ratio));
  int height = int(std::lround(cssHeight * ratio));

  auto [current, currentHeight] = getFramebufferSize();

  if (width > 0 && height > 0 &&
      (width != current || height != currentHeight)) {
    emscripten_set_canvas_element_size("#canvas", width, height);
  }
#endif
}

void Window::toggleFullscreen() {
#ifdef __EMSCRIPTEN__
  EmscriptenFullscreenChangeEvent status;
  emscripten_get_fullscreen_status(&status);

  if (status.isFullscreen) {
    emscripten_exit_fullscreen();
  } else {
    // Deferred: browsers allow it only inside an input event handler
    emscripten_request_fullscreen("#canvas", true);
  }
#else
  if (glfwGetWindowMonitor(window)) {
    glfwSetWindowMonitor(window, nullptr, windowedX, windowedY,
                         windowedW, windowedH, GLFW_DONT_CARE);
    return;
  }

  glfwGetWindowPos(window, &windowedX, &windowedY);
  glfwGetWindowSize(window, &windowedW, &windowedH);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                       mode->refreshRate);
#endif
}

std::pair<int, int> Window::getFramebufferSize() const {
  int width, height;

#ifdef __EMSCRIPTEN__
  // GLFW's own idea lags behind canvas resizes made above
  emscripten_get_canvas_element_size("#canvas", &width, &height);
#else
  glfwGetFramebufferSize(window, &width, &height);
#endif

  return {width, height};
}

} // namespace te
