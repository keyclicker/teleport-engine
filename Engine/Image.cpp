#include "Image.hpp"

#include <cstring>
#include <iostream>
#include <memory>

// Only the formats Resources/Textures uses
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#include <stb_image.h>

static_assert(sizeof(Color) == 4, "Color must match STBI_rgb_alpha");

bool Image::loadFromFile(const std::string &path) {
  int w, h;
  std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> data{
    stbi_load(path.c_str(), &w, &h, nullptr, STBI_rgb_alpha),
    stbi_image_free
  };

  if (!data) {
    std::cerr << "Failed to load image \"" << path << "\": "
              << stbi_failure_reason() << std::endl;
    return false;
  }

  width = w;
  height = h;
  pixels.resize(size_t(w) * h);
  std::memcpy(pixels.data(), data.get(), pixels.size() * sizeof(Color));

  return true;
}
