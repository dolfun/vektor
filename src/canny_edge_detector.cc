#include "canny_edge_detector.h"
#include <utility>
#include <cassert>
#include <cmath>

using Image::Image_t;

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

    float magnitude = std::sqrt(grad_x * grad_x + grad_y * grad_y);
    max_magnitude = std::max(max_magnitude, magnitude);
    float angle = std::atan2(grad_y, grad_x);

    result(x, y) = std::make_pair(magnitude, angle);
  });

  Image::apply(width, height, [&] (int x, int y) {
    result(x, y).first /= max_magnitude;
  });

  return result;
}

Image_t detect_edges(const Image_t& source_image) {
  assert(source_image.padding() == gaussian_kernel.size() / 2);
  Image_t blurred_image = apply_gaussian_blur(source_image);

  GradientImage_t gradient_image = compute_gradient(blurred_image);

  Image_t result { source_image.width(), source_image.height() };
  Image::apply(source_image.width(), source_image.height(), [&] (int x, int y) {
    result(x, y) = gradient_image(x, y).first;
  });

  return result;
}

} // namespace Canny