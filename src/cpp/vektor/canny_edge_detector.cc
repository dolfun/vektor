#include "canny_edge_detector.h"

#include <algorithm>
#include <cassert>
#include <glm/glm.hpp>
#include <numbers>
#include <numeric>
#include <ranges>
#include <stack>
#include <utility>

#include "image.h"

using Image::BinaryImage;
using Image::ColorImage;
using Image::GradientImage;
using Image::GreyscaleImage;
constexpr auto pi = std::numbers::pi_v<float>;

namespace Canny {

ColorImage apply_adaptive_blur(const ColorImage& image, float h, int nr_iterations) {
  int width = image.width();
  int height = image.height();

  ColorImage result = image;
  for (int iter = 0; iter < nr_iterations; ++iter) {
    ColorImage source = std::move(result);
    result = ColorImage { width, height, Canny::gradient_x_kernel.size() / 2 };

    ColorImage weights { width, height, 1 };
    Image::apply(width, height, [&](int x, int y) {
      auto grad_x = Image::evaluate_kernel<glm::vec3>(Canny::gradient_x_kernel, source, x, y);
      auto grad_y = Image::evaluate_kernel<glm::vec3>(Canny::gradient_y_kernel, source, x, y);
      weights[x, y] =
        glm::exp(-glm::sqrt(glm::sqrt(grad_x * grad_x + grad_y * grad_y)) / (2.0f * h * h));
    });

    Image::apply(width, height, [&](int x, int y) {
      glm::vec3 weight_sum { 0.0f };
      for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
          glm::vec3 weight = weights[x + i, y + j];
          result[x, y] += source[x + i, y + j] * weight;
          weight_sum += weight;
        }
      }
      result[x, y] /= weight_sum;
    });
  }

  return result;
}

GradientImage compute_gradient(const ColorImage& image) {
  int width = image.width();
  int height = image.height();
  GradientImage result { width, height, 1 };

  float max_magnitude = 0.0f;
  int inset = 0;
  Image::apply_with_inset(width, height, inset, inset, [&](int x, int y) {
    auto grad_x = Image::evaluate_kernel<glm::vec3>(Canny::gradient_x_kernel, image, x, y);
    auto grad_y = Image::evaluate_kernel<glm::vec3>(Canny::gradient_y_kernel, image, x, y);

    float grad_r = grad_x.r * grad_x.r + grad_y.r * grad_y.r;
    float grad_g = grad_x.g * grad_x.g + grad_y.g * grad_y.g;
    float grad_b = grad_x.b * grad_x.b + grad_y.b * grad_y.b;

    glm::vec2 grad;
    if (grad_r > grad_g && grad_r > grad_b) {
      grad = glm::vec2(grad_x.r, grad_y.r);
    } else if (grad_g > grad_b) {
      grad = glm::vec2(grad_x.g, grad_y.g);
    } else {
      grad = glm::vec2(grad_x.b, grad_y.b);
    }

    float magnitude = glm::length(grad);
    max_magnitude = std::max(max_magnitude, magnitude);
    float angle = std::atan2f(grad.y, grad.x);
    if (angle < 0.0f) angle += pi;

    result[x, y] = std::make_pair(magnitude, angle);
  });

  Image::apply(width, height, [&](int x, int y) { result[x, y].first /= max_magnitude; });

  return result;
}

GreyscaleImage thin_edges(const GradientImage& image) {
  int width = image.width();
  int height = image.height();
  GreyscaleImage result { width, height, 1 };

  Image::apply(width, height, [&](int x, int y) {
    float angle = image[x, y].second * 180.0f / pi;

    glm::ivec2 dir;
    if (angle <= 22.5f || angle >= 157.5f)
      dir = { 1, 0 };
    else if (angle < 67.5f)
      dir = { 1, 1 };
    else if (angle < 122.5f)
      dir = { 0, 1 };
    else
      dir = { -1, 1 };

    float g0 = image[x, y].first;
    float g1 = image[x + dir.x, y + dir.y].first;
    float g2 = image[x - dir.x, y - dir.y].first;

    if (g0 > g1 && g0 > g2) {
      result[x, y] = g0;
    }
  });

  for (int x = 0; x < result.width(); ++x) {
    result[x, 0] = result[x, result.height() - 1] = 0.0f;
  }

  return result;
}

