#include "renderer.h"

#include <glm/glm.hpp>

#include "bezier_curve.h"

void draw_line(glm::dvec2 p1, glm::dvec2 p2, auto&& f) {
  auto i_part = [](double x) { return glm::floor(x); };

  auto round = [&](double x) { return i_part(x + 0.5); };

  auto f_part = [](double x) { return x - glm::floor(x); };

  auto rf_part = [&](double x) { return 1.0 - f_part(x); };

  double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
  bool steep = glm::abs(y1 - y0) > glm::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  double dx = x1 - x0, dy = y1 - y0;
  double gradient = dy / dx;
  if (dx == 0.0) gradient = 1.0;

  double xend = round(x0);
  double yend = y0 + gradient * (xend - x0);
  double xgap = rf_part(x0 + 0.5);
  double xpxl1 = xend;
  double ypxl1 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl1, xpxl1, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl1 + 1.0, xpxl1, f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl1, ypxl1, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl1, ypxl1 + 1.0, f_part(yend) * xgap);
  }

  double intery = yend + gradient;
  xend = round(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = f_part(x1 + 0.5);
  double xpxl2 = xend;
  double ypxl2 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl2, xpxl2, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl2 + 1, xpxl2, f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl2, ypxl2, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl2, ypxl2 + 1, f_part(yend) * xgap);
  }

  if (steep) {
    for (double x = xpxl1 + 1.0; x <= xpxl2 - 1.0; x += 1.0) {
      std::forward<decltype(f)>(f)(i_part(intery), x, rf_part(intery));
      std::forward<decltype(f)>(f)(i_part(intery) + 1.0, x, f_part(intery));
      intery += gradient;
    }
  } else {
    for (double x = xpxl1 + 1.0; x <= xpxl2 - 1.0; x += 1.0) {
      std::forward<decltype(f)>(f)(x, i_part(intery), rf_part(intery));
      std::forward<decltype(f)>(f)(x, i_part(intery) + 1.0, f_part(intery));
      intery += gradient;
    }
  }
}

void draw_curve(const BezierCurve& curve, auto&& f) {
  auto [p0, p1, p2, p3] = curve;

  auto square = [](auto x) { return x * x; };
  auto cube = [](auto x) { return x * x * x; };

  double delta = 0.1f;
  double dd0 = square(p0.x - 2.0 * p1.x + p2.x) + square(p0.y - 2.0 * p1.y + p2.y);
  double dd1 = square(p1.x - 2.0 * p2.x + p3.x) + square(p1.y - 2.0 * p2.y + p3.y);
  double dd = 6.0 * glm::sqrt(glm::max(dd0, dd1));
  double e2 = 8.0 * delta <= dd ? 8.0 * delta / dd : 1.0;
  double epsilon = glm::sqrt(e2);

  glm::dvec2 prev = p0;
  for (double t = epsilon; t < 1.0; t += epsilon) {
    glm::dvec2 curr = p0 * cube(1.0 - t) + p1 * 3.0 * square(1.0 - t) * t +
                      p2 * 3.0 * (1.0 - t) * square(t) + p3 * cube(t);

    draw_line(prev, curr, std::forward<decltype(f)>(f));
    prev = curr;
  }
  draw_line(prev, p3, std::forward<decltype(f)>(f));
}

namespace Renderer {

auto render_greyscale(
  int width,
  int height,
  const std::vector<BezierCurveWithColor>& curves,
  float background_value
) -> Image::GreyscaleImage {
  Image::GreyscaleImage result { width, height };
  Image::apply(width, height, [&](int x, int y) { result[x, y] = background_value; });

  for (auto [curve, _] : curves) {
    BezierCurve::scale(curve, width);

    draw_curve(curve, [&](float x, float y, float c) {
      x = glm::round(x), y = glm::round(y);
      result[x, y] = glm::mix(result[x, y], 1.0f - background_value, c);
    });
  }

  return result;
}

glm::vec3 compute_curve_color(BezierCurve curve, const Image::RGBImage& image) {
  BezierCurve::scale(curve, image.width());

  glm::vec3 path_color {};
  float weight_sum = 0.0f;
  draw_curve(curve, [&](float x, float y, float c) {
    x = glm::round(x), y = glm::round(y);
    path_color += image[x, y] * c;
    weight_sum += c;
  });
  path_color /= weight_sum;

  return path_color;
}

auto render_color(
  int width,
  int height,
  const std::vector<BezierCurveWithColor>& curves,
  glm::vec3 background_color
) -> Image::RGBImage {
  Image::RGBImage result { width, height };
  Image::apply(width, height, [&](int x, int y) { result[x, y] = background_color; });

  for (auto [curve, color] : curves) {
    BezierCurve::scale(curve, width);
    draw_curve(curve, [&](float x, float y, float c) {
      x = glm::round(x), y = glm::round(y);
      result[x, y] = glm::mix(result[x, y], color, c);
    });
  }

  return result;
}

};  // namespace Renderer