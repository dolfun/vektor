#include "canny_edge_detector.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <numbers>
#include <ranges>
#include <stack>

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

  ColorImage result { image };
  for (int iter = 0; iter < nr_iterations; ++iter) {
    ColorImage source = std::move(result);
    result = ColorImage { width, height, Canny::gradient_x_kernel.size() / 2 };

    GreyscaleImage weights { width, height, 1 };
    Image::apply(width, height, [&](int x, int y) {
      auto gx = Image::evaluate_kernel<glm::vec3>(Canny::gradient_x_kernel, source, x, y);
      auto gy = Image::evaluate_kernel<glm::vec3>(Canny::gradient_y_kernel, source, x, y);
      float g2 = glm::dot(gx, gx) + glm::dot(gy, gy);
      float w = std::exp(-std::sqrt(std::sqrt(g2)) / (2.0f * h * h));
      weights[x, y] = w;
    });

    Image::apply(width, height, [&](int x, int y) {
      float weight_sum = 0.0f;
      for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
          float weight = weights[x + i, y + j];
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
    auto gx = Image::evaluate_kernel<glm::vec3>(Canny::gradient_x_kernel, image, x, y);
    auto gy = Image::evaluate_kernel<glm::vec3>(Canny::gradient_y_kernel, image, x, y);

    float a = glm::dot(gx, gx);
    float b = glm::dot(gx, gy);
    float c = glm::dot(gy, gy);

    float trace = a + c;
    float delta = glm::max(0.0f, (a - c) * (a - c) + 4.0f * b * b);
    float lambda_max = 0.5f * (trace + glm::sqrt(delta));

    float magnitude = glm::sqrt(lambda_max);
    max_magnitude = glm::max(max_magnitude, magnitude);

    constexpr float eps = 1e-12f;
    float angle = 0.5f * glm::atan(2.0f * b, a - c + eps);
    if (angle < 0.0f) angle += pi;

    result[x, y] = std::make_pair(magnitude, angle);
  });

  Image::apply(width, height, [&](int x, int y) { result[x, y].first /= max_magnitude; });

  return result;
}

GreyscaleImage thin_edges(const GradientImage& image) {
  int width = image.width();
  int height = image.height();
  GreyscaleImage result { width, height, 2 };

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

// https://www.nature.com/articles/s41598-025-86860-9
std::pair<float, float> compute_threshold(const GreyscaleImage& image, int nr_bins) {
  std::vector<int> bins(nr_bins + 1);
  Image::apply(image.width(), image.height(), [&](int x, int y) {
    int idx = std::min(nr_bins, 1 + static_cast<int>(image[x, y] * nr_bins));
    ++bins[idx];
  });

  int nr_pixels = image.width() * image.height();
  std::vector<std::pair<double, double>> pref_sums(nr_bins + 1);
  for (int i = 1; i <= nr_bins; ++i) {
    auto [sum, sum_i] = pref_sums[i - 1];
    auto p = static_cast<double>(bins[i]) / nr_pixels;
    pref_sums[i] = { sum + p, sum_i + p * i };
  }

  const double u = pref_sums[nr_bins].second;

  auto compute_inter_class_variance = [&](int tl, int th) {
    double w1 = pref_sums[tl].first;
    double w2 = pref_sums[th].first - pref_sums[tl].first;
    double w3 = pref_sums[nr_bins].first - pref_sums[th].first;

    if (w1 == 0.0 || w2 == 0.0 || w3 == 0.0) {
      return -1.0;
    }

    double p1 = pref_sums[tl].second / w1;
    double p2 = (pref_sums[th].second - pref_sums[tl].second) / w2;
    double p3 = (pref_sums[nr_bins].second - pref_sums[th].second) / w3;

    double var = w1 * (p1 - u) * (p1 - u) + w2 * (p2 - u) * (p2 - u) + w3 * (p3 - u) * (p3 - u);
    return var;
  };

  double max_var = 0.0;
  int best_tl, best_th;
  for (int tl = 1; tl < nr_bins - 1; ++tl) {
    for (int th = tl + 1; th < nr_bins; ++th) {
      double var = compute_inter_class_variance(tl, th);

      if (var > max_var) {
        max_var = var;
        best_tl = tl;
        best_th = th;
      }
    }
  }

  return { best_tl / static_cast<float>(nr_bins), best_th / static_cast<float>(nr_bins) };
}

BinaryImage
apply_hysteresis(const GreyscaleImage& image, float low, float high, float take_percentile) {
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
  Image::Image<unsigned char> visited { width, height, image.padding() };
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

  const int take_amount = static_cast<int>(weak_pixels.size() * take_percentile);
  for (const auto& v : std::views::take(weak_pixels, take_amount)) {
    for (auto p : v) {
      result[p.x, p.y] = 1;
    }
  }

  return result;
}

BinaryImage detect_edges(const ColorImage& source_image) {
  auto blurred_image = apply_adaptive_blur(source_image, 1.0f, 1);
  auto gradient_image = compute_gradient(blurred_image);
  auto thinned_image = thin_edges(gradient_image);
  auto [tl, th] = compute_threshold(thinned_image);
  auto final_image = apply_hysteresis(thinned_image, tl, th);
  return final_image;
}

}  // namespace Canny