float compute_threshold(const GreyscaleImage& image) {
  constexpr int nr_bins = 256;
  std::vector<int> bins(nr_bins);
  Image::apply(image.width(), image.height(), [&](int x, int y) {
    int index = static_cast<int>(image[x, y] * (nr_bins - 1));
    ++bins[index];
  });

  float w_t = static_cast<float>(image.width() * image.height());
  auto indices = std::views::iota(0);
  float mu_t = std::inner_product(
    bins.begin(),
    bins.end(),
    indices.begin(),
    0.0f,
    std::plus<> {},
    std::multiplies<float> {}
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

BinaryImage apply_hysteresis(const GreyscaleImage& image, float high, float low) {
  // clang-format off
  constexpr std::array<std::pair<int, int>, 8> dirs = {{
     { -1, -1, },
     {  0, -1, },
     {  1, -1, },
     { -1,  0, },
     {  1,  0, },
     { -1,  1, },
     {  0,  1, },
     {  1,  1, },
  }};
  // clang-format on

  int width = image.width();
  int height = image.height();
  Image::Image<unsigned char> visited { width, height, 1 };
  auto dfs = [&](int x0, int y0, std::vector<glm::ivec2>& points) {
    std::stack<glm::ivec2> s;
    s.push({ x0, y0 });

    bool found_strong_pixel = false;

    while (!s.empty()) {
      auto p = s.top();
      s.pop();

      points.push_back(p);
      visited[p.x, p.y] = true;

      for (auto [dx, dy] : dirs) {
        glm::ivec2 p1 = p + glm::ivec2(dx, dy);
        float val = image[p1.x, p1.y];

        if (!found_strong_pixel && val >= high) {
          found_strong_pixel = true;
        }

        if (visited[p1.x, p1.y] || val < low || val >= high) continue;
        s.push({ p1.x, p1.y });
      }
    }

    return found_strong_pixel;
  };

  std::vector<std::vector<glm::ivec2>> weak_pixels;
  BinaryImage result { width, height, 2 };
  Image::apply(width, height, [&](int x, int y) {
    if (visited[x, y]) return;
    float val = image[x, y];

    if (val >= high) {
      result[x, y] = 1;

    } else if (val >= low) {
      std::vector<glm::ivec2> points;
      bool strong_pixel_found = dfs(x, y, points);

      if (strong_pixel_found) {
        for (auto p : points) {
          result[p.x, p.y] = 1;
        }
      } else {
        weak_pixels.emplace_back(std::move(points));
      }
    }
  });

  std::ranges::sort(weak_pixels, std::greater<> {}, [&](const auto& v) { return v.size(); });

  constexpr float take_percentile = 0.25f;
  const int take_amount = static_cast<int>(weak_pixels.size() * take_percentile);
  for (const auto& v : std::views::take(weak_pixels, take_amount)) {
    for (auto p : v) {
      result[p.x, p.y] = 1;
    }
  }

  return result;
}

BinaryImage detect_edges(const ColorImage& source_image, float threshold) {
  auto blurred_image = apply_adaptive_blur(source_image, 1.0f, 1);
  auto gradient_image = compute_gradient(blurred_image);
  auto thinned_image = thin_edges(gradient_image);

  float high_threshold = (threshold > 0.0f ? threshold : compute_threshold(thinned_image));
  float low_threshold = high_threshold / 2.0f;
  auto final_image = apply_hysteresis(thinned_image, high_threshold, low_threshold);
  return final_image;
}

}  // namespace Canny