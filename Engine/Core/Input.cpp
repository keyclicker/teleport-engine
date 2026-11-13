#include "Input.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

namespace te {

Input &Input::from(GLFWwindow *window) {
  return *static_cast<Input *>(glfwGetWindowUserPointer(window));
}

Input::Input(Window &w): window(w.handle()) {
  glfwSetWindowUserPointer(window, this);

  glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int, int action,
                                int) {
    if (key < 0 || key >= KEYS) return;

    auto &input = from(win);
    if (action == GLFW_PRESS) input.keysPressed[key] = true;
    if (action != GLFW_REPEAT) input.keysDown[key] = action == GLFW_PRESS;
  });

  glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button,
                                        int action, int) {
    if (button < 0 || button >= BUTTONS) return;

    auto &input = from(win);
    if (action == GLFW_PRESS) input.buttonsPressed[button] = true;
    input.buttonsDown[button] = action == GLFW_PRESS;
  });

#ifdef __EMSCRIPTEN__
  // Pointer lock reports movement deltas, the cursor position freezes
  emscripten_set_mousemove_callback(
    EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, false,
    [](int, const EmscriptenMouseEvent *event, void *data) {
      auto &input = *static_cast<Input *>(data);

      if (input.isCaptured()) {
        input.motion += Vec2(event->movementX, event->movementY);
      }

      return false;
    });
#else
  glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
    auto &input = from(win);

    if (input.captured && input.hasLastCursor) {
      input.motion += Vec2(float(x - input.lastX), float(y - input.lastY));
    }

    input.lastX = x;
    input.lastY = y;
    input.hasLastCursor = true;
  });

  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }
#endif
}

void Input::beginFrame() {
  keysPressed.fill(false);
  buttonsPressed.fill(false);
  motion = {};
}

bool Input::isCaptured() const {
#ifdef __EMSCRIPTEN__
  // The browser may drop the lock on its own, e.g. on Esc
  EmscriptenPointerlockChangeEvent status;
  emscripten_get_pointerlock_status(&status);
  return status.isActive;
#else
  return captured;
#endif
}

void Input::capture() {
#ifdef __EMSCRIPTEN__
  emscripten_request_pointerlock("#canvas", true);
#else
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  captured = true;
  hasLastCursor = false;
#endif
}

void Input::release() {
#ifdef __EMSCRIPTEN__
  emscripten_exit_pointerlock();
#else
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  captured = false;
#endif
}

} // namespace te
