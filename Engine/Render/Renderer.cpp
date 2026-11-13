#include "Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace te {

// Near clipping plane, world units in front of the eye
static constexpr float NEAR = 1;

// World units per falloff table entry
static constexpr float FALLOFF_STEP = 4;

// Columns per task: smaller balances better, but each task walks the
// portal graph again
static constexpr int MIN_STRIP_WIDTH = 16;
static constexpr int STRIPS_PER_THREAD = 4;

static constexpr float FAR = std::numeric_limits<float>::infinity();

// Untextured parts: missing textures and too-deep portals
static constexpr Color BLACK = {0, 0, 0, 255};

/**
 * @brief floor() that compiles to two instructions without SSE4.1
 */
static inline int fastFloor(float x) {
  int i = int(x);
  return i - (x < float(i));
}

// ---- Setup ---------------------------------------------------------------

Renderer::Renderer(ThreadPool &pool): pool(pool) {
  // Bright up close, fading to 40% at a distance
  for (size_t i = 0; i < falloff.size(); ++i) {
    float z = (float(i) + 0.5f) * FALLOFF_STEP;
    float near = 255 * 2000 / (z * z);
    float far = std::clamp(100 + 255 * 250 / z, 0.0f, 150.0f);
    falloff[i] = uint8_t(std::clamp(near + far, 0.0f, 255.0f));
  }
}

uint8_t Renderer::brightness(float depth, float light) const {
  // Unsigned compare also sends NaN and negatives to the last entry
  auto i = unsigned(depth * (1 / FALLOFF_STEP));
  uint8_t base = falloff[std::min<size_t>(i, falloff.size() - 1)];

  // Most sectors have normal light, skip the float math for them
  if (light == 1) return base;
  return uint8_t(std::min(255.0f, base * light));
}

void Renderer::render(const Camera &camera, Frame &target) {
  frame = &target;
  self = camera.self;

  int width = target.getWidth();
  int height = target.getHeight();

  proj.focal = width / 2.0f / std::tan(camera.fov / 2);
  proj.centerX = width / 2.0f;
  proj.centerY = height / 2.0f + camera.pitch * height;
  proj.eyeZ = camera.z;
  proj.height = height;

  top.assign(width, 0);
  bottom.assign(width, height);

  rowSlope.resize(height);
  for (int y = 0; y < height; ++y) {
    rowSlope[y] = 1 / (y + 0.5f - proj.centerY);
  }

  // Strips
  int count = pool.size() == 1 ? 1 : pool.size() * STRIPS_PER_THREAD;
  count = std::clamp(count, 1, std::max(1, width / MIN_STRIP_WIDTH));

  strips.resize(count);

  for (int i = 0; i < count; ++i) {
    strips[i].begin = width * i / count;
    strips[i].end = width * (i + 1) / count;
  }

  Vec2 dir = Vec2::fromAngle(camera.angle);
  View root = {camera.sector, camera.pos, dir, dir.right()};

  pool.run(count, [&](int i) {
    Strip &strip = strips[i];
    strip.sprites.clear();

    renderSector(strip, root, strip.begin, strip.end, 0);
    drawSprites(strip);
  });
}

// ---- Portal walk ---------------------------------------------------------

void Renderer::renderSector(Strip &strip, const View &view, int begin,
                            int end, int depth) {
  if (depth > maxDepth) {
    for (int x = begin; x < end; ++x) {
      std::fill(frame->colorColumn(x) + top[x],
                frame->colorColumn(x) + bottom[x], BLACK);
      std::fill(frame->depthColumn(x) + top[x],
                frame->depthColumn(x) + bottom[x], FAR);
      top[x] = bottom[x];
    }

    return;
  }

  collectSprites(strip, view, begin, end);

  // Sectors are convex: each column meets exactly one front-facing wall
  for (const Wall &wall : view.sector->walls) {
    renderWall(strip, view, wall, begin, end, depth);
  }
}

