#pragma once
#include "bezier_curve.h"
#include "image.h"

namespace Renderer {

auto render_greyscale(int, int, const std::vector<BezierCurve>&, float = 0.0f)
  -> Image::GreyscaleImage;

glm::vec3 compute_curve_color(const BezierCurve&, const Image::ColorImage&);

auto render_color(
  int,
  int,
  const std::vector<BezierCurve>&,
  const Image::ColorImage&,
  glm::vec3 = glm::vec3(0.0f)
) -> Image::ColorImage;

};  // namespace Renderer