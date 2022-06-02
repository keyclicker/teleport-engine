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

  [[nodiscard]] Clip clamped(Plain portal) const;
  [[nodiscard]] Segment<> hClamp(Segment<> seg) const;
  [[nodiscard]] Segment<> vClamp(uint16_t x, Segment<> seg) const;
};