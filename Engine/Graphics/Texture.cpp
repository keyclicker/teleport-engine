#include "Texture.hpp"

#include <memory>
#include <stdexcept>

// Only the formats Resources/ uses
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#include <stb_image.h>

namespace te {

static_assert(sizeof(Color) == 4, "Color must match STBI_rgb_alpha");

/**
 * @brief `size - 1` if size is a power of two, else 0
 */
static uint32_t powerOfTwoMask(int size) {
  bool pow2 = size > 0 && (size & (size - 1)) == 0;
  return pow2 ? uint32_t(size - 1) : 0;
}

Texture::Texture(int width, int height, Color color):
  width(width), height(height),
  widthMask(powerOfTwoMask(width)), heightMask(powerOfTwoMask(height)),
  texels(size_t(width) * height, color) {}

Texture Texture::load(const std::string &path) {
  int w, h;
  std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> data{
    stbi_load(path.c_str(), &w, &h, nullptr, STBI_rgb_alpha),
    stbi_image_free
  };

  if (!data) {
    throw std::runtime_error("Failed to load image \"" + path + "\": "
                             + stbi_failure_reason());
  }

  // stb gives rows, we store columns
  auto rows = reinterpret_cast<const Color *>(data.get());
  Texture texture(w, h);

  for (int v = 0; v < h; ++v) {
    for (int u = 0; u < w; ++u) {
      texture.set(u, v, rows[size_t(v) * w + u]);
    }
  }

  return texture;
}

std::shared_ptr<const Texture> TextureCache::get(const std::string &name) {
  auto &slot = cache[name];

  if (!slot) {
    slot = std::make_shared<const Texture>(Texture::load(directory + name));
  }

  return slot;
}

} // namespace te
