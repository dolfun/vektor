#pragma once
#include "image.h"

namespace Image {

ColorImage load(const char*, int padding = 0);

extern int (*stbi_write_png_impl)(const char*, int, int, int, const void*, int);
template <typename T>
void save_as_png(const Image<T>& image, const char* path) {
  const int width = image.width();
  const int height = image.height();
  constexpr int nr_channels = 3;
  std::vector<unsigned char> data(width * height * nr_channels);

  apply(width, height, [&] (int x, int y) {
    int index = y * width + x;

    if constexpr (std::same_as<T, float>) {
      unsigned char value = static_cast<unsigned char>(image(x, y) * 255.9999f);
      data[nr_channels * index] = data[nr_channels * index + 1] = data[nr_channels * index + 2] = value;
    } else if constexpr (std::same_as<T, glm::vec3>) {
      data[nr_channels * index + 0] = static_cast<unsigned char>(image(x, y).r * 255.9999f);
      data[nr_channels * index + 1] = static_cast<unsigned char>(image(x, y).g * 255.9999f);
      data[nr_channels * index + 2] = static_cast<unsigned char>(image(x, y).b * 255.9999f);
    }

  });

  int stride = width * nr_channels;
  stbi_write_png_impl(path, width, height, nr_channels, data.data(), stride);
}

} // namespace Image