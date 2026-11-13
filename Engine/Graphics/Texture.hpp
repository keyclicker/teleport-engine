#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace te {

/**
 * @brief RGBA8 color, laid out as GL_RGBA / GL_UNSIGNED_BYTE expects
 *
 * In a Frame, alpha holds brightness: the screen multiplies rgb by it.
 * In a Texture, alpha is coverage: sprites skip texels below 128.
 */
struct Color {
  uint8_t r = 0, g = 0, b = 0, a = 255;

  /**
   * @brief Same color with another alpha
   */
  constexpr Color withAlpha(uint8_t alpha) const {
    return {r, g, b, alpha};
  }
};

/**
 * @brief Image stored column by column, for vertical span rendering
 *
 * Walls and sprites read whole columns, so columns are contiguous.
 * Coordinates wrap: textures tile in both directions.
 */
class Texture {
private:
  int width = 0, height = 0;

  // `size - 1` for power-of-two sizes, so wrapping is a mask; 0 otherwise
  uint32_t widthMask = 0, heightMask = 0;

  // Column-major: texel (u, v) is at `u * height + v`
  std::vector<Color> texels;

  static int wrap(int i, int size, uint32_t mask) {
    if (mask) return int(uint32_t(i) & mask);

    int m = i % size;
    return m < 0 ? m + size : m;
  }

public:
  Texture() = default;

  /**
   * @brief Construct a texture filled with one color
   */
  Texture(int width, int height, Color color = {});

  /**
   * @brief Decode a PNG, JPEG or GIF (first frame) file
   *
   * @param path File path
   * @throw std::runtime_error The file is missing or unsupported
   */
  static Texture load(const std::string &path);

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  /**
   * @brief Texels of column `u`, wrapped, `getHeight()` of them
   */
  const Color *column(int u) const {
    return texels.data() + size_t(wrap(u, width, widthMask)) * height;
  }

  /**
   * @brief Texel at wrapped coordinates
   */
  Color at(int u, int v) const {
    return column(u)[wrap(v, height, heightMask)];
  }

  /**
   * @brief Wrap a row index into [0, height)
   */
  int wrapV(int v) const {
    return wrap(v, height, heightMask);
  }

  void set(int u, int v, Color color) {
    texels[size_t(u) * height + v] = color;
  }
};

/**
 * @brief Loads each texture file once and keeps it alive
 */
class TextureCache {
private:
  std::string directory;
  std::unordered_map<std::string, std::shared_ptr<const Texture>> cache;

public:
  /**
   * @param directory Prefix for every name, e.g. `Resources/Textures/`
   */
  explicit TextureCache(std::string directory):
    directory(std::move(directory)) {}

  /**
   * @brief Texture by file name, loaded on first use
   *
   * @throw std::runtime_error The file failed to load
   */
  std::shared_ptr<const Texture> get(const std::string &name);
};

} // namespace te
