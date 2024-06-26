#include "canny_edge_detector.h"
#include <numbers>
#include <utility>
#include <cassert>
#include <cmath>

using Image::Image_t;
constexpr auto pi = std::numbers::pi_v<float>;

namespace Canny {

Image_t apply_gaussian_blur(const Image_t& image) {
  int width = image.width();
  int height = image.height();
  Image_t result { width, height, gradient_x_kernel.size() / 2 };

  Image::apply(width, height, [&] (int x, int y) {
    result(x, y) = Image::evaluate_kernel(gaussian_kernel, image, x, y);
  });

  return result;
}

using GradientImage_t = Image::Image<std::pair<float, float>>;
GradientImage_t compute_gradient(const Image_t& image) {
  int width = image.width();
  int height = image.height();
  GradientImage_t result { width, height, 1 };

  float max_magnitude = 0.0f;
  int inset = gradient_x_kernel.size();
  Image::apply_with_inset(width, height, inset, inset, [&] (int x, int y) {
    float grad_x = Image::evaluate_kernel(gradient_x_kernel, image, x, y);
    float grad_y = Image::evaluate_kernel(gradient_y_kernel, image, x, y);

    float magnitude = std::sqrtf(grad_x * grad_x + grad_y * grad_y);
    max_magnitude = std::max(max_magnitude, magnitude);
    float angle = std::atan2f(grad_y, grad_x);
    if (angle < 0.0f) angle = pi - angle;

    result(x, y) = std::make_pair(magnitude, angle);
  });

  Image::apply(width, height, [&] (int x, int y) {
    result(x, y).first /= max_magnitude;
  });

  return result;
}

Image_t thin_edges(const GradientImage_t& image) {
  int width = image.width();
  int height = image.height();
  Image_t result { width, height };

  constexpr std::array<std::array<int, 2>, 4> dirs {
     1, 0, 
     1, 1,
     0, 1,
    -1, 1,
  };

  Image::apply(width, height, [&] (int x, int y) {
    float angle = image(x, y).second;
    int dir_index = static_cast<int>((angle + pi / 8.0f) / (pi / 4.0f));
    auto dir = dirs[dir_index % dirs.size()];

    float g0 = image(x, y).first;
    float g1 = image(x + dir[0], y + dir[1]).first;
    float g2 = image(x - dir[0], y - dir[1]).first;

    if (g0 > g1 && g0 > g2) {
      result(x, y) = g0;
    }
  });

  return result;
}

Image_t detect_edges(const Image_t& source_image) {
  assert(source_image.padding() == gaussian_kernel.size() / 2);
  Image_t blurred_image = apply_gaussian_blur(source_image);

  GradientImage_t gradient_image = compute_gradient(blurred_image);

  Image_t result = thin_edges(gradient_image);

  return result;
}

} // namespace Canny