#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <utility>
#include <vector>

namespace Image {

inline void apply_with_inset(int width, int height, int inset_x, int inset_y, auto&& f) {
  for (int y = inset_y; y < height - inset_y; ++y) {
    for (int x = inset_x; x < width - inset_x; ++x) {
      std::forward<decltype(f)>(f)(x, y);
    }
  }
}

inline void apply(int width, int height, auto&& f) {
  apply_with_inset(width, height, 0, 0, std::forward<decltype(f)>(f));
}

template <typename T>
class Image {
public:
  Image(int width = 0, int height = 0, int padding = 0)
      : m_width { width },
        m_height { height },
        m_padding { padding },
        m_data((m_width + 2 * m_padding) * (m_height + 2 * padding)) {}

  Image(const Image& image, int padding) : Image { image.width(), image.height(), padding } {
    apply(m_width, m_height, [&image, this](int x, int y) {
      this->operator[](x, y) = image[x, y];
    });
  }

  decltype(auto) operator[](this auto&& self, int x, int y) noexcept {
    return std::forward_like<decltype(self)>(
      self.m_data[(self.m_width + 2 * self.m_padding) * (y + self.m_padding) + (x + self.m_padding)]
    );
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

  void clear() noexcept {
    m_width = m_height = m_padding = 0;
    m_data.clear();
  }

private:
  int m_width, m_height;
  int m_padding;
  std::vector<T> m_data;
};

using RGBAImage = Image<glm::vec4>;
using RGBImage = Image<glm::vec3>;
using Gradient = std::pair<float, float>;
using GradientImage = Image<Gradient>;
using GreyscaleImage = Image<float>;
using BinaryImage = Image<char>;

}  // namespace Image