#pragma once
#include <glm/vec3.hpp>
#include <utility>
#include <vector>

namespace Image {

template <typename T>
class Image {
public:
  Image(int width, int height, int padding = 0)
      : m_width { width },
        m_height { height },
        m_padding { padding },
        m_data((m_width + 2 * m_padding) * (m_height + 2 * padding)) {
  }

  auto operator[](this auto&& self, int x, int y) noexcept -> decltype(auto) {
    return std::forward_like<decltype(self)>(
      self.m_data[self.m_width * (y + self.m_padding) + (x + self.m_padding)]
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

private:
  int m_width, m_height;
  int m_padding;
  std::vector<T> m_data;
};

using ColorImage = Image<glm::vec3>;
using GreyscaleImage = Image<float>;
using BinaryImage = Image<char>;

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

}  // namespace Image