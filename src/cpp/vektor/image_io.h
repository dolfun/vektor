#pragma once
#include "image.h"

namespace Image {

RGBImage load(const char*, int padding = 0);

extern int (*stbi_write_png_impl)(const char*, int, int, int, const void*, int);

template <typename T>
void save_as_png(const Image<T>& image, const char* path) {
  const int width = image.width();
  const int height = image.height();
  constexpr int NR_CHANNELS = 3;
  constexpr float SCALE_FACTOR = 255.9999f;

  std::vector<unsigned char> data(width * height * NR_CHANNELS);
  apply(width, height, [&](int x, int y) {
    int index = y * width + x;

    if constexpr (std::same_as<T, float>) {
      unsigned char value = static_cast<unsigned char>(image[x, y] * SCALE_FACTOR);
      data[NR_CHANNELS * index] = data[NR_CHANNELS * index + 1] = data[NR_CHANNELS * index + 2] =
        value;
    } else if constexpr (std::same_as<T, glm::vec3>) {
      data[NR_CHANNELS * index + 0] = static_cast<unsigned char>(image[x, y].r * SCALE_FACTOR);
      data[NR_CHANNELS * index + 1] = static_cast<unsigned char>(image[x, y].g * SCALE_FACTOR);
      data[NR_CHANNELS * index + 2] = static_cast<unsigned char>(image[x, y].b * SCALE_FACTOR);
    }
  });

  int stride = width * NR_CHANNELS;
  stbi_write_png_impl(path, width, height, NR_CHANNELS, data.data(), stride);
}

}  // namespace Image