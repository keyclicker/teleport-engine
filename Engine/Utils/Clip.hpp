#pragma once

#include <algorithm>

#include "Utils.hpp"

class Renderer;

// Used for storing horizontal clipping
// for each portal branch and providing interface
// for clipping both vClip and horizontal clip
struct Clip {
  Renderer *rend;
  Segment<> hClip;

  explicit Clip(Renderer *rend): rend(rend), hClip({-1, 1}) {};
  Clip(Renderer *rend, Segment<> hClip): rend(rend), hClip(hClip) {};

  /**
   * @brief Clips clip with another clip (portal)
   *
   * @param portal Portal to clip with
   */
  [[nodiscard]] Clip clamped(Plane portal) const;

  /**
   * @brief Clips horizontal segment
   *
   * @param seg Segment to clip
   */
  [[nodiscard]] Segment<> hSegClamp(Segment<> seg) const;

  /**
   * @brief Clips vertical segment
   *
   * @param x X coordinate of the portal
   * @param seg Segment to clip
   */
  [[nodiscard]] Segment<> vSegClamp(uint16_t x, Segment<> seg) const;

  /**
   * @brief Clips horizontal coordinate
   * @param x X coordinate
   * @return uint16_t Clipped coordinate
   */
  [[nodiscard]] double hClamp(double x) const;

  /**
   * @brief Clips vertical coordinate
   * @param x X coordinate
   * @param y Y coordinate
   * @return uint16_t Clipped coordinate
   */
  [[nodiscard]] double vClamp(uint16_t x, double y) const;
};