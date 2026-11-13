#include "Canvas.hpp"

#include <algorithm>
#include <cmath>

namespace te {

void Canvas::fillRect(int x, int y, int w, int h, Color color) {
  int x0 = std::max(x, 0), x1 = std::min(x + w, getWidth());
  int y0 = std::max(y, 0), y1 = std::min(y + h, getHeight());

  for (int px = x0; px < x1; ++px) {
    std::fill(frame.colorColumn(px) + y0, frame.colorColumn(px) + y1,
              color.withAlpha(255));
  }
}

void Canvas::drawTexture(const Texture &texture, int x, int y,
                         float scale) {
  int w = int(std::ceil(texture.getWidth() * scale));
  int h = int(std::ceil(texture.getHeight() * scale));

  int x0 = std::max(x, 0), x1 = std::min(x + w, getWidth());
  int y0 = std::max(y, 0), y1 = std::min(y + h, getHeight());

  for (int px = x0; px < x1; ++px) {
    const Color *src = texture.column(int((px - x) / scale));
    Color *dst = frame.colorColumn(px);

    for (int py = y0; py < y1; ++py) {
      Color c = src[std::min(int((py - y) / scale), texture.getHeight() - 1)];
      if (c.a >= 128) dst[py] = c.withAlpha(255);
    }
  }
}

void Canvas::tint(Color color, float amount) {
  int k = std::clamp(int(amount * 256), 0, 256);
  if (k == 0) return;

  auto mix = [k](uint8_t a, uint8_t b) {
    return uint8_t((a * (256 - k) + b * k) >> 8);
  };

  for (int px = 0; px < getWidth(); ++px) {
    Color *column = frame.colorColumn(px);

    for (int py = 0; py < getHeight(); ++py) {
      Color &c = column[py];

      // Mix the lit color, then show it at full brightness
      Color lit = {uint8_t(c.r * c.a / 255), uint8_t(c.g * c.a / 255),
                   uint8_t(c.b * c.a / 255)};
      c = {mix(lit.r, color.r), mix(lit.g, color.g), mix(lit.b, color.b),
           255};
    }
  }
}

} // namespace te
