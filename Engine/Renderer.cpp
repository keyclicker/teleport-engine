#include "Renderer.hpp"
#include "Game.hpp"

#include <iostream> // for debugging

void Renderer::renderFrame() {
  visitedSectors.clear();
  vClip = std::vector<Segment<>>{bf.getWidth(), Segment<>{1, -1}};

  auto clip = Clip(this, {-1, 1});
  renderSector(game->player.pos, game->activeSector, clip);
//  __debugRenderVClip();
}

void Renderer::renderSector(const Vector &pos, Map::Sector *sec,
                        const Clip &clip, Map::Line *portal) {
  // Variable for controlling depth of the rendering
  static int depth = 0;

  ++depth;
  if (!visitedSectors.contains({sec, pos.x, pos.y}) && depth < MAX_DEPTH)  {
    visitedSectors.insert({sec, pos.x, pos.y});
    renderVisplane(pos, sec, clip, true);
    renderVisplane(pos, sec, clip, false);
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
          if (i1.y < i2.y) i = i1; else i = i2;
        } else if (i1.y > 0) i = i1;
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
      Plane plane = {vSeg,
                     {rch / ctLine.begin.y, rfh / ctLine.begin.y},
                     {rch / ctLine.end.y, rfh / ctLine.end.y}};

      if (clip.hSegClamp(plane.hSeg).length() >0) {
        auto newClip = renderPlane(plane, clip, a, tLine, ctLine);

        if (a->portal) {
          auto np = pos + (a->portal->v2 - a->v1);
          renderSector(np, a->portal->sector, newClip, a);
        }
      }
    }
  }
}

