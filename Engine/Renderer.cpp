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
        if (sec->floorheight < a->portal->sector->floorheight) {
          auto rfhPortal = a->portal->sector->floorheight
                           - player.getHeight() - activeFloorH;
          Plain lower = { vSeg,
                          { rfhPortal / v1.y, rfh / v1.y } ,
                          { rfhPortal / v2.y, rfh / v2.y }};
          renderPlain(lower, clip, 50, v1, v2, a->side->lower);
        }

        if (sec->ceilingheight > a->portal->sector->ceilingheight) {
          auto rchPortal = a->portal->sector->ceilingheight
                           - player.getHeight() - activeFloorH;
          Plain upper = { vSeg,
                          { rch / v1.y, rchPortal / v1.y } ,
                          { rch / v2.y, rchPortal / v2.y }};
          renderPlain(upper, clip, 50, v1, v2, a->side->upper);
        }

        renderSector(pos, a->portal->sector, clip.clamped(middle), a);
      } else {
        renderPlain(middle, clip, 100, v1, v2, a->side->middle);
      }
    }
  }
}

void Renderer::renderPlain(Plain plain, Clip clip, double height,
                       Vector v1, Vector v2, const sf::Image &texture) {
  if (plain.hSeg.begin > plain.hSeg.end) {
    std::swap(plain.hSeg.begin, plain.hSeg.end);
    std::swap(plain.lSeg, plain.rSeg);
  }

  // Horizontal segment of visible part of the wall
  auto hSeg = hToScreen(clip.hClamp(plain.hSeg));

  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    // Calculating k for interpolation
    auto k = (double) (xToView(x) - plain.hSeg.begin)
             / (plain.hSeg.end - plain.hSeg.begin);

    Segment vSeg = vToScreen(clip.vClamp(x, {
        interpolate(plain.lSeg.begin, plain.rSeg.begin, k),
        interpolate(plain.lSeg.end, plain.rSeg.end, k)
    }));

    uint16_t tx = (uint16_t) (k * texture.getSize().x);

    for (uint16_t y = vSeg.begin; y < vSeg.end; ++y) {
      uint16_t ty = y;
      auto c = texture.getPixel(tx, ty);
      bf.setPixel(x, y, c);
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
