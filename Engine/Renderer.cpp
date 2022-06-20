#include "Renderer.hpp"
#include "Game.hpp"

void Renderer::renderFrame() {
  vClip = std::vector<Segment<>>{bf.getWidth(),
                                 Segment<>{1, -1}};

  auto clip = Clip(this, {-1, 1});
  renderSector(game->player.pos, game->activeSector, clip);

}

void Renderer::renderSector(const Vector &pos, const Map::Sector *sec,
                        const Clip &clip, Map::Line *portal) {
  // Variable for controlling depth of the rendering
  static int depth = 0;
  ++depth;
  if (depth < MAX_DEPTH)  {
    renderWalls(pos, sec, clip, portal);
  }
  --depth;
}

void Renderer::renderWalls(const Vector &pos, const Map::Sector *sec,
                       const Clip &clip, Map::Line *portal) {
  auto player = game->player;

  // ceiling and floor heights relative to the player
  auto activeFloorH = game->activeSector->floorheight;
  auto rch = sec->ceilingheight
             - player.getHeight() - activeFloorH;
  auto rfh = sec->floorheight
             -  player.getHeight() - activeFloorH;

  for (auto &a : sec->lines) {
    // transform the line to the player's perspective
    Segment<Vector> tLine = {
        (a->v1 - pos).rotatedToDir(player.dir),
        (a->v2 - pos).rotatedToDir(player.dir)
    };

    // Transformed line clipped if necessary
    Segment<Vector> ctLine = tLine;

    if ((tLine.begin.y > tLine.begin.x || tLine.end.y > tLine.end.x) &&
        (tLine.begin.y > -tLine.begin.x || tLine.end.y > -tLine.end.x)) {

      // Handling lines that are partially  behind the player
      if (tLine.begin.y < 0 || tLine.end.y < 0) {
        // Finding the intersection of the line with the player's field of view
        auto i1 = intersec(tLine.begin, tLine.end, {0, 0}, {-1, 1});
        auto i2 = intersec(tLine.begin, tLine.end, {0, 0}, {1, 1});

        // Finding the right intersection
        Vector i{};
        if (i1.y > 0 && i2.y > 0) {
          if (i1.y < i2.y) i = i1; else i = i2; }
        else if (i1.y > 0) i = i1;
        else if (i2.y > 0) i = i2;
        else continue;

        // Assigning the intersection to the vertex that's behind the player
        if (tLine.begin.y < 0) ctLine.begin = i;
        if (tLine.end.y < 0) ctLine.end = i;
      }

      // Screen vertical begin and end
      Segment<> vSeg = {
          ctLine.begin.x / ctLine.begin.y,
          ctLine.end.x / ctLine.end.y
      };

      // Wall projection on the screen
      Plain plain = { vSeg,
       { rch / ctLine.begin.y, rfh / ctLine.begin.y } ,
       { rch / ctLine.end.y, rfh / ctLine.end.y }};

      auto newClip = renderPlain(plain, clip, a, tLine, ctLine);
      newClip = clip.clamped(plain);

      if (a->portal) {
        auto np = pos - (a->portal->v2 - a->v1);
        renderSector(np, a->portal->sector, newClip, a);
      }
    }
  }
}

