#pragma once
#include "image.h"
#include <array>

namespace Image {

template <int N>
struct Kernel {
  static_assert(N % 2 == 1);
  
  constexpr float operator()(int i, int j) const noexcept {
    return data[N * i + j];
  }

  constexpr int size() const noexcept {
    return N;
  }

  std::array<int, N * N> data;
  float normalizing_factor = 1.0f;
};

template <int N>
inline float evaluate_kernel(const Kernel<N>& kernel, const Image& image, int i, int j) noexcept {
  float result = 0.0f;

  for (int i0 = -N / 2; i0 <= N / 2; ++i0) {
    for (int j0 = -N / 2; j0 <= N / 2; ++j0) {
      result += kernel(i0 + N / 2, j0 + N / 2) * image(i + i0, j + j0);
    }
  }
  result /= kernel.normalizing_factor;

  return result;
}

} // namespace Image

