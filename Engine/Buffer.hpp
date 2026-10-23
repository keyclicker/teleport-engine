#pragma once

#include <cstdint>

#include "Image.hpp"
#include "Window.hpp"

/**
 * @brief Screen canvas: a CPU frame presented through an OpenGL texture
 *
 * Needs a current GL context for its whole lifetime, see Window.
 */
class Buffer {
private:
  Image frame;

  GLuint texture = 0;
  GLuint program = 0;
  GLuint vao = 0;

public:
  Buffer(const Buffer &rhs) = delete;
  Buffer &operator=(const Buffer &rhs) = delete;

  /**
   * @brief Construct a new Buffer object
   *
   * @param width Frame width
   * @param height Frame height
   * @throw std::runtime_error The present shader failed to build
   */
  Buffer(uint16_t width, uint16_t height);

  ~Buffer();

  /**
   * @brief Draw pixel with specified color
   *
   * @param x x coordinate
   * @param y y coordinate
   * @param color Color, its alpha is brightness
   */
  void setPixel(uint16_t x, uint16_t y, Color color) {
    frame.setPixel(x, y, color);
  }

  Color getPixel(uint16_t x, uint16_t y) const {
    return frame.getPixel(x, y);
  }

  /**
   * @brief Draw the frame over the whole viewport, then clear it
   *
   * Call Window::display() afterwards to show it.
   *
   * @param viewportWidth Target framebuffer width in pixels
   * @param viewportHeight Target framebuffer height in pixels
   */
  void present(int viewportWidth, int viewportHeight);

  /**
   * @brief Get frame width
   *
   * @return uint16_t Frame width
   */
  uint16_t getWidth() const {
    return frame.getWidth();
  }

  /**
   * @brief Get frame height
   *
   * @return uint16_t Frame height
   */
  uint16_t getHeight() const {
    return frame.getHeight();
  }
};
