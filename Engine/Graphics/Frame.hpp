#pragma once

#include <cstdint>
#include <vector>

#include "Texture.hpp"

namespace te {

/**
 * @brief The software-rendered picture: color plus depth per pixel
 *
 * Column-major like Texture, since the renderer fills vertical spans:
 * pixel (x, y) is at `x * height + y`. Screen rotates it back on
 * upload. Color alpha is brightness, see Color.
 */
class Frame {
private:
  int width = 0, height = 0;

  std::vector<Color> colors;

  // View-space distance along the camera direction, for sprites
  std::vector<float> depths;

public:
  /**
   * @brief Reallocate for a new size, contents become undefined
   */
  void resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    colors.assign(size_t(width) * height, Color{});
    depths.assign(size_t(width) * height, 0);
  }

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  Color *colorColumn(int x) { return colors.data() + size_t(x) * height; }
  float *depthColumn(int x) { return depths.data() + size_t(x) * height; }

  const Color *data() const { return colors.data(); }
};

} // namespace te
