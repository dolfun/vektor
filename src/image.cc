#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace Image {

Image load(const char* path, int padding) {
  int width, height, nr_channels;
  unsigned char* data = stbi_load(path, &width, &height, &nr_channels, 0);

  Image image { width, height, padding };
  apply(width, height, [&] (int i, int j) {
    int index = i * width + j;
    float r = static_cast<float>(data[index * nr_channels + 0]) / 255.0f;
    float g = static_cast<float>(data[index * nr_channels + 1]) / 255.0f;
    float b = static_cast<float>(data[index * nr_channels + 2]) / 255.0f;
    image(i, j) = r * 0.2989f + g * 0.5870f + b * 0.1140f;
  });

  stbi_image_free(data);
  return image;
}

void save_as_png(const Image& image, const char* path) {
  const int width = image.width();
  const int height = image.height();
  constexpr int nr_channels = 3;
  std::vector<unsigned char> data(width * height * nr_channels);

  apply(width, height, [&] (int i, int j) {
    int index = i * width + j;
    unsigned char value = static_cast<unsigned char>(image(i, j) * 255.9999f);
    data[nr_channels * index] = data[nr_channels * index + 1] = data[nr_channels * index + 2] = value;
  });

  int stride = width * nr_channels;
  stbi_write_png(path, width, height, nr_channels, data.data(), stride);
}

} // namespace Image