Clip Renderer::renderPlain(Plain plain, Clip clip, const Map::Line *line,
                           Segment<Vector> tLine, Segment<Vector> ctLine) {
  // Swap begin and end if necessary
  if (plain.hSeg.begin > plain.hSeg.end) {
    std::swap(plain.hSeg.begin, plain.hSeg.end);
    std::swap(plain.lSeg, plain.rSeg);
    std::swap(tLine.begin, tLine.end);
    std::swap(ctLine.begin, ctLine.end);
  }

  // Calculating the height of the wall in screen coordinates
  double height = line->sector->ceilingheight - line->sector->floorheight;
  double upperHeight, lowerHeight;
  if (line->portal) {
    upperHeight = line->sector->ceilingheight - line->portal->sector->ceilingheight;
    lowerHeight = line->portal->sector->floorheight - line->sector->floorheight;
  }

  // Horizontal segment of visible part of the wall
  auto hSeg = hScreenToBuff(clip.hSegClamp(plain.hSeg));

  // "unpacking" the textures
  auto &middleT = line->side->middle;
  auto &lowerT = line->side->lower;
  auto &upperT = line->side->upper;


  // Calculating left and right start and end
  // points for the lower and upper parts of the wall (great explanation)
//  auto upperEndL = plain.lSeg.begin +
//                  (plain.lSeg.end - plain.lSeg.begin) * (upperHeight / height);
//  auto upperEndR = plain.rSeg.begin +
//                  (plain.rSeg.end - plain.rSeg.begin) * (upperHeight / height);
//  auto lowerBeginL = plain.lSeg.end -
//                  (plain.lSeg.end - plain.lSeg.begin) * (lowerHeight / height);
//  auto lowerBeginR = plain.rSeg.end -
//                  (plain.rSeg.end - plain.rSeg.begin) * (lowerHeight / height);

  // Iterating over the columns
  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    // Calculating k for interpolation
    auto kx = (double) (xBuffToScreen(x) - plain.hSeg.begin)
              / (plain.hSeg.end - plain.hSeg.begin);

    // Screen segment of current column
    Segment<> vSeg = {
      interpolate(plain.lSeg.begin, plain.rSeg.begin, kx),
      interpolate(plain.lSeg.end, plain.rSeg.end, kx)
    };

    // Column position on the map
    auto cy =  1 / interpolate(1 / ctLine.begin.y, 1 / ctLine.end.y, kx);
    auto cx = cy * interpolate(ctLine.begin.x / ctLine.begin.y,
                               ctLine.end.x / ctLine.end.y, kx);

    // Calculating texture x coordinate for all the textures
    auto txk = ((Vector{cx, cy} - tLine.begin).length());
    auto txu = uint16_t(
        upperT.getSize().y * txk / upperHeight) % upperT.getSize().x;
    auto txm = uint16_t(
        middleT.getSize().y * txk / height) % middleT.getSize().x;
    auto txl = uint16_t(
        lowerT.getSize().y * txk / lowerHeight) % lowerT.getSize().x;

    // Clamped buff segment of the column
    Segment vBuffSeg = vScreenToBuff(clip.vSegClamp(x, vSeg));

    // Rendering upper and lower textures if wall is portal
    if (line->portal) {
      // Calculating start and end points for the lower and upper parts of the wall
//      auto upperEnd = interpolate(upperEndL, upperEndR, kx);
//      auto lowerBegin = interpolate(lowerBeginL, lowerBeginR, kx);

      auto upperEnd = vSeg.begin + (vSeg.end - vSeg.begin) * (upperHeight / height);
      auto lowerBegin = vSeg.end - (vSeg.end - vSeg.begin) * (lowerHeight / height);

      // Transforming the points to the buffer coordinates
      auto upperEndBuff = yScreenToBuff(clip.vClamp(x, upperEnd));
      auto lowerBeginBuff = yScreenToBuff(clip.vClamp(x, lowerBegin));

      // Rendering the upper wall
      double tyk =  upperT.getSize().y  / (upperEnd - vSeg.begin);
      for (uint16_t y = vBuffSeg.begin; y < upperEndBuff; ++y) {
        uint16_t ty = tyk * (yBuffToScreen(y) - vSeg.begin);
        bf.setPixel(x, y, upperT.getPixel(txu, ty));
      }

      // Rendering the lower wall
      tyk =  lowerT.getSize().y  / (vSeg.end - lowerBegin);
      for (uint16_t y = lowerBeginBuff; y < vBuffSeg.end; ++y) {
        uint16_t ty = tyk * (yBuffToScreen(y) - lowerBegin);
        bf.setPixel(x, y, lowerT.getPixel(txl, ty));
      }
    } else {
      // Rendering the middle wall
      auto tyk =  middleT.getSize().y  / (vSeg.end - vSeg.begin);
      for (uint16_t y = vBuffSeg.begin; y < vBuffSeg.end; ++y) {
        uint16_t ty = tyk * (yBuffToScreen(y) - vSeg.begin);
        bf.setPixel(x, y, middleT.getPixel(txm, ty));
      }
    }
  }
  return clip.clamped(plain);
}



uint16_t Renderer::xScreenToBuff(double x) {
  return (uint16_t) bf.getWidth() * ((x + 1.0) / 2.0);
}

uint16_t Renderer::yScreenToBuff(double y) {
  return (uint16_t) bf.getHeight() * ((1.0 - y) / 2.0);
}

Segment<uint16_t> Renderer::hScreenToBuff(Segment<double> hSeg) {
  return {
      xScreenToBuff(hSeg.begin),
      xScreenToBuff(hSeg.end)
  };
}

Segment<uint16_t> Renderer::vScreenToBuff(Segment<double> vSeg) {
  return {
      yScreenToBuff(vSeg.begin),
      yScreenToBuff(vSeg.end)
  };
}

double Renderer::xBuffToScreen(uint16_t x) {
  return (double) x / bf.getWidth() * 2.0 - 1.0;
}

double Renderer::yBuffToScreen(uint16_t y) {
  return -((double) y / bf.getHeight() * 2.0 - 1.0);
}