Clip Renderer::renderPlane(Plane plane, Clip clip, const Map::Line *line,
                           Segment<Vector> tLine, Segment<Vector> ctLine) {
  // Swap begin and end if necessary
  if (plane.hSeg.begin > plane.hSeg.end) {
    std::swap(plane.hSeg.begin, plane.hSeg.end);
    std::swap(plane.lSeg, plane.rSeg);
    std::swap(tLine.begin, tLine.end);
    std::swap(ctLine.begin, ctLine.end);
  }

  // Calculating the height of the wall in screen coordinates
  double height = line->sector->ceilingheight - line->sector->floorheight;
  double upperHeight, lowerHeight,
      upperEndL, upperEndR, lowerBeginL, lowerBeginR;

  if (line->portal) {
    upperHeight = line->sector->ceilingheight - line->portal->sector->ceilingheight;
    lowerHeight = line->portal->sector->floorheight - line->sector->floorheight;

    // Calculating left and right start and end
    // points for the lower and upper parts of the wall (great explanation)
    upperEndL = plane.lSeg.begin +
        (plane.lSeg.end - plane.lSeg.begin) * (upperHeight / height);
    upperEndR = plane.rSeg.begin +
        (plane.rSeg.end - plane.rSeg.begin) * (upperHeight / height);
    lowerBeginL = plane.lSeg.end -
        (plane.lSeg.end - plane.lSeg.begin) * (lowerHeight / height);
    lowerBeginR = plane.rSeg.end -
        (plane.rSeg.end - plane.rSeg.begin) * (lowerHeight / height);
  }

  // Horizontal segment of visible part of the wall
  auto hSeg = hScreenToBuff(clip.hSegClamp(plane.hSeg));

  // "unpacking" the textures
  auto &middleT = line->side->middle;
  auto &lowerT = line->side->lower;
  auto &upperT = line->side->upper;

  // Iterating over the columns
  for (uint16_t x = hSeg.begin; x < hSeg.end; ++x) {
    // Calculating k for interpolation
    auto kx = (double) (xBuffToScreen(x) - plane.hSeg.begin)
              / (plane.hSeg.end - plane.hSeg.begin);

    // Screen segment of current column
    Segment<> vSeg = {
      interpolate(plane.lSeg.begin, plane.rSeg.begin, kx),
      interpolate(plane.lSeg.end, plane.rSeg.end, kx)
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

    // Color alpha for the column
    auto ca = zFunction(cy);

    // Rendering upper and lower textures if wall is portal
    if (line->portal) {
      // Calculating start and end points for the lower and upper parts of the wall
      auto upperEnd = interpolate(upperEndL, upperEndR, kx);
      auto lowerBegin = interpolate(lowerBeginL, lowerBeginR, kx);

      // Transforming the points to the buffer coordinates
      auto upperEndBuff = yScreenToBuff(clip.vClamp(x, upperEnd));
      auto lowerBeginBuff = yScreenToBuff(clip.vClamp(x, lowerBegin));

      // Rendering the upper wall
      double tyk =  upperT.getSize().y  / (upperEnd - vSeg.begin);
      for (uint16_t y = vBuffSeg.begin; y < upperEndBuff; ++y) {
        uint16_t ty = uint16_t (tyk *
            (yBuffToScreen(y) - vSeg.begin)) % upperT.getSize().y;
        auto c = upperT.getPixel(txu, ty); c.a = ca;
        bf.setPixel(x, y, c);
      }

      // Rendering the lower wall
      tyk =  lowerT.getSize().y  / (vSeg.end - lowerBegin);
      for (uint16_t y = lowerBeginBuff; y < vBuffSeg.end; ++y) {
        uint16_t ty = uint16_t (tyk *
            (yBuffToScreen(y) - lowerBegin)) % lowerT.getSize().y;;
        auto c = lowerT.getPixel(txl, ty); c.a = ca;
        bf.setPixel(x, y, c);
      }
    } else {
      // Rendering the middle wall
      auto tyk =  middleT.getSize().y  / (vSeg.end - vSeg.begin);
      for (uint16_t y = vBuffSeg.begin; y < vBuffSeg.end; ++y) {
        uint16_t ty = uint16_t (tyk *
            (yBuffToScreen(y) - vSeg.begin)) % middleT.getSize().y;;
        auto c = middleT.getPixel(txm, ty); c.a = ca;
        bf.setPixel(x, y, c);
      }
    }
  }

  if (line->portal)
    return clip.clamped({plane.hSeg,
   {std::min(upperEndL, plane.lSeg.begin), std::max(lowerBeginL, plane.lSeg.end)},
   {std::min(upperEndR, plane.rSeg.begin), std::max(lowerBeginR, plane.rSeg.end)}});
  else
    return clip;
}

// Needs to be rewritten with fill algorithm
void Renderer::renderVisplane(const Vector &pos, const Map::Sector *sec,
                       const Clip &clip, bool isFloor = true) {
  auto rayDirL = game->player.dir - game->player.plane;
  auto rayDirR = game->player.dir + game->player.plane;
  auto vec = (rayDirR - rayDirL);

  // Vertical position of the camera
  double h;
  if (isFloor)
    h = game->player.getHeight() -
      sec->floorheight + game->activeSector->floorheight;
  else
    h = game->player.getHeight() -
      sec->ceilingheight + game->activeSector->floorheight ;

  auto &tex = isFloor ? sec->floor : sec->ceiling;

  // Horizontal clip in buffer coordinates
  auto hClipBuff = hScreenToBuff(clip.hClip);

  // Preparing necessary data for the loop
  auto yBegin = bf.getHeight() / 2;
  auto yEnd = isFloor ? bf.getHeight() : 0;
  auto xBegin = hClipBuff.begin;
  auto xEnd = hClipBuff.end;

  auto yTexSize = tex.getSize().y;
  auto xTexSize = tex.getSize().x;

  for (int y = yBegin; isFloor == y < yEnd; isFloor ? ++y : --y) {
    // Z coordinate of the row
    double rowZ = (double) h / -yBuffToScreen(y);

    auto floorStep = (rowZ / bf.getWidth()) * vec;
    auto floorP = pos + rowZ * rayDirL + floorStep * hClipBuff.begin;

    auto ca = zFunction(rowZ);

    bool pixelSet = false;
    for (int x = xBegin; x < xEnd; ++x) {
      if (y > yScreenToBuff(vClip[x].begin) &&
          y < yScreenToBuff(vClip[x].end)) {
        uint16_t ty = int(floorP.y * yTexSize / 50) % yTexSize;
        uint16_t tx = int(floorP.x * xTexSize / 50) % xTexSize;

        auto c = tex.getPixel(tx, ty); c.a = ca;
        bf.setPixel(x, y, c);
        pixelSet = true;
      }
      else if (pixelSet) break;

      floorP += floorStep;
    }
    if (!pixelSet) break;
  }
}

void Renderer::__debugRenderVClip() {
  for (uint16_t x = 0; x < bf.getWidth(); ++x) {
    for (uint16_t y = yScreenToBuff(vClip[x].begin); y < yScreenToBuff(vClip[x].end); ++y) {
      bf.setPixel(x, y, sf::Color(0, 20, 0, 20) + bf.getPixel(x, y));
    }
  }
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
