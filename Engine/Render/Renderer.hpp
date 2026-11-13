#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../Core/ThreadPool.hpp"
#include "../Graphics/Frame.hpp"
#include "../World/Entity.hpp"
#include "../World/Map.hpp"

namespace te {

/**
 * @brief Where the picture is taken from
 */
struct Camera {
  Sector *sector = nullptr;
  Vec2 pos;

  // Eye height, absolute
  float z = 0;

  // Radians, 0 faces +x
  float angle = 0;

  // Horizontal field of view in radians
  float fov = radians(90);

  // Look up (> 0) or down by shearing, in screen heights
  float pitch = 0;

  // Entity whose sprite isn't drawn, usually the one holding the camera
  const Entity *self = nullptr;

  /**
   * @brief Height gained per unit of distance along the screen center,
   *        e.g. to aim where the crosshair points
   *
   * @param aspect Frame width / height
   */
  float centerSlope(float aspect) const {
    return 2 * pitch * std::tan(fov / 2) / aspect;
  }
};

/**
 * @brief Software portal renderer: walls, flats and sprites into a Frame
 *
 * Starts in the camera's sector and recurses through every portal in
 * view, clipped to its window on screen. Teleporting portals just move
 * the view, so they render seamlessly. Columns are independent, so the
 * screen is split into strips rendered in parallel.
 */
class Renderer {
private:
  // Eye-space view of one sector, see renderSector()
  struct View {
    const Sector *sector;
    Vec2 pos, dir, right;
  };

  // Projection constants for the current frame
  struct Projection {
    float focal;       // pixels per world unit at depth 1
    float centerX, centerY;
    float eyeZ;
    int height;
  };

  // Sprite queued during the portal walk, drawn after it
  struct SpriteDraw {
    const Texture *texture;
    float depth;
    float left, right, top, bottom;
    int clipBegin, clipEnd;
    uint8_t light;
  };

  // Columns one task renders
  struct Strip {
    int begin, end;
    std::vector<SpriteDraw> sprites;
  };

  ThreadPool &pool;

  Frame *frame = nullptr;
  Projection proj{};
  const Entity *self = nullptr;

  // Open rows per column, [top, bottom): what's still undrawn. Strips
  // touch only their own columns, so sharing them is race-free
  std::vector<int> top, bottom;

  // 1 / (row center - horizon) per row, so flats multiply, not divide
  std::vector<float> rowSlope;

  std::vector<Strip> strips;

  // Brightness by depth, see brightness()
  std::array<uint8_t, 1024> falloff{};

  void renderSector(Strip &strip, const View &view, int begin, int end,
                    int depth);

  void renderWall(Strip &strip, const View &view, const Wall &wall,
                  int begin, int end, int depth);

  void collectSprites(Strip &strip, const View &view, int begin, int end);
  void drawSprites(Strip &strip);

  /**
   * @brief Fill a column span of a floor or ceiling
   */
  void drawFlat(const View &view, int x, int y0, int y1, float planeZ,
                const Texture *texture, float scale, float light);

  /**
   * @brief Fill a column span of a wall part
   */
  void drawWallSpan(int x, int y0, int y1, float depth, float u,
                    float topZ, float partHeight, const Texture *texture,
                    float scale, uint8_t light);

  /**
   * @brief Pixel brightness at a depth, scaled by sector light
   */
  uint8_t brightness(float depth, float light) const;

public:
  // Portal recursion limit; deeper windows are drawn black
  int maxDepth = 32;

  explicit Renderer(ThreadPool &pool);

  /**
   * @brief Draw the camera's view into the whole frame
   */
  void render(const Camera &camera, Frame &frame);
};

} // namespace te
