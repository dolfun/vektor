#pragma once
#include <vector>

namespace Image {

class Image {
public:
  Image(int width, int height, int padding = 0)
  : m_width { width }, m_height { height }, m_padding { padding },
    m_data((m_width + 2 * m_padding) * (m_height + 2 * padding)) {}

  const float& operator()(int i, int j) const noexcept {
    return m_data[m_width * (i + m_padding) + j + m_padding];
  }

  float& operator()(int i, int j) noexcept {
    return const_cast<float&>(std::as_const(*this)(i, j));
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

  auto data() const noexcept -> const std::vector<float>& {
    return m_data;
  }

  

private:
  int m_width, m_height;
  int m_padding;
  std::vector<float> m_data;
};

void apply(int width, int height, auto&& f) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      std::forward<decltype(f)>(f)(i, j);
    }
  }
}

Image load(const char*, int padding = 0);
void save_as_png(const Image&, const char*);

} // namespace Image