#include "Clip.hpp"
#include "Math.hpp"
#include "../Renderer.hpp"

Clip Clip::clamped(Plain portal) const {
  auto phClamped = hClamp(portal.hSeg);
  Segment<> hClamped = {
      std::max(hClip.begin, phClamped.begin),
      std::min(hClip.end, phClamped.end)
  };

  auto hSeg = rend->hToScreen(hClamped);
  auto &vClip = rend->vClip;

  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    auto k = (double) (rend->xToView(x) - portal.hSeg.begin)
             / (portal.hSeg.end - portal.hSeg.begin);

    Segment<> secCol = vClamp(x,{
        interpolate(portal.lSeg.begin, portal.rSeg.begin, k),
        interpolate(portal.lSeg.end, portal.rSeg.end, k)
    });

    vClip[x] = {
        std::min(vClip[x].begin, secCol.begin),
        std::max(vClip[x].end, secCol.end)
    };
  }

  return Clip(rend, hClamped);
}

Segment<> Clip::hClamp(Segment<> seg) const {
  return {
      std::clamp(seg.begin, hClip.begin, hClip.end),
      std::clamp(seg.end, hClip.begin, hClip.end)
  };
}

Segment<> Clip::vClamp(uint16_t x, Segment<> seg) const {
  auto &vClip = rend->vClip;
  return {
      std::clamp(seg.begin, vClip[x].begin, vClip[x].end),
      std::clamp(seg.end, vClip[x].begin, vClip[x].end)
  };
}