void Renderer::renderWall(Strip &strip, const View &view, const Wall &wall,
                          int begin, int end, int depth) {
  // ---- Project the wall ----

  Vec2 ra = wall.a - view.pos, rb = wall.b - view.pos;
  float za = ra.dot(view.dir), zb = rb.dot(view.dir);

  if (za < NEAR && zb < NEAR) return;

  float sa = ra.dot(view.right), sb = rb.dot(view.right);
  float length = wall.length();
  float ua = 0, ub = length;

  // Clip to the near plane, keeping texture u in step
  if (za < NEAR) {
    float t = (NEAR - za) / (zb - za);
    sa = lerp(sa, sb, t);
    ua = length * t;
    za = NEAR;
  } else if (zb < NEAR) {
    float t = (NEAR - za) / (zb - za);
    sb = lerp(sa, sb, t);
    ub = length * t;
    zb = NEAR;
  }

  float xa = proj.centerX + proj.focal * sa / za;
  float xb = proj.centerX + proj.focal * sb / zb;

  // Clockwise walls run left to right when seen from inside
  if (xa >= xb) return;

  int x0 = std::max(begin, int(std::ceil(xa - 0.5f)));
  int x1 = std::min(end, int(std::ceil(xb - 0.5f)));

  if (x0 >= x1) return;

  // ---- Columns ----

  const Sector &sector = *view.sector;
  const Sector *next = wall.next();

  // 1/z and u/z are linear in screen x: perspective-correct texturing
  float iza = 1 / za, izb = 1 / zb;
  float uza = ua * iza, uzb = ub * izb;
  float invDx = 1 / (xb - xa);

  for (int x = x0; x < x1; ++x) {
    int &t = top[x], &b = bottom[x];
    if (t >= b) continue;

    float k = (x + 0.5f - xa) * invDx;
    float iz = lerp(iza, izb, k);
    float z = 1 / iz;
    float u = lerp(uza, uzb, k) * z;

    // First row at or below a world height, clamped to the open rows
    auto rowAt = [&](float worldZ, int lo, int hi) {
      float y = proj.centerY - proj.focal * (worldZ - proj.eyeZ) * iz;
      return std::clamp(int(std::ceil(y - 0.5f)), lo, hi);
    };

    int ceilingY = rowAt(sector.ceiling, t, b);
    int floorY = rowAt(sector.floor, ceilingY, b);

    drawFlat(view, x, t, ceilingY, sector.ceiling,
             sector.ceilingTexture.get(), sector.flatScale, sector.light);
    drawFlat(view, x, floorY, b, sector.floor,
             sector.floorTexture.get(), sector.flatScale, sector.light);

    uint8_t light = brightness(z, sector.light);

    if (!next) {
      drawWallSpan(x, ceilingY, floorY, z, u, sector.ceiling,
                   sector.ceiling - sector.floor, wall.middle.get(),
                   wall.textureScale, light);
      t = b;
      continue;
    }

    // Portal: upper and lower parts frame the opening
    int openTop = rowAt(next->ceiling, ceilingY, floorY);
    int openBottom = rowAt(next->floor, openTop, floorY);

    drawWallSpan(x, ceilingY, openTop, z, u, sector.ceiling,
                 sector.ceiling - next->ceiling, wall.upper.get(),
                 wall.textureScale, light);
    drawWallSpan(x, openBottom, floorY, z, u, next->floor,
                 next->floor - sector.floor, wall.lower.get(),
                 wall.textureScale, light);

    t = openTop;
    b = openBottom;
  }

  if (!next) return;

  // ---- Through the portal ----

  const Transform &teleport = wall.toPartner;
  View behind = {next, teleport.apply(view.pos),
                 teleport.applyDir(view.dir), teleport.applyDir(view.right)};

  renderSector(strip, behind, x0, x1, depth + 1);

  // Whatever the far side left open is done too
  for (int x = x0; x < x1; ++x) {
    top[x] = bottom[x];
  }
}

// ---- Spans ---------------------------------------------------------------

