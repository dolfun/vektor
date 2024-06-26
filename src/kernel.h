#pragma once
#include <array>

namespace Image {

template <int N>
struct Kernel {
  static_assert(N % 2 == 1);
  
  constexpr float operator()(int x, int y) const noexcept {
    return data[N * y + x];
  }

  constexpr int size() const noexcept {
    return N;
  }

  std::array<int, N * N> data;
  int normalizing_factor = 1;
};

template <int N>
inline float evaluate_kernel(const Kernel<N>& kernel, const auto& f, int x, int y) noexcept {
  float result = 0.0f;

  for (int y0 = -N / 2; y0 <= N / 2; ++y0) {
    for (int x0 = -N / 2; x0 <= N / 2; ++x0) {
      result += kernel(x0 + N / 2, y0 + N / 2) * f(x - x0, y - y0);
    }
  }
  result /= static_cast<float>(kernel.normalizing_factor);

  return result;
}

} // namespace Image

