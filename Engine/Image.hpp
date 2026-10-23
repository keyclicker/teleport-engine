#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief RGBA8 color, laid out as GL_RGBA / GL_UNSIGNED_BYTE expects
 *
 * Alpha holds brightness: the screen multiplies rgb by it on present.
 */
struct Color {
  uint8_t r = 0, g = 0, b = 0, a = 255;

  /**
   * @brief Add channel by channel, saturating at 255
   */
  Color operator+(const Color &rhs) const {
    auto add = [](uint8_t x, uint8_t y) {
      return uint8_t(std::min(x + y, 255));
    };

    return {add(r, rhs.r), add(g, rhs.g), add(b, rhs.b), add(a, rhs.a)};
  }
};

/**
 * @brief Row-major RGBA8 bitmap, top row first
 *
 * Holds both textures and the screen frame.
 */
class Image {
private:
  uint32_t width = 0, height = 0;
  std::vector<Color> pixels;

public:
  Image() = default;

  /**
   * @brief Construct an image filled with one color
   *
   * @param width Image width
   * @param height Image height
   * @param color Fill color, opaque black by default
   */
  Image(uint32_t width, uint32_t height, Color color = {}):
    width(width), height(height), pixels(size_t(width) * height, color) {}

  /**
   * @brief Load a PNG, JPEG or GIF (first frame) file
   *
   * On failure prints the reason to stderr and leaves the image as is.
   *
   * @param path File path
   * @return true The image is loaded
   * @return false The file is missing or its format is unsupported
   */
  bool loadFromFile(const std::string &path);

  /**
   * @brief Fill the whole image with one color
   */
  void fill(Color color) {
    std::fill(pixels.begin(), pixels.end(), color);
  }

  /**
   * @brief Set pixel color, coordinates are not checked
   */
  void setPixel(uint32_t x, uint32_t y, Color color) {
    pixels[size_t(y) * width + x] = color;
  }

  /**
   * @brief Get pixel color, coordinates are not checked
   */
  Color getPixel(uint32_t x, uint32_t y) const {
    return pixels[size_t(y) * width + x];
  }

  /**
   * @brief Raw pixels, `width * height` of them
   */
  const Color *data() const {
    return pixels.data();
  }

  uint32_t getWidth() const {
    return width;
  }

  uint32_t getHeight() const {
    return height;
  }
};
