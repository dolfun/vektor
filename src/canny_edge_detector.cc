#include "canny_edge_detector.h"
#include <numeric>
#include <numbers>
#include <utility>
#include <cassert>
#include <ranges>
#include <cmath>

using Image::Image_t;
constexpr auto pi = std::numbers::pi_v<float>;

namespace Canny {

Image_t apply_adaptive_blur(const Image_t& image, float h, int nr_iterations = 1) {
  int width = image.width();
  int height = image.height();

  Image_t result = image;
  for (int iter = 0; iter < nr_iterations; ++iter) {
    Image_t source = std::move(result);
    result = Image_t { width, height, gradient_x_kernel.size() / 2 };

    Image_t weights { width, height, 1 };
    int inset = 3;
    Image::apply_with_inset(width, height, inset, inset, [&] (int x, int y) {
      float grad_x = Image::evaluate_kernel(gradient_x_kernel, source, x, y);
      float grad_y = Image::evaluate_kernel(gradient_y_kernel, source, x, y);
      weights(x, y) = std::expf(-std::powf(grad_x * grad_x + grad_y * grad_y, 0.25f) / (2.0f * h * h));
    });

    Image::apply_with_inset(width, height, inset, inset, [&] (int x, int y) {
      float weight_sum = 0.0f;
      for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
          float weight = weights(x + i, y + j);
          result(x, y) += source(x + i, y + j) * weight;
          weight_sum += weight;
        }
      }

      result(x, y) /= weight_sum;
    });
  }
  
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

float compute_threshold(const Image_t& image, int nr_bins = 256) {
  std::vector<int> bins(nr_bins);
  Image::apply(image.width(), image.height(), [&] (int x, int y) {
    int index = static_cast<int>(image(x, y) * (nr_bins - 1));
    ++bins[index];
  });

  float w_t = static_cast<float>(image.width() * image.height());
  auto indices = std::views::iota(0);
  float mu_t = std::inner_product(
    bins.begin(), bins.end(),
    indices.begin(), 0.0f,
    std::plus<>{}, std::multiplies<float>{}
  );

  float maximum = 0.0f;
  int maximum_index;
  float w_0 = 0.0f, mu_0 = 0.0f;
  for (int i = 0; i < nr_bins; ++i) {
    float w_1 = w_t - w_0;
    if (w_0 > 0.0f && w_1 > 0.0f) {
      float mu_1 = (mu_t - mu_0) / w_1;
      float val = w_0 * w_1 * (mu_1 / w_1 - mu_0) * (mu_1 / w_1 - mu_0);
      if (val > maximum) {
        maximum = val;
        maximum_index = i;
      }
    }

    float count = static_cast<float>(bins[i]);
    w_0 += count;
    mu_0 += i * count;
  }

  return static_cast<float>(maximum_index) / nr_bins;
}

Image_t detect_edges(const Image_t& source_image) {
  assert(source_image.padding() == gaussian_kernel.size() / 2);
  Image_t blurred_image = apply_adaptive_blur(source_image, 1.5f, 8);

  GradientImage_t gradient_image = compute_gradient(blurred_image);

  Image_t thinned_image = thin_edges(gradient_image);

  float high_threshold = compute_threshold(thinned_image);
  float low_threshold = high_threshold / 2.0f;

  Image::apply(thinned_image.width(), thinned_image.height(), [&] (int x, int y) {
    float val = thinned_image(x, y);
    if (val >= high_threshold) {
      thinned_image(x, y) = 1.0f;
    } else if (val >= low_threshold) {
      thinned_image(x, y) = 0.5f;
    } else {
      thinned_image(x, y) = 0.0f;
    }
  });

  return thinned_image;
}

} // namespace Canny