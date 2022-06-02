#include "Renderer.hpp"
#include "Game.hpp"

void Renderer::renderFrame() {
  vClip = std::vector<Segment<>>{bf.getWidth(),
                                 Segment<>{-1, 1}};

  auto clip = Clip(this, {-1, 1});
  renderSector(game->player.pos, game->activeSector, clip);

}

void Renderer::renderSector(const Vector &pos, const Map::Sector *sec,
                        const Clip &clip, Map::Line *portal) {

  static int depth = 0;
  ++depth;
  if (depth < 10)  {
    renderWalls(pos, sec, clip, portal);
  }
  --depth;
}


void Renderer::renderWalls(const Vector &pos, const Map::Sector *sec,
                       const Clip &clip, Map::Line *portal) {
  auto player = game->player;
  auto activeFloorH = game->activeSector->floorheight;

  for (auto &a : sec->lines) {
    // transform and rotate the line
    auto v1 = (a->v1 - pos).rotatedToDir(player.dir);
    auto v2 = (a->v2 - pos).rotatedToDir(player.dir);

    if (v1.y > 0 || v2.y > 0) {
      // handling lines that are partially  behind the player
      if (v1.y <= 0 || v2.y <= 0) {
        auto i1 = intersec(v1, v2, {0, 0}, {-1, 1});
        auto i2 = intersec(v1, v2, {0, 0}, {1, 1});

        Vector i;
        if (i1.y > 0 && i2.y > 0) {
          if (i1.y < i2.y) i = i1; else i = i2; }
        else if (i1.y > 0) i = i1;
        else if (i2.y > 0) i = i2;
        else continue;

        if (v1.y < 0) v1 = i;
        if (v2.y < 0) v2 = i;
      }

      // Screen vertical begin and end
      Segment<> vSeg = {
          v1.x / v1.y,
          v2.x / v2.y
      };

      // ceiling and floor heights relative to the player
      auto rch = sec->ceilingheight
                 - player.getHeight() - activeFloorH;
      auto rfh = sec->floorheight
                 -  player.getHeight() - activeFloorH;

      Plain middle = { vSeg,
                       { rch / v1.y, rfh / v1.y } ,
                       { rch / v2.y, rfh / v2.y }};

      if (a->portal) {
        renderSector(pos, a->portal->sector, clip.clamped(middle), a);
      }

      renderPlain(middle, clip, a,
        sec->ceilingheight - sec->floorheight,
        a->portal ? sec->ceilingheight - a->portal->sector->ceilingheight : 0,
        a->portal ? a->portal->sector->floorheight - sec->floorheight : 0);
    }
  }
}

void Renderer::renderPlain(Plain plain, Clip clip, const Map::Line *line,
                     double height, double upperHeight, double lowerHeight) {
  if (plain.hSeg.begin > plain.hSeg.end) {
    std::swap(plain.hSeg.begin, plain.hSeg.end);
    std::swap(plain.lSeg, plain.rSeg);
  }

  // Horizontal segment of visible part of the wall
  auto hSeg = hToScreen(clip.hSegClamp(plain.hSeg));

  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    // Calculating k for interpolation
    auto k = (double) (xToView(x) - plain.hSeg.begin)
             / (plain.hSeg.end - plain.hSeg.begin);

    // View screen segment of current column
    Segment<> iSeg = {
      interpolate(plain.lSeg.begin, plain.rSeg.begin, k),
      interpolate(plain.lSeg.end, plain.rSeg.end, k)
    };

    // Clamped screen segment of the column
    Segment vSeg = vToScreen(clip.vSegClamp(x, iSeg));

    if (line->portal) {
      uint16_t upperEnd = yToScreen(clip.vClamp(x,
        iSeg.begin + (iSeg.end - iSeg.begin) * (upperHeight / height)));
      uint16_t lowerBegin = yToScreen(clip.vClamp(x,
        iSeg.end - (iSeg.end - iSeg.begin) * (lowerHeight / height)));

      for (uint16_t y = vSeg.begin; y < upperEnd; ++y)
        bf.setPixel(x, y, sf::Color::Blue);

      for (uint16_t y = lowerBegin; y < vSeg.end; ++y)
        bf.setPixel(x, y, sf::Color::Green);

    } else {
      for (uint16_t y = vSeg.begin; y < vSeg.end; ++y) {
        bf.setPixel(x, y, sf::Color::Red);
      }
    }
  }
}

uint16_t Renderer::xToScreen(double x) {
  return (uint16_t) bf.getWidth() * ((x + 1.0) / 2.0);
}

uint16_t Renderer::yToScreen(double y) {
  return (uint16_t) bf.getHeight() * ((1.0 - y) / 2.0);
}

Segment<uint16_t> Renderer::hToScreen(Segment<double> hSeg) {
  return {
      xToScreen(hSeg.begin),
      xToScreen(hSeg.end)
  };
}

Segment<uint16_t> Renderer::vToScreen(Segment<double> vSeg) {
  return {
      yToScreen(vSeg.begin),
      yToScreen(vSeg.end)
  };
}

double Renderer::xToView(uint16_t x) {
  return (double) x / bf.getWidth() * 2.0 - 1.0;
}

double Renderer::yToView(uint16_t y) {
  return (double) y / bf.getHeight() * 2.0 - 1.0;
}
