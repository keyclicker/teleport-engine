#pragma once

#include "Frame.hpp"
#include "Texture.hpp"

namespace te {

/**
 * @brief 2D drawing into a Frame, in frame pixels: HUDs, weapons, menus
 *
 * Everything is clipped to the frame. Drawn pixels get full brightness.
 */
class Canvas {
private:
  Frame &frame;

public:
  explicit Canvas(Frame &frame): frame(frame) {}

  int getWidth() const { return frame.getWidth(); }
  int getHeight() const { return frame.getHeight(); }

  /**
   * @brief Fill a rectangle with a solid color
   *
   * @param x Left edge
   * @param y Top edge, y goes down
   * @param w Width
   * @param h Height
   * @param color Fill color, its alpha is ignored
   */
  void fillRect(int x, int y, int w, int h, Color color);

  /**
   * @brief Draw a texture scaled, skipping transparent texels
   *
   * @param texture Source, texels with alpha < 128 are skipped
   * @param x Left edge
   * @param y Top edge
   * @param scale Frame pixels per texel
   */
  void drawTexture(const Texture &texture, int x, int y, float scale);

  /**
   * @brief Blend a color over the whole frame, e.g. a damage flash
   *
   * @param color Tint color
   * @param amount 0 leaves the frame, 1 replaces it with `color`
   */
  void tint(Color color, float amount);
};

} // namespace te
