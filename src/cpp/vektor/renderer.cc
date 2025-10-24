#include "renderer.h"

#include <glm/glm.hpp>

void draw_line(glm::vec2 p1, glm::vec2 p2, auto&& f) {
  auto i_part = [](float x) { return glm::floor(x); };

  auto round = [&](float x) { return i_part(x + 0.5f); };

  auto f_part = [](float x) { return x - glm::floor(x); };

  auto rf_part = [&](float x) { return 1.0f - f_part(x); };

  float x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
  bool steep = glm::abs(y1 - y0) > glm::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  float dx = x1 - x0, dy = y1 - y0;
  float gradient = dy / dx;
  if (dx == 0.0f) gradient = 1.0f;

  float xend = round(x0);
  float yend = y0 + gradient * (xend - x0);
  float xgap = rf_part(x0 + 0.5f);
  float xpxl1 = xend;
  float ypxl1 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl1, xpxl1, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl1 + 1.0f, xpxl1, f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl1, ypxl1, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl1, ypxl1 + 1.0f, f_part(yend) * xgap);
  }

  float intery = yend + gradient;
  xend = round(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = f_part(x1 + 0.5f);
  float xpxl2 = xend;
  float ypxl2 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl2, xpxl2, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl2 + 1, xpxl2, f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl2, ypxl2, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl2, ypxl2 + 1, f_part(yend) * xgap);
  }

  if (steep) {
    for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x += 1.0f) {
      std::forward<decltype(f)>(f)(i_part(intery), x, rf_part(intery));
      std::forward<decltype(f)>(f)(i_part(intery) + 1.0f, x, f_part(intery));
      intery += gradient;
    }
  } else {
    for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x += 1.0f) {
      std::forward<decltype(f)>(f)(x, i_part(intery), rf_part(intery));
      std::forward<decltype(f)>(f)(x, i_part(intery) + 1.0f, f_part(intery));
      intery += gradient;
    }
  }
}

void draw_curve(const BezierCurve& curve, auto&& f) {
  auto [p0, p1, p2, p3] = curve;

  auto square = [](auto x) { return x * x; };
  auto cube = [](auto x) { return x * x * x; };

  float delta = 0.1f;
  float dd0 = square(p0.x - 2.0f * p1.x + p2.x) + square(p0.y - 2.0f * p1.y + p2.y);
  float dd1 = square(p1.x - 2.0f * p2.x + p3.x) + square(p1.y - 2.0f * p2.y + p3.y);
  float dd = 6.0f * glm::sqrt(glm::max(dd0, dd1));
  float e2 = 8.0f * delta <= dd ? 8.0f * delta / dd : 1.0f;
  float epsilon = glm::sqrt(e2);

  glm::vec2 prev = p0;
  for (float t = epsilon; t < 1.0f; t += epsilon) {
    glm::vec2 curr = p0 * cube(1.0f - t) + p1 * 3.0f * square(1.0f - t) * t +
                     p2 * 3.0f * (1.0f - t) * square(t) + p3 * cube(t);

    draw_line(prev, curr, std::forward<decltype(f)>(f));
    prev = curr;
  }
  draw_line(prev, p3, std::forward<decltype(f)>(f));
}

namespace Renderer {

auto render_greyscale(int width, int height, const std::vector<BezierCurve>& curves)
  -> Image::GreyscaleImage {
  Image::GreyscaleImage result { width, height };
  for (auto curve : curves) {
    curve.apply_scale(width);

    draw_curve(curve, [&](float x, float y, float c) {
      result[x, y] = glm::mix(result[x, y], 1.0f, c);
    });
  }

  return result;
}

auto render_color(
  int width,
  int height,
  const std::vector<BezierCurve>& curves,
  const Image::ColorImage& color_image
) -> Image::ColorImage {
  Image::ColorImage result { width, height };
  for (auto curve : curves) {
    auto curve_for_color = curve;
    curve_for_color.apply_scale(color_image.width());

    glm::vec3 path_color {};
    float weight_sum = 0.0f;
    draw_curve(curve_for_color, [&](float x, float y, float c) {
      path_color += color_image[x, y] * c;
      weight_sum += c;
    });
    path_color /= weight_sum;

    curve.apply_scale(width);
    draw_curve(curve, [&](float x, float y, float c) {
      result[x, y] = glm::mix(result[x, y], path_color, c);
    });
  }

  return result;
}

};  // namespace Renderer