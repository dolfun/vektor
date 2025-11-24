#include <stdexcept>

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace Image {

RGBImage load(const char* path, int padding) {
  int width, height, nr_channels;
  unsigned char* data = stbi_load(path, &width, &height, &nr_channels, 0);

  if (data == nullptr) {
    throw std::runtime_error("File not found: " + std::string(path));
  }

  RGBImage image { width, height, padding };
  apply(width, height, [&](int x, int y) {
    int index = y * width + x;
    float r = static_cast<float>(data[index * nr_channels + 0]);
    float g = static_cast<float>(data[index * nr_channels + 1]);
    float b = static_cast<float>(data[index * nr_channels + 2]);
    image[x, y] = glm::vec3(r, g, b) / 255.0f;
  });

  stbi_image_free(data);
  return image;
}

int (*stbi_write_png_impl)(const char*, int, int, int, const void*, int) = &stbi_write_png;

}  // namespace Image