void Renderer::drawFlat(const View &view, int x, int y0, int y1,
                        float planeZ, const Texture *texture, float scale,
                        float light) {
  if (y0 >= y1) return;

  Color *colors = frame->colorColumn(x);
  float *depths = frame->depthColumn(x);

  // > 0 for floors, < 0 for ceilings; rows above the horizon flip too
  float height = proj.focal * (proj.eyeZ - planeZ);
  float invScale = 1 / scale;

  // Texel step per unit of depth, in texture space
  Vec2 ray = view.dir + view.right * ((x + 0.5f - proj.centerX) / proj.focal);
  Vec2 origin = view.pos * invScale;
  ray *= invScale;

  for (int y = y0; y < y1; ++y) {
    float depth = height * rowSlope[y];

    if (!(depth > 0)) {
      colors[y] = BLACK;
      depths[y] = FAR;
      continue;
    }

    Vec2 p = origin + ray * depth;
    Color c = texture ? texture->at(fastFloor(p.x), fastFloor(p.y)) : BLACK;

    colors[y] = c.withAlpha(brightness(depth, light));
    depths[y] = depth;
  }
}

void Renderer::drawWallSpan(int x, int y0, int y1, float depth, float u,
                            float topZ, float partHeight,
                            const Texture *texture, float scale,
                            uint8_t light) {
  if (y0 >= y1) return;

  Color *colors = frame->colorColumn(x);
  float *depths = frame->depthColumn(x);

  std::fill(depths + y0, depths + y1, depth);

  if (!texture) {
    std::fill(colors + y0, colors + y1, BLACK);
    return;
  }

  // Fit: one texture height per wall part
  if (scale <= 0) scale = partHeight / texture->getHeight();

  const Color *column = texture->column(fastFloor(u / scale));

  // Texel row is linear in screen y along a wall column
  float dv = depth / (proj.focal * scale);
  float v = (topZ - proj.eyeZ) / scale + (y0 + 0.5f - proj.centerY) * dv;

  for (int y = y0; y < y1; ++y, v += dv) {
    colors[y] = column[texture->wrapV(fastFloor(v))].withAlpha(light);
  }
}

// ---- Sprites -------------------------------------------------------------

void Renderer::collectSprites(Strip &strip, const View &view, int begin,
                              int end) {
  for (const Entity *entity : view.sector->entities) {
    if (entity == self || !entity->sprite) continue;

    Vec2 rel = entity->pos - view.pos;
    float depth = rel.dot(view.dir);

    if (depth < NEAR) continue;

    const Texture &texture = *entity->sprite;
    float width = texture.getWidth() * entity->spriteScale;
    float height = texture.getHeight() * entity->spriteScale;

    float scale = proj.focal / depth;
    float center = proj.centerX + rel.dot(view.right) * scale;
    float left = center - width / 2 * scale;
    float right = center + width / 2 * scale;

    if (right < begin || left > end) continue;

    float bottomY = proj.centerY - (entity->z - proj.eyeZ) * scale;
    float topY = bottomY - height * scale;

    strip.sprites.push_back({&texture, depth, left, right, topY, bottomY,
                             begin, end,
                             brightness(depth, view.sector->light)});
  }
}

void Renderer::drawSprites(Strip &strip) {
  // Far to near, so nearer ones paint over
  std::sort(strip.sprites.begin(), strip.sprites.end(),
            [](const auto &a, const auto &b) { return a.depth > b.depth; });

  for (const SpriteDraw &s : strip.sprites) {
    const Texture &texture = *s.texture;
    int texWidth = texture.getWidth(), texHeight = texture.getHeight();

    int x0 = std::max({strip.begin, s.clipBegin,
                       int(std::ceil(s.left - 0.5f))});
    int x1 = std::min({strip.end, s.clipEnd,
                       int(std::ceil(s.right - 0.5f))});
    int y0 = std::max(0, int(std::ceil(s.top - 0.5f)));
    int y1 = std::min(proj.height, int(std::ceil(s.bottom - 0.5f)));

    float du = texWidth / (s.right - s.left);
    float dv = texHeight / (s.bottom - s.top);

    for (int x = x0; x < x1; ++x) {
      int u = std::clamp(int((x + 0.5f - s.left) * du), 0, texWidth - 1);
      const Color *column = texture.column(u);

      Color *colors = frame->colorColumn(x);
      const float *depths = frame->depthColumn(x);

      float v = (y0 + 0.5f - s.top) * dv;

      for (int y = y0; y < y1; ++y, v += dv) {
        if (s.depth >= depths[y]) continue;

        Color c = column[std::min(int(v), texHeight - 1)];
        if (c.a >= 128) colors[y] = c.withAlpha(s.light);
      }
    }
  }
}

} // namespace te
