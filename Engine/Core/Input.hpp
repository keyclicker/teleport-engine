#pragma once

#include <array>

#include "../Math/Vec2.hpp"
#include "Window.hpp"

namespace te {

/**
 * @brief Keyboard and mouse state for the current frame
 *
 * Keys and buttons are GLFW codes: `GLFW_KEY_W`, `GLFW_MOUSE_BUTTON_LEFT`.
 * "Pressed" means it went down since the previous frame. Mouse motion
 * is reported only while the mouse is captured (pointer lock on the
 * web), so it can steer a camera. A click captures it; that click
 * isn't reported.
 */
class Input {
private:
  static constexpr int KEYS = GLFW_KEY_LAST + 1;
  static constexpr int BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;

  GLFWwindow *window;

  std::array<bool, KEYS> keysDown{}, keysPressed{};
  std::array<bool, BUTTONS> buttonsDown{}, buttonsPressed{};

  // Accumulated since beginFrame()
  Vec2 motion;

#ifndef __EMSCRIPTEN__
  // Last cursor position: motion is its difference. The web gets
  // motion and capture state from the browser instead
  double lastX = 0, lastY = 0;
  bool hasLastCursor = false;

  bool captured = false;
#endif

  static Input &from(GLFWwindow *window);

public:
  Input(const Input &rhs) = delete;
  Input &operator=(const Input &rhs) = delete;

  /**
   * @brief Start listening to a window's events
   */
  explicit Input(Window &window);

  /**
   * @brief Forget this frame's presses and motion, call before
   *        Window::pollEvents()
   */
  void beginFrame();

  bool isDown(int key) const { return keysDown[key]; }
  bool isPressed(int key) const { return keysPressed[key]; }

  bool isButtonDown(int button) const { return buttonsDown[button]; }
  bool isButtonPressed(int button) const { return buttonsPressed[button]; }

  /**
   * @brief Mouse movement this frame in screen pixels, y goes down
   */
  Vec2 getMotion() const { return motion; }

  /**
   * @brief Whether the cursor is hidden and locked to the window
   */
  bool isCaptured() const;

  /**
   * @brief Hide and lock the cursor; on the web, takes effect on the
   *        next click (browsers allow it only inside input events)
   */
  void capture();

  /**
   * @brief Show the cursor again
   */
  void release();
};

} // namespace te
