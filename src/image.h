#pragma once
#include <vector>
#include <glm/vec3.hpp>

namespace Image {

template <typename T>
class Image {
public:
  Image(int width, int height, int padding = 0)
  : m_width { width }, m_height { height }, m_padding { padding },
    m_data((m_width + 2 * m_padding) * (m_height + 2 * padding)) {}

  const T& operator()(int x, int y) const noexcept {
    return m_data[m_width * (y + m_padding) + (x + m_padding)];
  }

  T& operator()(int x, int y) noexcept {
    return const_cast<T&>(std::as_const(*this)(x, y));
  }

  int width() const noexcept {
    return m_width;
  }

  int height() const noexcept {
    return m_height;
  }

  int padding() const noexcept {
    return m_padding;
  }

  auto data() const noexcept -> const std::vector<T>& {
    return m_data;
  }

private:
  int m_width, m_height;
  int m_padding;
  std::vector<T> m_data;
};

using ColorImage = Image<glm::vec3>;
using GreyscaleImage = Image<float>;

inline void apply_with_inset(int width, int height, int inset_x, int inset_y, auto&& f) noexcept {
  for (int y = inset_y; y < height - inset_y; ++y) {
    for (int x = inset_x; x < width - inset_x; ++x) {
      std::forward<decltype(f)>(f)(x, y);
    }
  }
}

inline void apply(int width, int height, auto&& f) noexcept {
  apply_with_inset(width, height, 0, 0, std::forward<decltype(f)>(f));
}

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